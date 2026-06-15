#!/usr/bin/env bash
set -euo pipefail

frontend_compiler="clang++"
backend_compiler="clang"
zig_exe=""
warmup_runs=3
measure_runs=9
confidence_margin_percent=2.0
require_per_test_domination=1
include_edge_non_blocking=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --frontend-compiler)
      frontend_compiler="$2"
      shift 2
      ;;
    --backend-compiler)
      backend_compiler="$2"
      shift 2
      ;;
    --zig-exe)
      zig_exe="$2"
      shift 2
      ;;
    --warmup-runs)
      warmup_runs="$2"
      shift 2
      ;;
    --measure-runs)
      measure_runs="$2"
      shift 2
      ;;
    --confidence-margin-percent)
      confidence_margin_percent="$2"
      shift 2
      ;;
    --require-per-test-domination)
      require_per_test_domination="$2"
      shift 2
      ;;
    --include-edge-non-blocking)
      include_edge_non_blocking="$2"
      shift 2
      ;;
    *)
      echo "Unknown option: $1" >&2
      echo "Usage: ./tests/run_zig_comparison.sh [--frontend-compiler clang++] [--backend-compiler clang] [--zig-exe /path/to/zig] [--warmup-runs 3] [--measure-runs 9] [--confidence-margin-percent 2.0] [--require-per-test-domination 1] [--include-edge-non-blocking 1]" >&2
      exit 1
      ;;
  esac
done

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

uname_s="$(uname -s 2>/dev/null || echo unknown)"
case "$uname_s" in
  MINGW*|MSYS*|CYGWIN*)
    exe_suffix=".exe"
    compiler_bin="$root/lsc.exe"
    ;;
  *)
    exe_suffix=""
    compiler_bin="$root/lsc"
    ;;
esac
if [[ -f "$root/lsc" ]]; then
  compiler_bin="$root/lsc"
elif [[ -f "$root/lsc.exe" ]]; then
  compiler_bin="$root/lsc.exe"
fi

normalize_text() {
  local s="$1"
  s="${s//$'\r\n'/$'\n'}"
  s="${s//$'\r'/$'\n'}"
  while [[ "$s" == *$'\n' ]]; do
    s="${s%$'\n'}"
  done
  printf '%s' "$s"
}

now_ms() {
  if has_working_python3; then
    python3 - <<'PY'
import time
print(int(time.time() * 1000))
PY
  elif command -v perl >/dev/null 2>&1; then
    perl -MTime::HiRes=time -e 'printf("%d\n", int(time()*1000))'
  else
    date +%s
  fi
}

