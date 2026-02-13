param(
  [string]$OutDir = "dist",
  [string]$Compiler = "clang++",
  [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$compilerSrc = Join-Path $root "src\\lsc.cpp"
$compilerExe = Join-Path $root "lsc.exe"

if (-not (Test-Path $compilerSrc)) {
  throw "Missing compiler source: $compilerSrc"
}

function Build-Lsc {
  param([string]$Preferred)
  foreach ($cxx in @($Preferred, "clang++", "g++")) {
    if ([string]::IsNullOrWhiteSpace($cxx)) { continue }
    try {
      & $cxx -std=c++20 -O3 -Wall -Wextra -pedantic $compilerSrc -o $compilerExe
      if ($LASTEXITCODE -eq 0) {
        Write-Host "Built compiler with $cxx"
        return
      }
    } catch {
      # Try next compiler.
    }
  }
  throw "Failed to build lsc.exe. Install clang++ or g++ and retry."
}

if (-not $SkipBuild) {
  Build-Lsc -Preferred $Compiler
} elseif (-not (Test-Path $compilerExe)) {
  throw "SkipBuild requested but missing: $compilerExe"
}

$dateTag = Get-Date -Format "yyyyMMdd"
$bundleDir = Join-Path (Join-Path $root $OutDir) "LineScript-win64-$dateTag"
$zipPath = "$bundleDir.zip"

if (Test-Path $bundleDir) { Remove-Item -Recurse -Force $bundleDir }
if (Test-Path $zipPath) { Remove-Item -Force $zipPath }
New-Item -ItemType Directory -Path $bundleDir | Out-Null

Copy-Item $compilerExe (Join-Path $bundleDir "lsc.exe")
Copy-Item $compilerSrc (Join-Path $bundleDir "lsc.cpp")
Copy-Item (Join-Path $root "linescript.ps1") (Join-Path $bundleDir "linescript.ps1")
Copy-Item (Join-Path $root "linescript.cmd") (Join-Path $bundleDir "linescript.cmd")
if (Test-Path (Join-Path $root "linescript.sh")) {
  Copy-Item (Join-Path $root "linescript.sh") (Join-Path $bundleDir "linescript.sh")
}
Copy-Item (Join-Path $root "README.md") (Join-Path $bundleDir "README.md")

$docsOut = Join-Path $bundleDir "docs"
$examplesOut = Join-Path $bundleDir "examples"
$testsOut = Join-Path $bundleDir "tests"
$scriptsOut = Join-Path $bundleDir "scripts"
$vscodeOut = Join-Path $bundleDir ".vscode"
$vscodeExtOut = Join-Path $bundleDir "vscode-extension"
New-Item -ItemType Directory -Path $docsOut | Out-Null
New-Item -ItemType Directory -Path $examplesOut | Out-Null
New-Item -ItemType Directory -Path $testsOut | Out-Null
New-Item -ItemType Directory -Path $scriptsOut | Out-Null
New-Item -ItemType Directory -Path $vscodeOut | Out-Null
New-Item -ItemType Directory -Path $vscodeExtOut | Out-Null

Get-ChildItem (Join-Path $root "docs") -File | Where-Object { $_.Extension -eq ".md" } |
  Copy-Item -Destination $docsOut
Get-ChildItem (Join-Path $root "examples") -File |
  Where-Object { $_.Extension -eq ".lsc" -or $_.Extension -eq ".ls" } |
  Copy-Item -Destination $examplesOut
Get-ChildItem (Join-Path $root "tests") -File |
  Where-Object { $_.Extension -eq ".ps1" -or $_.Extension -eq ".sh" -or $_.Extension -eq ".md" } |
  Copy-Item -Destination $testsOut
Copy-Item (Join-Path $root "tests\\cases") (Join-Path $testsOut "cases") -Recurse
if (Test-Path (Join-Path $root "tests\\stress")) {
  $stressOut = Join-Path $testsOut "stress"
  New-Item -ItemType Directory -Path $stressOut | Out-Null
  Get-ChildItem (Join-Path $root "tests\\stress") -File |
    Where-Object { $_.Extension -eq ".lsc" } |
    Copy-Item -Destination $stressOut
}
if (Test-Path (Join-Path $root "tests\\zig")) {
  $zigOut = Join-Path $testsOut "zig"
  New-Item -ItemType Directory -Path $zigOut | Out-Null
  Get-ChildItem (Join-Path $root "tests\\zig") -File |
    Where-Object { $_.Extension -eq ".zig" } |
    Copy-Item -Destination $zigOut
}
if (Test-Path (Join-Path $root ".vscode\\tasks.json")) {
  Copy-Item (Join-Path $root ".vscode\\tasks.json") (Join-Path $vscodeOut "tasks.json")
}
if (Test-Path (Join-Path $root ".vscode\\launch.json")) {
  Copy-Item (Join-Path $root ".vscode\\launch.json") (Join-Path $vscodeOut "launch.json")
}
if (Test-Path (Join-Path $root ".vscode\\settings.json")) {
  Copy-Item (Join-Path $root ".vscode\\settings.json") (Join-Path $vscodeOut "settings.json")
}
if (Test-Path (Join-Path $root ".vscode\\extensions.json")) {
  Copy-Item (Join-Path $root ".vscode\\extensions.json") (Join-Path $vscodeOut "extensions.json")
}
foreach ($scriptName in @("install_vscode_extension.ps1", "install_vscode_extension.sh", "package.ps1", "package.sh")) {
  $srcScript = Join-Path $root ("scripts\\" + $scriptName)
  if (Test-Path $srcScript) {
    Copy-Item $srcScript (Join-Path $scriptsOut $scriptName)
  }
}
if (Test-Path (Join-Path $root "vscode-extension\\linescript-vscode")) {
  $extDst = Join-Path $vscodeExtOut "linescript-vscode"
  Copy-Item (Join-Path $root "vscode-extension\\linescript-vscode") $extDst -Recurse
  $extNodeModules = Join-Path $extDst "node_modules"
  if (Test-Path $extNodeModules) {
    Remove-Item -Recurse -Force $extNodeModules
  }
}

Compress-Archive -Path "$bundleDir\\*" -DestinationPath $zipPath

Write-Host "Package directory: $bundleDir"
Write-Host "Package zip: $zipPath"
