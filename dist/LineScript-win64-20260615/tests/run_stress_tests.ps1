param(
  [string]$FrontendCompiler = "clang++",
  [string]$BackendCompiler = "clang",
  [switch]$KeepArtifacts
)

$ErrorActionPreference = "Stop"
if ($null -ne (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue)) {
  $PSNativeCommandUseErrorActionPreference = $false
}

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

function Normalize-Text {
  param([string]$Text)
  if ($null -eq $Text) { return "" }
  $x = $Text -replace "`r`n", "`n"
  $x = $x -replace "`r", "`n"
  return $x.TrimEnd("`n")
}

function Build-Frontend {
  param([string]$Preferred)
  $compilerSrc = Join-Path $root "src\\lsc.cpp"
  $compilerExe = Join-Path $root "lsc.exe"
  if (-not (Test-Path $compilerSrc)) {
    if (Test-Path $compilerExe) {
      Write-Host "Using existing frontend binary: lsc.exe"
      return
    }
    throw "Missing both src\\lsc.cpp and lsc.exe."
  }
  foreach ($cxx in @($Preferred, "clang++", "g++")) {
    if ([string]::IsNullOrWhiteSpace($cxx)) { continue }
    try {
      & $cxx -std=c++20 -O3 -Wall -Wextra -pedantic $compilerSrc -o $compilerExe 2>&1 | Out-Null
      if ($LASTEXITCODE -eq 0) {
        Write-Host "Frontend build OK with $cxx"
        return
      }
    } catch {
      # try next compiler
    }
  }
  throw "Failed to build lsc.exe. Install clang++ or g++."
}

function Run-Lsc {
  param([string[]]$CliArgs)
  $prev = $ErrorActionPreference
  $ErrorActionPreference = "Continue"
  try {
    $out = & .\lsc.exe @CliArgs 2>&1 | Out-String
  } finally {
    $ErrorActionPreference = $prev
  }
  return [PSCustomObject]@{
    Output = $out
    ExitCode = $LASTEXITCODE
  }
}

Build-Frontend -Preferred $FrontendCompiler

$artifactDir = Join-Path $root "tests\\artifacts_stress"
if (Test-Path $artifactDir) {
  Remove-Item -Recurse -Force $artifactDir
}
New-Item -ItemType Directory -Path $artifactDir | Out-Null

$tests = @(
  [PSCustomObject]@{
    Name = "stress_integer_reduction"
    Source = "tests\\stress\\stress_integer_reduction.lsc"
    Expected = "49999995000000"
  },
  [PSCustomObject]@{
    Name = "stress_mod_pipeline"
    Source = "tests\\stress\\stress_mod_pipeline.lsc"
    Expected = "232800000"
  },
  [PSCustomObject]@{
    Name = "stress_parallel_independent"
    Source = "tests\\stress\\stress_parallel_independent.lsc"
    Expected = "55999974000000"
  },
  [PSCustomObject]@{
    Name = "stress_collections_pipeline"
    Source = "tests\\stress\\stress_collections_pipeline.lsc"
    Expected = "199990000"
  },
  [PSCustomObject]@{
    Name = "stress_memory_reuse"
    Source = "tests\\stress\\stress_memory_reuse.lsc"
    Expected = "0`n1056000"
  },
  [PSCustomObject]@{
    Name = "stress_task_spawn_reuse"
    Source = "tests\\stress\\stress_task_spawn_reuse.lsc"
    Expected = "0`n6400"
  },
  [PSCustomObject]@{
    Name = "stress_http_burst_roundtrip"
    Source = "tests\\stress\\stress_http_burst_roundtrip.lsc"
    Expected = "160"
  },
  [PSCustomObject]@{
    Name = "stress_edge_guarded_ranges"
    Source = "tests\\stress\\stress_edge_guarded_ranges.lsc"
    Expected = "22412"
  },
  [PSCustomObject]@{
    Name = "gcd_walk"
    Source = "tests\\stress\\gcd_walk.lsc"
    Expected = "15472700"
  },
  [PSCustomObject]@{
    Name = "dense_nested"
    Source = "tests\\stress\\dense_nested.lsc"
    Expected = "5480160000"
  },
  [PSCustomObject]@{
    Name = "mix_sum"
    Source = "tests\\stress\\mix_sum.lsc"
    Expected = "576005760000800006"
  },
  [PSCustomObject]@{
    Name = "blocked_accum"
    Source = "tests\\stress\\blocked_accum.lsc"
    Expected = "31999996000000"
  },
  [PSCustomObject]@{
    Name = "pair_coupled"
    Source = "tests\\stress\\pair_coupled.lsc"
    Expected = "2666726667300003"
  },
  [PSCustomObject]@{
    Name = "alternating_sign"
    Source = "tests\\stress\\alternating_sign.lsc"
    Expected = "-4000000"
  },
  [PSCustomObject]@{
    Name = "graphics_raster_sum"
    Source = "tests\\stress\\graphics_raster_sum.lsc"
    Expected = "549755781120"
  },
  [PSCustomObject]@{
    Name = "graphics_interop_lines"
    Source = "tests\\stress\\graphics_interop_lines.lsc"
    Expected = "133126"
  },
  [PSCustomObject]@{
    Name = "game_capacity_alloc"
    Source = "tests\\stress\\game_capacity_alloc.lsc"
    Expected = "32`n-1"
  },
  [PSCustomObject]@{
    Name = "game_render_pipeline"
    Source = "tests\\stress\\game_render_pipeline.lsc"
    Expected = "124302463`n240`nfalse"
  },
  [PSCustomObject]@{
    Name = "vectorization_probe"
    Source = "tests\\stress\\vectorization_probe.lsc"
    Expected = "499999500000"
  },
  [PSCustomObject]@{
    Name = "physics_capacity_walk"
    Source = "tests\\stress\\physics_capacity_walk.lsc"
    Expected = "8386561"
  }
)

