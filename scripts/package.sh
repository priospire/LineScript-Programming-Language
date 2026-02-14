#!/usr/bin/env bash
set -euo pipefail

out_dir="dist"
compiler="clang++"
skip_build=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --out-dir)
      out_dir="$2"
      shift 2
      ;;
    --compiler)
      compiler="$2"
      shift 2
      ;;
    --skip-build)
      skip_build=1
      shift
      ;;
    *)
      echo "Unknown option: $1" >&2
      echo "Usage: ./scripts/package.sh [--out-dir dist] [--compiler clang++] [--skip-build]" >&2
      exit 1
      ;;
  esac
done

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
compiler_src="$root/src/lsc.cpp"

uname_s="$(uname -s 2>/dev/null || echo unknown)"
case "$uname_s" in
  MINGW*|MSYS*|CYGWIN*)
    platform_tag="win64"
    compiler_bin="$root/lsc.exe"
    ;;
  Linux*)
    platform_tag="linux"
    compiler_bin="$root/lsc"
    ;;
  Darwin*)
    platform_tag="macos"
    compiler_bin="$root/lsc"
    ;;
  *)
    platform_tag="unknown"
    compiler_bin="$root/lsc"
    ;;
esac

if [[ ! -f "$compiler_src" ]]; then
  echo "Missing compiler source: $compiler_src" >&2
  exit 1
fi

build_lsc() {
  local preferred="$1"
  local candidates=()
  if [[ -n "$preferred" ]]; then candidates+=("$preferred"); fi
  candidates+=("clang++" "g++")

  for cxx in "${candidates[@]}"; do
    if [[ -z "$cxx" ]]; then
      continue
    fi
    if "$cxx" -std=c++20 -O3 -Wall -Wextra -pedantic "$compiler_src" -o "$compiler_bin"; then
      echo "Built compiler with $cxx"
      return 0
    fi
  done
  echo "Failed to build compiler. Install clang++ or g++ and retry." >&2
  return 1
}

if [[ $skip_build -eq 0 ]]; then
  build_lsc "$compiler"
else
  if [[ ! -f "$compiler_bin" ]]; then
    echo "--skip-build requested but missing: $compiler_bin" >&2
    exit 1
  fi
fi

date_tag="$(date +%Y%m%d)"
bundle_dir="$root/$out_dir/LineScript-$platform_tag-$date_tag"
zip_path="$bundle_dir.zip"

auto_cleanup() {
  :
}
trap auto_cleanup EXIT

rm -rf "$bundle_dir"
rm -f "$zip_path"
mkdir -p "$bundle_dir"

cp "$compiler_bin" "$bundle_dir/$(basename "$compiler_bin")"
cp "$compiler_src" "$bundle_dir/lsc.cpp"

[[ -f "$root/linescript.ps1" ]] && cp "$root/linescript.ps1" "$bundle_dir/linescript.ps1"
[[ -f "$root/linescript.cmd" ]] && cp "$root/linescript.cmd" "$bundle_dir/linescript.cmd"
[[ -f "$root/linescript.sh" ]] && cp "$root/linescript.sh" "$bundle_dir/linescript.sh"
cp "$root/README.md" "$bundle_dir/README.md"

mkdir -p "$bundle_dir/docs" "$bundle_dir/examples" "$bundle_dir/tests" "$bundle_dir/scripts" "$bundle_dir/.vscode"
mkdir -p "$bundle_dir/vscode-extension"
mkdir -p "$bundle_dir/editor-configs"

if [[ -d "$root/docs" ]]; then
  find "$root/docs" -maxdepth 1 -type f -name "*.md" -exec cp {} "$bundle_dir/docs/" \;
fi
if [[ -d "$root/examples" ]]; then
  find "$root/examples" -maxdepth 1 -type f \( -name "*.lsc" -o -name "*.ls" \) -exec cp {} "$bundle_dir/examples/" \;
fi
if [[ -d "$root/tests" ]]; then
  find "$root/tests" -maxdepth 1 -type f \( -name "*.ps1" -o -name "*.sh" -o -name "*.md" \) -exec cp {} "$bundle_dir/tests/" \;
fi
[[ -d "$root/tests/cases" ]] && cp -r "$root/tests/cases" "$bundle_dir/tests/cases"
if [[ -d "$root/tests/stress" ]]; then
  mkdir -p "$bundle_dir/tests/stress"
  find "$root/tests/stress" -maxdepth 1 -type f -name "*.lsc" -exec cp {} "$bundle_dir/tests/stress/" \;
fi
if [[ -d "$root/tests/zig" ]]; then
  mkdir -p "$bundle_dir/tests/zig"
  find "$root/tests/zig" -maxdepth 1 -type f -name "*.zig" -exec cp {} "$bundle_dir/tests/zig/" \;
fi

[[ -f "$root/.vscode/tasks.json" ]] && cp "$root/.vscode/tasks.json" "$bundle_dir/.vscode/tasks.json"
[[ -f "$root/.vscode/launch.json" ]] && cp "$root/.vscode/launch.json" "$bundle_dir/.vscode/launch.json"
[[ -f "$root/.vscode/settings.json" ]] && cp "$root/.vscode/settings.json" "$bundle_dir/.vscode/settings.json"
[[ -f "$root/.vscode/extensions.json" ]] && cp "$root/.vscode/extensions.json" "$bundle_dir/.vscode/extensions.json"
for script_name in install_vscode_extension.ps1 install_vscode_extension.sh package.ps1 package.sh; do
  [[ -f "$root/scripts/$script_name" ]] && cp "$root/scripts/$script_name" "$bundle_dir/scripts/$script_name"
done
if [[ -d "$root/vscode-extension/linescript-vscode" ]]; then
  cp -r "$root/vscode-extension/linescript-vscode" "$bundle_dir/vscode-extension/linescript-vscode"
  rm -rf "$bundle_dir/vscode-extension/linescript-vscode/node_modules"
fi
if [[ -d "$root/editor-configs" ]]; then
  cp -r "$root/editor-configs/." "$bundle_dir/editor-configs/"
fi

if command -v zip >/dev/null 2>&1; then
  (
    cd "$bundle_dir"
    zip -qr "$zip_path" .
  )
else
  echo "zip command not found; skipping zip creation." >&2
fi

echo "Package directory: $bundle_dir"
if [[ -f "$zip_path" ]]; then
  echo "Package zip: $zip_path"
fi