avg_list() {
  if [[ $# -eq 0 ]]; then
    printf '0'
    return
  fi
  if has_working_python3; then
    python3 - "$@" <<'PY'
import sys
vals=[float(x) for x in sys.argv[1:]]
print(f"{sum(vals)/len(vals):.3f}")
PY
  else
    awk -v vals="$*" 'BEGIN {
      n = split(vals, a, " ");
      if (n <= 0) { printf "0.000"; exit }
      s = 0.0;
      for (i = 1; i <= n; ++i) s += a[i] + 0.0;
      printf "%.3f", (s / n);
    }'
  fi
}

has_working_python3() {
  if ! command -v python3 >/dev/null 2>&1; then
    return 1
  fi
  python3 - <<'PY' >/dev/null 2>&1
print(1)
PY
}

calc_ratio() {
  awk -v ls="$1" -v zg="$2" 'BEGIN { if (zg > 0) printf "%.3f", (ls / zg); else printf "0.000" }'
}

calc_delta_pct() {
  awk -v ls="$1" -v zg="$2" 'BEGIN { if (zg > 0) printf "%.2f", (((ls - zg) / zg) * 100.0); else printf "0.00" }'
}

calc_lead_pct() {
  awk -v ls="$1" -v zg="$2" 'BEGIN { if (zg > 0) printf "%.2f", (((zg - ls) / zg) * 100.0); else printf "0.00" }'
}

domination_pass() {
  awk -v lead="$1" -v margin="$2" 'BEGIN { if (lead >= margin) print "1"; else print "0" }'
}

build_frontend() {
  local preferred="$1"
  local src="$root/src/lsc.cpp"
  local candidates=()
  [[ -n "$preferred" ]] && candidates+=("$preferred")
  candidates+=("clang++" "g++")
  for cxx in "${candidates[@]}"; do
    if [[ -z "$cxx" ]]; then
      continue
    fi
    if "$cxx" -std=c++20 -O3 -Wall -Wextra -pedantic "$src" -o "$compiler_bin" >/dev/null 2>&1; then
      echo "Frontend build OK with $cxx"
      return
    fi
  done
  echo "Failed to build compiler. Install clang++ or g++." >&2
  exit 1
}

resolve_zig() {
  if [[ -n "$zig_exe" ]]; then
    if [[ -x "$zig_exe" || -f "$zig_exe" ]]; then
      printf '%s' "$zig_exe"
      return
    fi
    echo "Configured Zig executable not found: $zig_exe" >&2
    exit 1
  fi
  if command -v zig >/dev/null 2>&1; then
    command -v zig
    return
  fi
  echo "Zig not found. Install zig and put it in PATH." >&2
  exit 1
}

measure_binary() {
  local exe="$1"
  local expected="$2"
  local warmup="$3"
  local runs="$4"

  local first
  first=$("$exe" 2>&1 || true)
  local actual expect
  actual=$(normalize_text "$first")
  expect=$(normalize_text "$expected")
  if [[ "$actual" != "$expect" ]]; then
    echo "Output mismatch for $exe" >&2
    echo "expected:" >&2
    echo "$expect" >&2
    echo "actual:" >&2
    echo "$actual" >&2
    return 1
  fi

  for ((i=0; i<warmup; i++)); do
    "$exe" >/dev/null 2>&1 || true
  done

  local samples=()
  for ((i=0; i<runs; i++)); do
    local start end
    start="$(now_ms)"
    "$exe" >/dev/null 2>&1 || true
    end="$(now_ms)"
    samples+=("$((end - start))")
  done

  avg_list "${samples[@]}"
}

build_frontend "$frontend_compiler"
zig_path="$(resolve_zig)"
echo "Using Zig: $zig_path"

artifact_dir="$root/tests/artifacts_zig_compare"
rm -rf "$artifact_dir"
mkdir -p "$artifact_dir"

cases=(
  "stress_integer_reduction|tests/stress/stress_integer_reduction.lsc|tests/zig/stress_integer_reduction.zig|49999995000000"
  "stress_mod_pipeline|tests/stress/stress_mod_pipeline.lsc|tests/zig/stress_mod_pipeline.zig|232800000"
  "gcd_walk|tests/stress/gcd_walk.lsc|tests/zig/gcd_walk.zig|15472700"
  "dense_nested|tests/stress/dense_nested.lsc|tests/zig/dense_nested.zig|5480160000"
  "mix_sum|tests/stress/mix_sum.lsc|tests/zig/mix_sum.zig|576005760000800006"
  "blocked_accum|tests/stress/blocked_accum.lsc|tests/zig/blocked_accum.zig|31999996000000"
  "pair_coupled|tests/stress/pair_coupled.lsc|tests/zig/pair_coupled.zig|2666726667300003"
  "alternating_sign|tests/stress/alternating_sign.lsc|tests/zig/alternating_sign.zig|-4000000"
)

results=()
ls_avgs=()
zig_avgs=()
domination_blocking_total=0
domination_blocking_pass=0
domination_non_blocking_total=0

for c in "${cases[@]}"; do
  IFS='|' read -r name ls_src zig_src expected <<<"$c"
  ls_exe="$artifact_dir/ls_${name}${exe_suffix}"
  zig_out="$artifact_dir/zig_${name}${exe_suffix}"

  "$compiler_bin" "$ls_src" --build -O4 --cc "$backend_compiler" -o "$ls_exe" >/dev/null
  "$zig_path" build-exe "$zig_src" -O ReleaseFast -mcpu native -femit-bin="$zig_out" >/dev/null

  ls_avg=$(measure_binary "$ls_exe" "$expected" "$warmup_runs" "$measure_runs")
  zig_avg=$(measure_binary "$zig_out" "$expected" "$warmup_runs" "$measure_runs")

  ratio=$(calc_ratio "$ls_avg" "$zig_avg")
  delta=$(calc_delta_pct "$ls_avg" "$zig_avg")
  lead=$(calc_lead_pct "$ls_avg" "$zig_avg")
  dom_pass=$(domination_pass "$lead" "$confidence_margin_percent")
  is_edge=0
  case_lc="${name,,}"
  if [[ "$case_lc" == *edge* ]]; then
    is_edge=1
  fi
  is_blocking=1
  if [[ "$include_edge_non_blocking" == "1" && "$is_edge" == "1" ]]; then
    is_blocking=0
  fi
  if [[ "$is_blocking" == "1" ]]; then
    domination_blocking_total=$((domination_blocking_total + 1))
    if [[ "$dom_pass" == "1" ]]; then
      domination_blocking_pass=$((domination_blocking_pass + 1))
    fi
  else
    domination_non_blocking_total=$((domination_non_blocking_total + 1))
  fi

  ls_avgs+=("$ls_avg")
  zig_avgs+=("$zig_avg")
  results+=("$name|$ls_avg|$zig_avg|$ratio|$delta|$lead|$dom_pass|$is_blocking")
done

echo
echo "Zig comparison summary (lower is better):"
printf '%-24s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n' "Case" "LS_AvgMs" "Zig_AvgMs" "LS/Zig" "Delta%" "Lead%" "DomPass" "Blocking"
for r in "${results[@]}"; do
  IFS='|' read -r name ls_avg zig_avg ratio delta lead dom_pass is_blocking <<<"$r"
  printf '%-24s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n' "$name" "$ls_avg" "$zig_avg" "$ratio" "$delta" "$lead" "$dom_pass" "$is_blocking"
done

avg_ls=$(avg_list "${ls_avgs[@]}")
avg_zig=$(avg_list "${zig_avgs[@]}")
avg_ratio=$(calc_ratio "$avg_ls" "$avg_zig")
avg_delta=$(calc_delta_pct "$avg_ls" "$avg_zig")

echo
echo "Average LS ms:  $avg_ls"
echo "Average Zig ms: $avg_zig"
echo "Average ratio LS/Zig: $avg_ratio"
echo "Average delta vs Zig: $avg_delta%"
echo "Domination (blocking tests, >=${confidence_margin_percent}% lead): $domination_blocking_pass/$domination_blocking_total"

if [[ "$require_per_test_domination" == "1" && "$domination_blocking_pass" -lt "$domination_blocking_total" ]]; then
  echo "Per-test domination gate failed." >&2
  exit 1
fi
