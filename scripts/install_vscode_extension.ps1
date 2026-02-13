param(
  [switch]$SkipNpmInstall
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$extDir = Join-Path $root "vscode-extension\linescript-vscode"

if (-not (Test-Path $extDir)) {
  throw "Missing extension directory: $extDir"
}

$codeCmd = Get-Command code -ErrorAction SilentlyContinue
if ($null -eq $codeCmd) {
  throw "VS Code CLI 'code' not found in PATH. In VS Code, run: 'Shell Command: Install 'code' command in PATH'."
}
$npmCmd = Get-Command npm.cmd -ErrorAction SilentlyContinue
if ($null -eq $npmCmd) {
  $npmCmd = Get-Command npm -ErrorAction SilentlyContinue
}
if ($null -eq $npmCmd) {
  throw "npm not found in PATH."
}

Push-Location $extDir
try {
  if (-not $SkipNpmInstall) {
    & $npmCmd.Source install
    if ($LASTEXITCODE -ne 0) { throw "npm install failed." }
  }

  & $npmCmd.Source run package:vsix
  if ($LASTEXITCODE -ne 0) { throw "VSIX packaging failed." }

  $vsixPath = Join-Path $extDir "linescript-vscode.vsix"
  if (-not (Test-Path $vsixPath)) {
    throw "VSIX not found: $vsixPath"
  }

  & $codeCmd.Source --install-extension $vsixPath --force
  if ($LASTEXITCODE -ne 0) { throw "VSIX install failed." }
} finally {
  Pop-Location
}

Write-Host "Installed LineScript VSCode extension:"
Write-Host "  $vsixPath"
Write-Host "If VS Code was open, run 'Developer: Reload Window'."
