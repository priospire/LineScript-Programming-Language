param(
  [string]$FrontendCompiler = "clang++",
  [string]$LineScriptBackendCompiler = "",
  [string]$BackendCompiler = "clang",
  [string]$CCompiler = "clang",
  [string]$CppCompiler = "clang++",
  [string]$ZigCompilerPath = "",
  [string]$CaseFilter = "",
  [int]$WarmupRuns = 3,
  [int]$MeasuredRuns = 9,
  [double]$ConfidenceMarginPercent = 2.0,
  [int]$RequirePerTestDomination = 1,
  [int]$IncludeEdgeNonBlocking = 1,
  [int]$MaxTests = 0,
  [switch]$SkipFrontendBuild
)

$ErrorActionPreference = "Stop"
if ($null -ne (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue)) {
  $PSNativeCommandUseErrorActionPreference = $false
}

if ($WarmupRuns -lt 0) { throw "WarmupRuns must be >= 0." }
if ($MeasuredRuns -lt 1) { throw "MeasuredRuns must be >= 1." }
if ($ConfidenceMarginPercent -lt 0.0) { throw "ConfidenceMarginPercent must be >= 0." }
if ($RequirePerTestDomination -ne 0 -and $RequirePerTestDomination -ne 1) { throw "RequirePerTestDomination must be 0 or 1." }
if ($IncludeEdgeNonBlocking -ne 0 -and $IncludeEdgeNonBlocking -ne 1) { throw "IncludeEdgeNonBlocking must be 0 or 1." }
$RequirePerTestDomination = ($RequirePerTestDomination -ne 0)
$IncludeEdgeNonBlocking = ($IncludeEdgeNonBlocking -ne 0)
if ($MaxTests -lt 0) { throw "MaxTests must be >= 0." }

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

$lsBackend = if ([string]::IsNullOrWhiteSpace($LineScriptBackendCompiler)) { $BackendCompiler } else { $LineScriptBackendCompiler }

function Normalize-Text {
  param([string]$Text)
  if ($null -eq $Text) { return "" }
  $x = $Text -replace "`r`n", "`n"
  $x = $x -replace "`r", "`n"
  return $x.TrimEnd("`n")
}

function Ensure-Directory {
  param([string]$Path)
  if (-not (Test-Path $Path)) {
    New-Item -ItemType Directory -Path $Path | Out-Null
  }
}

function Resolve-ToolPath {
  param(
    [string]$Provided,
    [string[]]$CommandNames,
    [string[]]$SearchFilenames,
    [string[]]$SearchRoots
  )

  if (-not [string]::IsNullOrWhiteSpace($Provided)) {
    if (Test-Path $Provided) {
      return (Resolve-Path $Provided).Path
    }
    $cmd = Get-Command $Provided -ErrorAction SilentlyContinue
    if ($null -ne $cmd) {
      return $cmd.Source
    }
  }

  foreach ($name in $CommandNames) {
    if ([string]::IsNullOrWhiteSpace($name)) { continue }
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($null -ne $cmd) {
      return $cmd.Source
    }
  }

  foreach ($file in $SearchFilenames) {
    foreach ($root in $SearchRoots) {
      if ([string]::IsNullOrWhiteSpace($root)) { continue }
      if (-not (Test-Path $root)) { continue }
      $found = Get-ChildItem -Path $root -Recurse -Filter $file -File -ErrorAction SilentlyContinue | Select-Object -First 1
      if ($null -ne $found) {
        return $found.FullName
      }
    }
  }

  return $null
}

function Build-Frontend {
  param([string]$Preferred)
  $compilerSrcPrimary = Join-Path $repoRoot "src\lsc.cpp"
  $compilerSrcFallback = Join-Path $repoRoot "lsc.cpp"
  $compilerExe = Join-Path $repoRoot "lsc.exe"

  $compilerSrc = $null
  foreach ($candidate in @($compilerSrcPrimary, $compilerSrcFallback)) {
    if (Test-Path $candidate) {
      $compilerSrc = $candidate
      break
    }
  }

  if ($null -eq $compilerSrc) {
    if (Test-Path $compilerExe) {
      Write-Host "Using existing frontend binary: lsc.exe"
      return
    }
    throw "Missing both lsc.exe and compiler source."
  }

  foreach ($cxx in @($Preferred, "clang++", "g++")) {
    if ([string]::IsNullOrWhiteSpace($cxx)) { continue }
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
      & $cxx -std=c++20 -O3 -Wall -Wextra -pedantic $compilerSrc -o $compilerExe 2>&1 | Out-Null
    } finally {
      $ErrorActionPreference = $prev
    }
    if ($LASTEXITCODE -eq 0) {
      Write-Host "Frontend build OK with $cxx"
      return
    }
  }
  throw "Failed to build lsc.exe."
}

