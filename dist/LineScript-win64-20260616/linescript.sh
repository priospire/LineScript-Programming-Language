#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
compiler_src_primary="$root/src/lsc.cpp"
compiler_src_fallback="$root/lsc.cpp"

uname_s="$(uname -s 2>/dev/null || echo unknown)"
case "$uname_s" in
  MINGW*|MSYS*|CYGWIN*) default_compiler_bin="$root/lsc.exe" ;;
  *) default_compiler_bin="$root/lsc" ;;
esac

if [[ -x "$root/lsc" || -f "$root/lsc" ]]; then
  compiler_bin="$root/lsc"
elif [[ -x "$root/lsc.exe" || -f "$root/lsc.exe" ]]; then
  compiler_bin="$root/lsc.exe"
else
  compiler_bin="$default_compiler_bin"
fi

compiler_src=""
if [[ -f "$compiler_src_primary" ]]; then
  compiler_src="$compiler_src_primary"
elif [[ -f "$compiler_src_fallback" ]]; then
  compiler_src="$compiler_src_fallback"
fi

has_src=0
[[ -n "$compiler_src" ]] && has_src=1
has_bin=0
[[ -f "$compiler_bin" ]] && has_bin=1

if [[ $has_src -eq 0 && $has_bin -eq 0 ]]; then
  echo "Missing compiler binary ($compiler_bin) and source ($compiler_src_primary or $compiler_src_fallback)." >&2
  exit 1
fi

needs_build=0
if [[ $has_src -eq 1 ]]; then
  needs_build=1
  if [[ $has_bin -eq 1 && "$compiler_bin" -nt "$compiler_src" ]]; then
    needs_build=0
  fi
fi

if [[ $needs_build -eq 1 ]]; then
  preferred="${LS_FRONTEND_COMPILER:-}"
  candidates=()
  if [[ -n "$preferred" ]]; then candidates+=("$preferred"); fi
  candidates+=("clang++" "g++")
  built=0
  for cxx in "${candidates[@]}"; do
    if [[ -z "$cxx" ]]; then
      continue
    fi
    if "$cxx" -std=c++20 -O3 -Wall -Wextra -pedantic "$compiler_src" -o "$compiler_bin" >/dev/null 2>&1; then
      built=1
      break
    fi
  done
  if [[ $built -eq 0 ]]; then
    echo "Failed to build compiler. Install clang++ or g++ and retry." >&2
    exit 1
  fi
fi

has_input=0
has_action=0
wants_help=0
wants_repl=0
for arg in "$@"; do
  if [[ "$arg" == "--help" || "$arg" == "-h" ]]; then
    wants_help=1
  fi
  if [[ "$arg" == "--repl" || "$arg" == "--shell" ]]; then
    wants_repl=1
  fi
  if [[ "$arg" == "--check" || "$arg" == "--build" || "$arg" == "--run" ]]; then
    has_action=1
  fi
  if [[ "$arg" != -* ]]; then
    case "${arg##*.}" in
      lsc|ls) has_input=1 ;;
    esac
  fi
done

if [[ $# -eq 0 ]]; then
  wants_repl=1
fi

if [[ $has_input -eq 0 && $wants_help -eq 0 && $wants_repl -eq 0 ]]; then
  echo "No .lsc/.ls input file found in arguments." >&2
  exit 1
fi

if [[ $# -eq 0 ]]; then
  final_args=(--repl)
else
  final_args=("$@")
fi
if [[ $has_input -eq 1 && $has_action -eq 0 ]]; then
  final_args+=("--build" "--run")
fi

"$compiler_bin" "${final_args[@]}"
