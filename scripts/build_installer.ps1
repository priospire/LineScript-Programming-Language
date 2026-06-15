param(
  [string]$Compiler = "clang++"
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$src = Join-Path $root "installer\LineScriptSetup.cpp"
$out = Join-Path $root "installer\LineScriptSetup.exe"

if (-not (Test-Path $src)) {
  throw "Missing installer source: $src"
}

function Get-WindowsSdkLibRoot {
  $kitsRoot = "C:\Program Files (x86)\Windows Kits\10\Lib"
  if (-not (Test-Path $kitsRoot)) { return $null }
  $versions = Get-ChildItem $kitsRoot -Directory -ErrorAction SilentlyContinue |
    Sort-Object Name -Descending
  foreach ($v in $versions) {
    $um = Join-Path $v.FullName "um\x64"
    $ucrt = Join-Path $v.FullName "ucrt\x64"
    if ((Test-Path (Join-Path $um "user32.lib")) -and (Test-Path (Join-Path $ucrt "ucrt.lib"))) {
      return [PSCustomObject]@{ Um = $um; Ucrt = $ucrt }
    }
  }
  return $null
}

$sdk = Get-WindowsSdkLibRoot
if ($null -eq $sdk) {
  throw "Windows SDK x64 libraries were not found. Install Windows SDK or build from a Developer Command Prompt."
}

$args = @(
  "-std=c++20",
  "-O2",
  "-Wall",
  "-Wextra",
  "-pedantic",
  $src,
  "-o",
  $out,
  "-fuse-ld=lld",
  "-Wl,/entry:mainCRTStartup",
  "-Wl,/libpath:$($sdk.Um)",
  "-Wl,/libpath:$($sdk.Ucrt)",
  (Join-Path $sdk.Um "shell32.lib"),
  (Join-Path $sdk.Um "ole32.lib"),
  (Join-Path $sdk.Um "user32.lib"),
  (Join-Path $sdk.Um "gdi32.lib")
)

& $Compiler @args
if ($LASTEXITCODE -ne 0) {
  throw "Failed to build LineScriptSetup.exe."
}

Write-Host "Built installer: $out"
