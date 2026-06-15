#!/usr/bin/env bash
set -euo pipefail

skip_npm_install=0
if [[ "${1:-}" == "--skip-npm-install" ]]; then
  skip_npm_install=1
fi

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ext_dir="$root/vscode-extension/linescript-vscode"

if [[ ! -d "$ext_dir" ]]; then
  echo "Missing extension directory: $ext_dir" >&2
  exit 1
fi

if ! command -v code >/dev/null 2>&1; then
  echo "VS Code CLI 'code' not found in PATH." >&2
  echo "Open VS Code and run: 'Shell Command: Install code command in PATH'." >&2
  exit 1
fi

pushd "$ext_dir" >/dev/null
if [[ $skip_npm_install -eq 0 ]]; then
  npm install
fi

npm run package:vsix
vsix_path="$ext_dir/linescript-vscode.vsix"
if [[ ! -f "$vsix_path" ]]; then
  echo "VSIX not found: $vsix_path" >&2
  exit 1
fi

code --install-extension "$vsix_path" --force
popd >/dev/null

echo "Installed LineScript VSCode extension:"
echo "  $vsix_path"
echo "If VS Code was open, run: Developer: Reload Window"