function Invoke-Lsc {
  param([string[]]$CliArgs)
  $prev = $ErrorActionPreference
  $ErrorActionPreference = "Continue"
  try {
    $out = & .\lsc.exe @CliArgs 2>&1 | Out-String
  } finally {
    $ErrorActionPreference = $prev
  }
  return [PSCustomObject]@{
    ExitCode = $LASTEXITCODE
    Output = $out
  }
}

function Invoke-Compiler {
  param(
    [string]$Compiler,
    [string[]]$Flags,
    [string]$SourcePath,
    [string]$OutputPath
  )
  $prev = $ErrorActionPreference
  $ErrorActionPreference = "Continue"
  try {
    $out = & $Compiler @Flags $SourcePath "-o" $OutputPath 2>&1 | Out-String
  } finally {
    $ErrorActionPreference = $prev
  }
  return [PSCustomObject]@{
    ExitCode = $LASTEXITCODE
    Output = $out
  }
}

function Invoke-ProcessTimed {
  param(
    [string]$Command,
    [string[]]$Arguments,
    [string]$InputText = ""
  )

  $stdout = ""
  $stderr = ""
  $exitCode = 1
  $threw = $false
  $sw = [System.Diagnostics.Stopwatch]::StartNew()

  try {
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $Command
    $psi.UseShellExecute = $false
    $psi.RedirectStandardInput = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true
    foreach ($arg in $Arguments) {
      [void]$psi.ArgumentList.Add($arg)
    }

    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $psi
    [void]$proc.Start()

    if (-not [string]::IsNullOrEmpty($InputText)) {
      $proc.StandardInput.Write($InputText)
    }
    $proc.StandardInput.Close()

    $stdout = $proc.StandardOutput.ReadToEnd()
    $stderr = $proc.StandardError.ReadToEnd()
    $proc.WaitForExit()
    $exitCode = $proc.ExitCode
  } catch {
    $threw = $true
    $stderr = $stderr + $_.Exception.Message
    $exitCode = 1
  } finally {
    $sw.Stop()
  }

  $combined = if ([string]::IsNullOrEmpty($stderr)) { $stdout } else { $stdout + "`n" + $stderr }
  return [PSCustomObject]@{
    ExitCode = $exitCode
    Stdout = Normalize-Text $stdout
    Stderr = Normalize-Text $stderr
    Output = Normalize-Text $combined
    ElapsedMs = $sw.Elapsed.TotalMilliseconds
    Threw = $threw
  }
}

function Get-FreeTcpPort {
  $listener = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, 0)
  $listener.Start()
  $port = ($listener.LocalEndpoint).Port
  $listener.Stop()
  return $port
}

function Invoke-HttpBurstClient {
  param(
    [int]$Port,
    [int]$Requests,
    [int]$ConnectRetries = 500,
    [int]$RetryDelayMs = 10
  )
  $req = "GET / HTTP/1.1`r`nHost: 127.0.0.1`r`nConnection: close`r`n`r`n"
  $bytes = [System.Text.Encoding]::ASCII.GetBytes($req)

  for ($i = 0; $i -lt $Requests; $i++) {
    $client = $null
    try {
      $client = New-Object System.Net.Sockets.TcpClient
      $client.NoDelay = $true
      $client.ReceiveTimeout = 5000
      $client.SendTimeout = 5000
      $connected = $false
      for ($attempt = 0; $attempt -lt $ConnectRetries; $attempt++) {
        try {
          $client.Connect("127.0.0.1", $Port)
          $connected = $true
          break
        } catch {
          Start-Sleep -Milliseconds $RetryDelayMs
        }
      }
      if (-not $connected) {
        return [PSCustomObject]@{ Ok = $false; Error = "failed to connect to 127.0.0.1:$Port" }
      }

      $stream = $client.GetStream()
      $stream.Write($bytes, 0, $bytes.Length)
      $stream.Flush()
    } catch {
      return [PSCustomObject]@{ Ok = $false; Error = $_.Exception.Message }
    } finally {
      if ($null -ne $client) {
        $client.Close()
      }
    }
  }

  return [PSCustomObject]@{ Ok = $true; Error = "" }
}

