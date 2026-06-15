$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$compilerSrcPrimary = Join-Path $root "src\\lsc.cpp"
$compilerSrcFallback = Join-Path $root "lsc.cpp"
$compilerExe = Join-Path $root "lsc.exe"

$compilerSrc = $null
foreach ($candidate in @($compilerSrcPrimary, $compilerSrcFallback)) {
  if (Test-Path $candidate) {
    $compilerSrc = $candidate
    break
  }
}
$hasSrc = $null -ne $compilerSrc
$hasExe = Test-Path $compilerExe
if (-not $hasSrc -and -not $hasExe) {
  throw "Missing compiler binary ($compilerExe) and source ($compilerSrcPrimary or $compilerSrcFallback)."
}

$needsBuild = $false
if ($hasSrc) {
  $needsBuild = $true
  if ($hasExe) {
    $srcTime = (Get-Item $compilerSrc).LastWriteTimeUtc
    $exeTime = (Get-Item $compilerExe).LastWriteTimeUtc
    if ($exeTime -ge $srcTime) {
      $needsBuild = $false
    }
  }
}

if ($needsBuild) {
  $built = $false
  foreach ($cxx in @("clang++", "g++")) {
    try {
      & $cxx -std=c++20 -O3 -Wall -Wextra -pedantic $compilerSrc -o $compilerExe
      if ($LASTEXITCODE -eq 0) {
        $built = $true
        break
      }
    } catch {
      # Try next compiler.
    }
  }
  if (-not $built) {
    throw "Failed to build lsc.exe. Install clang++ or g++ and retry."
  }
}

$hasInput = $false
$hasAction = $false
$wantsHelp = $false
$wantsRepl = $false
foreach ($arg in $args) {
  if ($arg -eq "--help" -or $arg -eq "-h") {
    $wantsHelp = $true
  }
  if ($arg -eq "--repl" -or $arg -eq "--shell") {
    $wantsRepl = $true
  }
  if ($arg -eq "--check") {
    $hasAction = $true
  }
  if ($arg -eq "--build" -or $arg -eq "--run") {
    $hasAction = $true
  }
  if (-not $arg.StartsWith("-")) {
    $ext = [System.IO.Path]::GetExtension($arg).ToLowerInvariant()
    if ($ext -eq ".lsc" -or $ext -eq ".ls") {
      $hasInput = $true
    }
  }
}

if ($args.Count -eq 0) {
  $wantsRepl = $true
}

if ((-not $hasInput) -and (-not $wantsHelp) -and (-not $wantsRepl)) {
  throw "No .lsc/.ls input file found in arguments."
}

$finalArgs = if ($args.Count -eq 0) { @("--repl") } else { @($args) }
if ($hasInput -and (-not $hasAction)) {
  $finalArgs += "--build"
  $finalArgs += "--run"
}

& $compilerExe @finalArgs
exit $LASTEXITCODE