$failures = New-Object System.Collections.Generic.List[string]
$passed = 0
$total = 0
$timings = New-Object System.Collections.Generic.List[string]

foreach ($t in $tests) {
  $total += 1
  $exe = Join-Path $artifactDir ($t.Name + ".exe")
  $build = Run-Lsc -CliArgs @($t.Source, "--build", "--max-speed", "--cc", $BackendCompiler, "-o", $exe)
  if ($build.ExitCode -ne 0) {
    $failures.Add("[stress:$($t.Name)] build failed:`n$($build.Output)")
    continue
  }

  $programOutput = ""
  $elapsed = Measure-Command {
    $programOutput = & $exe 2>&1 | Out-String
  }
  $actual = Normalize-Text $programOutput
  $expected = Normalize-Text $t.Expected
  if ($actual -ne $expected) {
    $failures.Add("[stress:$($t.Name)] output mismatch`nexpected:`n$expected`nactual:`n$actual")
    continue
  }
  $passed += 1
  $timings.Add("[stress:$($t.Name)] elapsed_ms=$([math]::Round($elapsed.TotalMilliseconds, 3))")
}

$backendLower = $BackendCompiler.ToLowerInvariant()
$canVectorReport = $backendLower.Contains("clang")
if ($canVectorReport) {
  $total += 1
  $vecExe = Join-Path $artifactDir "vectorization_report_probe.exe"
  $buildVec = Run-Lsc -CliArgs @("tests\\stress\\vectorization_probe.lsc", "--build", "--max-speed", "--cc", $BackendCompiler, "--keep-c", "-o", $vecExe)
  if ($buildVec.ExitCode -ne 0) {
    $failures.Add("[vectorization:report] build failed:`n$($buildVec.Output)")
  } else {
    $vecC = "$vecExe.c"
    if (-not (Test-Path $vecC)) {
      $failures.Add("[vectorization:report] missing generated C file: $vecC")
    } else {
      $vecObj = "$vecExe.vec.o"
      $prev = $ErrorActionPreference
      $ErrorActionPreference = "Continue"
      try {
        $vecReport = & $BackendCompiler -O3 -fopenmp -Rpass=loop-vectorize -Rpass-missed=loop-vectorize -c $vecC -o $vecObj 2>&1 | Out-String
      } finally {
        $ErrorActionPreference = $prev
      }
      if ($LASTEXITCODE -ne 0) {
        $failures.Add("[vectorization:report] clang report compile failed:`n$vecReport")
      } else {
        $reportLower = $vecReport.ToLowerInvariant()
        $hasVectorRemark = $reportLower.Contains("vectorized loop")
        $probeSource = Get-Content $vecC -Raw
        $hasSimdDirective = $probeSource.Contains("LS_OMP_SIMD_REDUCTION_PLUS(") -or $probeSource.Contains("LS_OMP_SIMD")
        if ($hasVectorRemark -or $hasSimdDirective) {
          $passed += 1
          if ($hasVectorRemark) {
            $timings.Add("[vectorization:report] vectorized loop remark detected")
          } else {
            $timings.Add("[vectorization:report] omp simd directive detected in probe loop")
          }
        } else {
          $failures.Add("[vectorization:report] no vectorized loop remark or simd directive detected.`nreport:`n$vecReport")
        }
      }
    }
  }
} else {
  $timings.Add("[vectorization:report] skipped (backend compiler is not clang-compatible for -Rpass)")
}

Write-Host ""
Write-Host "Stress summary: $passed / $total passed"
foreach ($line in $timings) {
  Write-Host $line
}

if ($failures.Count -gt 0) {
  Write-Host ""
  Write-Host "Stress failures:"
  foreach ($f in $failures) {
    Write-Host "----------------------------------------"
    Write-Host $f
  }
  exit 1
}

if (-not $KeepArtifacts) {
  Remove-Item -Recurse -Force $artifactDir
}

Write-Host "All deterministic stress tests passed."
exit 0