function Invoke-HttpServerTimed {
  param(
    [string]$Command,
    [string[]]$Arguments,
    [int]$Port,
    [int]$Requests
  )

  $stdout = ""
  $stderr = ""
  $exitCode = 1
  $threw = $false
  $sw = [System.Diagnostics.Stopwatch]::StartNew()

  try {
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $Command
    $psi.UseShellExecute = $false
    $psi.RedirectStandardInput = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true
    foreach ($arg in $Arguments) {
      [void]$psi.ArgumentList.Add($arg)
    }

    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $psi
    [void]$proc.Start()

    $proc.StandardInput.Write("$Port`n$Requests`n")
    $proc.StandardInput.Close()

    $httpRes = Invoke-HttpBurstClient -Port $Port -Requests $Requests
    if (-not $httpRes.Ok) {
      try { $proc.Kill($true) } catch { }
      throw $httpRes.Error
    }

    if (-not $proc.WaitForExit(30000)) {
      try { $proc.Kill($true) } catch { }
      throw "server did not exit in time"
    }

    $stdout = $proc.StandardOutput.ReadToEnd()
    $stderr = $proc.StandardError.ReadToEnd()
    $exitCode = $proc.ExitCode
  } catch {
    $threw = $true
    $stderr = $stderr + $_.Exception.Message
    $exitCode = 1
  } finally {
    $sw.Stop()
  }

  $combined = if ([string]::IsNullOrEmpty($stderr)) { $stdout } else { $stdout + "`n" + $stderr }
  return [PSCustomObject]@{
    ExitCode = $exitCode
    Stdout = Normalize-Text $stdout
    Stderr = Normalize-Text $stderr
    Output = Normalize-Text $combined
    ElapsedMs = $sw.Elapsed.TotalMilliseconds
    Threw = $threw
  }
}

function Get-CaseStableOutput {
  param(
    [string]$CaseName,
    [string]$Stdout
  )

  $lines = @()
  foreach ($line in (Normalize-Text $Stdout) -split "`n") {
    $trimmed = $line.Trim()
    if ($trimmed -ne "") { $lines += $trimmed }
  }
  if ($lines.Count -eq 0) {
    return [PSCustomObject]@{ Ok = $false; Value = ""; Error = "no stdout lines" }
  }

  if ($CaseName -eq "http" -and $lines[0] -eq "READY") {
    if ($lines.Count -lt 2) {
      return [PSCustomObject]@{ Ok = $false; Value = ""; Error = "missing checksum after READY" }
    }
    return [PSCustomObject]@{ Ok = $true; Value = $lines[1]; Error = "" }
  }

  return [PSCustomObject]@{ Ok = $true; Value = $lines[0]; Error = "" }
}

function Invoke-GauntletCaseTimed {
  param(
    [string]$CaseName,
    [string]$Command,
    [string[]]$Arguments
  )

  $runRes = $null
  switch ($CaseName) {
    "compute" {
      $runRes = Invoke-ProcessTimed -Command $Command -Arguments $Arguments -InputText "1337`n0`n1200000`n1`n17`n29`n1000003`n"
    }
    "concurrency" {
      $runRes = Invoke-ProcessTimed -Command $Command -Arguments $Arguments -InputText "1337`n4`n40000`n8`n"
    }
    "http" {
      $port = Get-FreeTcpPort
      $runRes = Invoke-HttpServerTimed -Command $Command -Arguments $Arguments -Port $port -Requests 8
    }
    default {
      $runRes = Invoke-ProcessTimed -Command $Command -Arguments $Arguments -InputText ""
    }
  }

  $stable = Get-CaseStableOutput -CaseName $CaseName -Stdout $runRes.Stdout
  return [PSCustomObject]@{
    ExitCode = $runRes.ExitCode
    Stdout = $runRes.Stdout
    Stderr = $runRes.Stderr
    Output = $runRes.Output
    ElapsedMs = $runRes.ElapsedMs
    Threw = $runRes.Threw
    StableOk = $stable.Ok
    StableValue = $stable.Value
    StableError = $stable.Error
  }
}

function Stop-ArtifactProcesses {
  param([string]$RootPath)
  try {
    $rootNorm = [System.IO.Path]::GetFullPath($RootPath).ToLowerInvariant()
    Get-Process | ForEach-Object {
      try {
        $p = $_
        if ([string]::IsNullOrEmpty($p.Path)) { return }
        $procPath = [System.IO.Path]::GetFullPath($p.Path).ToLowerInvariant()
        if ($procPath.StartsWith($rootNorm)) {
          Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
        }
      } catch {
      }
    }
  } catch {
  }
}

function Get-Median {
  param([double[]]$Values)
  if ($null -eq $Values -or $Values.Count -eq 0) {
    return [double]::NaN
  }
  $sorted = $Values | Sort-Object
  $count = $sorted.Count
  if (($count % 2) -eq 1) {
    return [double]$sorted[[int]($count / 2)]
  }
  $hi = [int]($count / 2)
  $lo = $hi - 1
  return ([double]$sorted[$lo] + [double]$sorted[$hi]) / 2.0
}

function Format-Double {
  param(
    [double]$Value,
    [int]$Digits = 6
  )
  if ([double]::IsNaN($Value)) { return "NaN" }
  if ([double]::IsPositiveInfinity($Value)) { return "inf" }
  if ([double]::IsNegativeInfinity($Value)) { return "-inf" }
  return [string]::Format("{0:F$Digits}", $Value)
}

function New-MultiComparison {
  param([hashtable]$MedianMap)
  $rows = @()
  foreach ($k in $MedianMap.Keys) {
    $rows += [PSCustomObject]@{ Language = $k; Ms = [double]$MedianMap[$k] }
  }
  $sorted = $rows | Sort-Object Ms, Language
  $winner = $sorted[0]
  $runnerUp = $sorted[1]
  $speedup = if ($winner.Ms -le 0.0) { [double]::PositiveInfinity } else { $runnerUp.Ms / $winner.Ms }
  $percent = if ($runnerUp.Ms -le 0.0) { 0.0 } else { (($runnerUp.Ms - $winner.Ms) / $runnerUp.Ms) * 100.0 }
  $delta = [Math]::Abs($runnerUp.Ms - $winner.Ms)
  return [PSCustomObject]@{
    FasterLanguage = $winner.Language
    RunnerUpLanguage = $runnerUp.Language
    SpeedupX = $speedup
    PercentFaster = $percent
    DeltaMs = $delta
  }
}

if (-not $SkipFrontendBuild) {
  Build-Frontend -Preferred $FrontendCompiler
}

$searchRoots = @(
  (Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages"),
  "C:\Program Files",
  "C:\Program Files (x86)"
)

$zigExe = Resolve-ToolPath -Provided $ZigCompilerPath -CommandNames @("zig") -SearchFilenames @("zig.exe") -SearchRoots $searchRoots
$cCompilerExe = Resolve-ToolPath -Provided $CCompiler -CommandNames @("clang", "gcc") -SearchFilenames @("clang.exe", "gcc.exe") -SearchRoots $searchRoots
$cppCompilerExe = Resolve-ToolPath -Provided $CppCompiler -CommandNames @("clang++", "g++") -SearchFilenames @("clang++.exe", "g++.exe") -SearchRoots $searchRoots

$missingTools = New-Object System.Collections.Generic.List[string]
if ([string]::IsNullOrWhiteSpace($zigExe)) { $missingTools.Add("zig") }
if ([string]::IsNullOrWhiteSpace($cCompilerExe)) { $missingTools.Add("c-compiler") }
if ([string]::IsNullOrWhiteSpace($cppCompilerExe)) { $missingTools.Add("cpp-compiler") }
if ($missingTools.Count -gt 0) {
  throw "Missing required tools: $($missingTools -join ', ')"
}

Write-Host "Resolved tools:"
Write-Host "  zig: $zigExe"
Write-Host "  cc:  $cCompilerExe"
Write-Host "  cxx: $cppCompilerExe"

$gauntletRoot = Join-Path $repoRoot "gauntlet"
$lsGauntletDir = Join-Path $gauntletRoot "linescript"
$cGauntletDir = Join-Path $gauntletRoot "c"
$cppGauntletDir = Join-Path $gauntletRoot "cpp"
$zigGauntletDir = Join-Path $gauntletRoot "zig"

foreach ($path in @($lsGauntletDir, $cGauntletDir, $cppGauntletDir, $zigGauntletDir)) {
  if (-not (Test-Path $path)) { throw "Missing gauntlet path: $path" }
}

$lsFiles = Get-ChildItem -Path $lsGauntletDir -File -Filter *.lsc | Sort-Object Name
if ($lsFiles.Count -eq 0) {
  throw "No .lsc files found in gauntlet/linescript."
}

$cases = New-Object System.Collections.Generic.List[object]
foreach ($ls in $lsFiles) {
  $stem = $ls.BaseName
  $cPath = Join-Path $cGauntletDir ($stem + ".c")
  $cppPath = Join-Path $cppGauntletDir ($stem + ".cpp")
  $zigPath = Join-Path $zigGauntletDir ($stem + ".zig")
  if (-not (Test-Path $cPath)) { throw "Missing gauntlet pair for '$stem': $cPath" }
  if (-not (Test-Path $cppPath)) { throw "Missing gauntlet pair for '$stem': $cppPath" }
  if (-not (Test-Path $zigPath)) { throw "Missing gauntlet pair for '$stem': $zigPath" }

  $cat = switch ($stem.ToLowerInvariant()) {
    "concurrency" { "concurrency" }
    "http" { "http" }
    "compute" { "deterministic" }
    { $_ -like "*edge*" } { "edge_case" }
    default { "gauntlet" }
  }

  $cases.Add([PSCustomObject]@{
    Name = $stem
    Category = $cat
    LSPath = $ls.FullName
    CPath = $cPath
    CppPath = $cppPath
    ZigPath = $zigPath
  })
}

if ($MaxTests -gt 0 -and $MaxTests -lt $cases.Count) {
  $subset = New-Object System.Collections.Generic.List[object]
  foreach ($t in ($cases | Select-Object -First $MaxTests)) { $subset.Add($t) }
  $cases = $subset
}

if (-not [string]::IsNullOrWhiteSpace($CaseFilter)) {
  $filtered = New-Object System.Collections.Generic.List[object]
  foreach ($c in $cases) {
    if ($c.Name -like $CaseFilter) {
      $filtered.Add($c)
    }
  }
  $cases = $filtered
  if ($cases.Count -eq 0) {
    throw "CaseFilter '$CaseFilter' matched zero gauntlet cases."
  }
}

$artifactRoot = Join-Path $repoRoot "tests\artifacts_gauntlet_compare"
if (Test-Path $artifactRoot) {
  Stop-ArtifactProcesses -RootPath $artifactRoot
  Remove-Item -Recurse -Force $artifactRoot
}

$binaryLsDir = Join-Path $artifactRoot "binaries\linescript"
$binaryCDir = Join-Path $artifactRoot "binaries\c"
$binaryCppDir = Join-Path $artifactRoot "binaries\cpp"
$binaryZigDir = Join-Path $artifactRoot "binaries\zig"
$resultDir = Join-Path $artifactRoot "results"

foreach ($d in @($binaryLsDir, $binaryCDir, $binaryCppDir, $binaryZigDir, $resultDir)) {
  Ensure-Directory $d
}

$languages = @("LineScript", "C", "C++", "Zig")
$results = New-Object System.Collections.Generic.List[object]
$failures = New-Object System.Collections.Generic.List[string]

Write-Host "Running gauntlet suite: $($cases.Count) tests (warmup=$WarmupRuns measured=$MeasuredRuns margin=$ConfidenceMarginPercent% perTestDomination=$RequirePerTestDomination edgeNonBlocking=$IncludeEdgeNonBlocking)"

$caseIndex = 0
foreach ($case in $cases) {
  $caseIndex += 1
  Write-Host ("[{0}/{1}] {2}" -f $caseIndex, $cases.Count, $case.Name)

  $lsBinaryPath = Join-Path $binaryLsDir ($case.Name + ".exe")
  $cBinaryPath = Join-Path $binaryCDir ($case.Name + ".exe")
  $cppBinaryPath = Join-Path $binaryCppDir ($case.Name + ".exe")
  $zigBinaryPath = Join-Path $binaryZigDir ($case.Name + ".exe")

  $lsArgs = @($case.LSPath, "--build", "-O4", "--passes", "64", "--backend", "asm", "--cc", $lsBackend, "-o", $lsBinaryPath)
  $lsBuild = Invoke-Lsc -CliArgs $lsArgs
  if ($lsBuild.ExitCode -ne 0) {
    $failures.Add("[build:$($case.Name):LineScript] failed`n$($lsBuild.Output)")
    continue
  }

  $cFlags = @(
    "-O3", "-march=native", "-ffast-math", "-fno-math-errno", "-fomit-frame-pointer", "-funroll-loops", "-std=c11"
  )
  if ($case.Name -eq "http") {
    $cFlags += "-lws2_32"
  }
  $cBuild = Invoke-Compiler -Compiler $cCompilerExe -Flags $cFlags -SourcePath $case.CPath -OutputPath $cBinaryPath
  if ($cBuild.ExitCode -ne 0) {
    $failures.Add("[build:$($case.Name):C] failed`n$($cBuild.Output)")
    continue
  }

  $cppFlags = @(
    "-O3", "-march=native", "-ffast-math", "-fno-math-errno", "-fomit-frame-pointer", "-funroll-loops", "-std=c++20"
  )
  if ($case.Name -eq "http") {
    $cppFlags += "-lws2_32"
  }
  $cppBuild = Invoke-Compiler -Compiler $cppCompilerExe -Flags $cppFlags -SourcePath $case.CppPath -OutputPath $cppBinaryPath
  if ($cppBuild.ExitCode -ne 0) {
    $failures.Add("[build:$($case.Name):C++] failed`n$($cppBuild.Output)")
    continue
  }

  $prev = $ErrorActionPreference
  $ErrorActionPreference = "Continue"
  try {
    $zigArgs = @("build-exe", $case.ZigPath, "-O", "ReleaseFast", "-mcpu", "native", "-femit-bin=$zigBinaryPath", "-fstrip")
    if ($case.Name -eq "http") {
      $zigArgs += "-lws2_32"
    }
    $zigOut = & $zigExe @zigArgs 2>&1 | Out-String
  } finally {
    $ErrorActionPreference = $prev
  }
  if ($LASTEXITCODE -ne 0) {
    $failures.Add("[build:$($case.Name):Zig] failed`n$zigOut")
    continue
  }

  $runMap = @{
    "LineScript" = $lsBinaryPath
    "C" = $cBinaryPath
    "C++" = $cppBinaryPath
    "Zig" = $zigBinaryPath
  }

  $baselineMap = @{}
  $detOk = $true
  foreach ($lang in $languages) {
    $first = $null
    for ($r = 0; $r -lt 3; $r++) {
      $res = Invoke-GauntletCaseTimed -CaseName $case.Name -Command $runMap[$lang] -Arguments @()
      if ($res.ExitCode -ne 0 -or $res.Threw) {
        $failures.Add("[run:$($case.Name):$lang] determinism run failed`n$($res.Output)")
        $detOk = $false
        break
      }
      if (-not $res.StableOk) {
        $failures.Add("[run:$($case.Name):$lang] could not parse stable output: $($res.StableError)`n$($res.Stdout)")
        $detOk = $false
        break
      }
      if ($null -eq $first) { $first = $res.StableValue }
      elseif ($first -ne $res.StableValue) {
        $failures.Add("[run:$($case.Name):$lang] non-deterministic stable output`nfirst=$first next=$($res.StableValue)")
        $detOk = $false
        break
      }
    }
    if (-not $detOk) { break }
    $baselineMap[$lang] = $first
  }
  if (-not $detOk) {
    continue
  }

  $ref = $baselineMap["LineScript"]
  $match = $true
  foreach ($lang in $languages) {
    if ($baselineMap[$lang] -ne $ref) { $match = $false; break }
  }
  if (-not $match) {
    $failures.Add("[run:$($case.Name)] output mismatch across languages")
    continue
  }

  $warmOk = $true
  for ($w = 0; $w -lt $WarmupRuns; $w++) {
    foreach ($lang in $languages) {
      $res = Invoke-GauntletCaseTimed -CaseName $case.Name -Command $runMap[$lang] -Arguments @()
      if ($res.ExitCode -ne 0 -or $res.Threw -or -not $res.StableOk) {
        $failures.Add("[run:$($case.Name):$lang] warmup failed`n$($res.Output)")
        $warmOk = $false
        break
      }
    }
    if (-not $warmOk) { break }
  }
  if (-not $warmOk) {
    continue
  }

  $sampleMap = @{
    "LineScript" = (New-Object System.Collections.Generic.List[double])
    "C" = (New-Object System.Collections.Generic.List[double])
    "C++" = (New-Object System.Collections.Generic.List[double])
    "Zig" = (New-Object System.Collections.Generic.List[double])
  }

  $measureOk = $true
  for ($m = 0; $m -lt $MeasuredRuns; $m++) {
    foreach ($lang in $languages) {
      $res = Invoke-GauntletCaseTimed -CaseName $case.Name -Command $runMap[$lang] -Arguments @()
      if ($res.ExitCode -ne 0 -or $res.Threw -or -not $res.StableOk) {
        $failures.Add("[run:$($case.Name):$lang] measured run failed`n$($res.Output)")
        $measureOk = $false
        break
      }
      if ($res.StableValue -ne $baselineMap[$lang]) {
        $failures.Add("[run:$($case.Name):$lang] measured output drift`nexpected=$($baselineMap[$lang]) got=$($res.StableValue)")
        $measureOk = $false
        break
      }
      $sampleMap[$lang].Add([double]$res.ElapsedMs)
    }
    if (-not $measureOk) { break }
  }
  if (-not $measureOk) {
    continue
  }

  $medianMap = @{}
  foreach ($lang in $languages) {
    $medianMap[$lang] = Get-Median -Values $sampleMap[$lang].ToArray()
  }
  $cmp = New-MultiComparison -MedianMap $medianMap
  $bestNonLs = [Math]::Min([double]$medianMap["C"], [double]$medianMap["C++"])
  $bestNonLs = [Math]::Min($bestNonLs, [double]$medianMap["Zig"])
  $lsMs = [double]$medianMap["LineScript"]
  $lsLeadPercentVsBest = if ($bestNonLs -le 0.0) { 0.0 } else { (($bestNonLs - $lsMs) / $bestNonLs) * 100.0 }
  $dominationPass = ($lsLeadPercentVsBest -ge $ConfidenceMarginPercent)
  $isEdgeCase = ($case.Category -eq "edge_case" -or $case.Category -eq "edge")
  $isBlockingCategory = -not ($IncludeEdgeNonBlocking -and $isEdgeCase)

  $results.Add([PSCustomObject]@{
    Test = $case.Name
    Category = $case.Category
    LineScriptMedianMs = [Math]::Round($medianMap["LineScript"], 6)
    CMedianMs = [Math]::Round($medianMap["C"], 6)
    CppMedianMs = [Math]::Round($medianMap["C++"], 6)
    ZigMedianMs = [Math]::Round($medianMap["Zig"], 6)
    FasterLanguage = $cmp.FasterLanguage
    RunnerUpLanguage = $cmp.RunnerUpLanguage
    SpeedupX = [Math]::Round($cmp.SpeedupX, 6)
    PercentFaster = [Math]::Round($cmp.PercentFaster, 6)
    DeltaMs = [Math]::Round($cmp.DeltaMs, 6)
    BestNonLineScriptMs = [Math]::Round($bestNonLs, 6)
    LineScriptLeadPercentVsBest = [Math]::Round($lsLeadPercentVsBest, 6)
    DominationMarginPercent = [Math]::Round($ConfidenceMarginPercent, 6)
    DominationPass = $dominationPass
    BlockingCategory = $isBlockingCategory
  })
}

$sorted = @($results | Sort-Object Category, Test)
if ($sorted.Count -eq 0) {
  if ($failures.Count -gt 0) {
    Write-Host "Collected failures: $($failures.Count)"
    $preview = [Math]::Min(10, $failures.Count)
    for ($i = 0; $i -lt $preview; $i++) {
      Write-Host "----------------------------------------"
      Write-Host $failures[$i]
    }
  }
  throw "No benchmark results were produced."
}

$overall = @{
  "LineScript" = Get-Median -Values @($sorted | ForEach-Object { [double]$_.LineScriptMedianMs })
  "C" = Get-Median -Values @($sorted | ForEach-Object { [double]$_.CMedianMs })
  "C++" = Get-Median -Values @($sorted | ForEach-Object { [double]$_.CppMedianMs })
  "Zig" = Get-Median -Values @($sorted | ForEach-Object { [double]$_.ZigMedianMs })
}
$overallCmp = New-MultiComparison -MedianMap $overall

$categoryRows = @()
foreach ($g in ($sorted | Group-Object Category | Sort-Object Name)) {
  $cat = @{
    "LineScript" = Get-Median -Values @($g.Group | ForEach-Object { [double]$_.LineScriptMedianMs })
    "C" = Get-Median -Values @($g.Group | ForEach-Object { [double]$_.CMedianMs })
    "C++" = Get-Median -Values @($g.Group | ForEach-Object { [double]$_.CppMedianMs })
    "Zig" = Get-Median -Values @($g.Group | ForEach-Object { [double]$_.ZigMedianMs })
  }
  $cmp = New-MultiComparison -MedianMap $cat
  $categoryRows += [PSCustomObject]@{
    Category = $g.Name
    Tests = $g.Count
    LineScriptMs = [Math]::Round($cat["LineScript"], 6)
    CMs = [Math]::Round($cat["C"], 6)
    CppMs = [Math]::Round($cat["C++"], 6)
    ZigMs = [Math]::Round($cat["Zig"], 6)
    Faster = $cmp.FasterLanguage
    RunnerUp = $cmp.RunnerUpLanguage
    SpeedupX = [Math]::Round($cmp.SpeedupX, 6)
    Percent = [Math]::Round($cmp.PercentFaster, 6)
  }
}

$lsWins = @($sorted | Where-Object { $_.FasterLanguage -eq "LineScript" }).Count
$cWins = @($sorted | Where-Object { $_.FasterLanguage -eq "C" }).Count
$cppWins = @($sorted | Where-Object { $_.FasterLanguage -eq "C++" }).Count
$zigWins = @($sorted | Where-Object { $_.FasterLanguage -eq "Zig" }).Count
$blockingRows = @($sorted | Where-Object { $_.BlockingCategory -eq $true })
$nonBlockingRows = @($sorted | Where-Object { $_.BlockingCategory -ne $true })
$blockingPass = @($blockingRows | Where-Object { $_.DominationPass -eq $true }).Count
$blockingFail = @($blockingRows | Where-Object { $_.DominationPass -ne $true }).Count

$csvPath = Join-Path $resultDir "gauntlet_compare.csv"
$jsonPath = Join-Path $resultDir "gauntlet_compare_summary.json"
$reportPath = Join-Path $resultDir "gauntlet_compare_report.md"

$sorted | Export-Csv -Path $csvPath -NoTypeInformation -Encoding ascii
([PSCustomObject]@{
  generated_utc = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
  warmup_runs = $WarmupRuns
  measured_runs = $MeasuredRuns
  confidence_margin_percent = [Math]::Round($ConfidenceMarginPercent, 6)
  require_per_test_domination = $RequirePerTestDomination
  include_edge_non_blocking = $IncludeEdgeNonBlocking
  total_tests = $cases.Count
  successful_results = $sorted.Count
  failed_results = $failures.Count
  wins = [PSCustomObject]@{
    linescript = $lsWins
    c = $cWins
    cpp = $cppWins
    zig = $zigWins
  }
  domination = [PSCustomObject]@{
    blocking_tests = $blockingRows.Count
    blocking_pass = $blockingPass
    blocking_fail = $blockingFail
    non_blocking_tests = $nonBlockingRows.Count
  }
  overall = [PSCustomObject]@{
    linescript_ms = [Math]::Round($overall["LineScript"], 6)
    c_ms = [Math]::Round($overall["C"], 6)
    cpp_ms = [Math]::Round($overall["C++"], 6)
    zig_ms = [Math]::Round($overall["Zig"], 6)
    faster_language = $overallCmp.FasterLanguage
    runner_up_language = $overallCmp.RunnerUpLanguage
    speedup_x = [Math]::Round($overallCmp.SpeedupX, 6)
    percent_faster = [Math]::Round($overallCmp.PercentFaster, 6)
  }
  categories = $categoryRows
}) | ConvertTo-Json -Depth 8 | Set-Content -Path $jsonPath -Encoding ascii

$sb = New-Object System.Text.StringBuilder
[void]$sb.AppendLine("# Gauntlet Benchmark Report")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("Source folder: gauntlet")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("- Warmup runs: $WarmupRuns")
[void]$sb.AppendLine("- Measured runs: $MeasuredRuns")
[void]$sb.AppendLine("- Domination margin per blocking test: $(Format-Double -Value $ConfidenceMarginPercent -Digits 2)%")
[void]$sb.AppendLine("- Require per-test domination: $RequirePerTestDomination")
[void]$sb.AppendLine("- Edge category treated as non-blocking: $IncludeEdgeNonBlocking")
[void]$sb.AppendLine("- Blocking tests passing domination: $blockingPass / $($blockingRows.Count)")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("| Category | Tests | LineScript ms | C ms | C++ ms | Zig ms | Faster | Runner-up | Speedup x | Percent faster |")
[void]$sb.AppendLine("|---|---:|---:|---:|---:|---:|---|---|---:|---:|")
foreach ($r in ($categoryRows | Sort-Object Category)) {
  [void]$sb.AppendLine("| $($r.Category) | $($r.Tests) | $(Format-Double -Value $r.LineScriptMs) | $(Format-Double -Value $r.CMs) | $(Format-Double -Value $r.CppMs) | $(Format-Double -Value $r.ZigMs) | $($r.Faster) | $($r.RunnerUp) | $(Format-Double -Value $r.SpeedupX -Digits 4) | $(Format-Double -Value $r.Percent -Digits 4)% |")
}
[void]$sb.AppendLine("")
[void]$sb.AppendLine("| Test | Category | LS ms | Best non-LS ms | LS lead % | Domination pass | Blocking |")
[void]$sb.AppendLine("|---|---|---:|---:|---:|---|---|")
foreach ($r in $sorted) {
  [void]$sb.AppendLine("| $($r.Test) | $($r.Category) | $(Format-Double -Value $r.LineScriptMedianMs) | $(Format-Double -Value $r.BestNonLineScriptMs) | $(Format-Double -Value $r.LineScriptLeadPercentVsBest -Digits 4)% | $($r.DominationPass) | $($r.BlockingCategory) |")
}
$sb.ToString() | Set-Content -Path $reportPath -Encoding ascii

Write-Host ""
Write-Host "Gauntlet suite complete."
Write-Host "  Report: $reportPath"
Write-Host "  CSV:    $csvPath"
Write-Host "  JSON:   $jsonPath"
Write-Host ("Overall winner={0}, runner-up={1}, speedup={2}x, LS-wins={3}/{4}, domination(blocking)={5}/{6}" -f `
  $overallCmp.FasterLanguage, `
  $overallCmp.RunnerUpLanguage, `
  (Format-Double -Value $overallCmp.SpeedupX -Digits 4), `
  $lsWins, `
  $sorted.Count, `
  $blockingPass, `
  $blockingRows.Count)

if ($RequirePerTestDomination -and $blockingFail -gt 0) {
  throw "LineScript does not meet per-test domination gate (>= $ConfidenceMarginPercent% lead) on blocking tests: pass=$blockingPass fail=$blockingFail total=$($blockingRows.Count)"
}

if ($failures.Count -gt 0) {
  Write-Host "Failures detected: $($failures.Count)"
  exit 1
}
exit 0
