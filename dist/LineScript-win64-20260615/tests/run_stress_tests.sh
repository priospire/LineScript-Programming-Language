#!/usr/bin/env bash
set -euo pipefail

frontend_compiler="clang++"
backend_compiler="clang"
keep_artifacts=0

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
    --keep-artifacts)
      keep_artifacts=1
      shift
      ;;
    *)
      echo "Unknown option: $1" >&2
      echo "Usage: ./tests/run_stress_tests.sh [--frontend-compiler clang++] [--backend-compiler clang] [--keep-artifacts]" >&2
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
  if command -v python3 >/dev/null 2>&1 && python3 -c 'import time; print(0)' >/dev/null 2>&1; then
    python3 - <<'PY'
import time
print(int(time.time() * 1000))
PY
  elif command -v python >/dev/null 2>&1 && python -c 'import time; print(0)' >/dev/null 2>&1; then
    python - <<'PY'
import time
print(int(time.time() * 1000))
PY
  elif command -v perl >/dev/null 2>&1; then
    perl -MTime::HiRes=time -e 'printf("%d\n", int(time()*1000))'
  elif date +%s%3N >/dev/null 2>&1; then
    date +%s%3N
  else
    printf '%s000\n' "$(date +%s)"
  fi
}

build_frontend() {
  local preferred="$1"
  local src="$root/src/lsc.cpp"
  if [[ ! -f "$src" ]]; then
    if [[ -f "$compiler_bin" ]]; then
      echo "Using existing frontend binary: $(basename "$compiler_bin")"
      return
    fi
    echo "Missing both src/lsc.cpp and compiler binary." >&2
    exit 1
  fi

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

run_lsc() {
  local out
  out=$("$compiler_bin" "$@" 2>&1) || {
    printf '%s' "$out"
    return 1
  }
  printf '%s' "$out"
}

build_frontend "$frontend_compiler"

artifact_dir="$root/tests/artifacts_stress"
rm -rf "$artifact_dir"
mkdir -p "$artifact_dir"

tests=(
  "stress_integer_reduction|tests/stress/stress_integer_reduction.lsc|49999995000000"
  "stress_mod_pipeline|tests/stress/stress_mod_pipeline.lsc|232800000"
  "stress_parallel_independent|tests/stress/stress_parallel_independent.lsc|55999974000000"
  "stress_collections_pipeline|tests/stress/stress_collections_pipeline.lsc|199990000"
  "stress_memory_reuse|tests/stress/stress_memory_reuse.lsc|0\\n1056000"
  "stress_task_spawn_reuse|tests/stress/stress_task_spawn_reuse.lsc|0\\n6400"
  "stress_http_burst_roundtrip|tests/stress/stress_http_burst_roundtrip.lsc|160"
  "stress_edge_guarded_ranges|tests/stress/stress_edge_guarded_ranges.lsc|22412"
  "gcd_walk|tests/stress/gcd_walk.lsc|15472700"
  "dense_nested|tests/stress/dense_nested.lsc|5480160000"
  "mix_sum|tests/stress/mix_sum.lsc|576005760000800006"
  "blocked_accum|tests/stress/blocked_accum.lsc|31999996000000"
  "pair_coupled|tests/stress/pair_coupled.lsc|2666726667300003"
  "alternating_sign|tests/stress/alternating_sign.lsc|-4000000"
  "graphics_raster_sum|tests/stress/graphics_raster_sum.lsc|549755781120"
  "graphics_interop_lines|tests/stress/graphics_interop_lines.lsc|133126"
  "game_capacity_alloc|tests/stress/game_capacity_alloc.lsc|32\\n-1"
  "game_render_pipeline|tests/stress/game_render_pipeline.lsc|124302463\\n240\\nfalse"
  "vectorization_probe|tests/stress/vectorization_probe.lsc|499999500000"
  "physics_capacity_walk|tests/stress/physics_capacity_walk.lsc|8386561"
)

failures=()
timings=()
passed=0
total=0

for t in "${tests[@]}"; do
  IFS='|' read -r name source expected_esc <<<"$t"
  expected=$(printf '%b' "$expected_esc")
  total=$((total + 1))
  exe="$artifact_dir/${name}${exe_suffix}"

  if ! build_out=$(run_lsc "$source" --build --max-speed --cc "$backend_compiler" -o "$exe"); then
    failures+=("[stress:$name] build failed:\n$build_out")
    continue
  fi

  start_ms="$(now_ms)"
  program_output=$("$exe" 2>&1 || true)
  end_ms="$(now_ms)"
  elapsed_ms=$((end_ms - start_ms))

  actual=$(normalize_text "$program_output")
  expected_norm=$(normalize_text "$expected")
  if [[ "$actual" != "$expected_norm" ]]; then
    failures+=("[stress:$name] output mismatch\nexpected:\n$expected_norm\nactual:\n$actual")
    continue
  fi

  passed=$((passed + 1))
  timings+=("[stress:$name] elapsed_ms=$elapsed_ms")
done

backend_lower="$(printf '%s' "$backend_compiler" | tr '[:upper:]' '[:lower:]')"
if [[ "$backend_lower" == *"clang"* ]]; then
  total=$((total + 1))
  vec_exe="$artifact_dir/vectorization_report_probe${exe_suffix}"
  if ! vec_build_out=$(run_lsc tests/stress/vectorization_probe.lsc --build --max-speed --cc "$backend_compiler" --keep-c -o "$vec_exe"); then
    failures+=("[vectorization:report] build failed:\n$vec_build_out")
  else
    vec_c="$vec_exe.c"
    if [[ ! -f "$vec_c" ]]; then
      failures+=("[vectorization:report] missing generated C file: $vec_c")
    else
      vec_obj="$vec_exe.vec.o"
      set +e
      vec_report=$("$backend_compiler" -O3 -fopenmp -Rpass=loop-vectorize -Rpass-missed=loop-vectorize -c "$vec_c" -o "$vec_obj" 2>&1)
      vec_rc=$?
      set -e
      if [[ $vec_rc -ne 0 ]]; then
        failures+=("[vectorization:report] clang report compile failed:\n$vec_report")
      else
        report_lower="$(printf '%s' "$vec_report" | tr '[:upper:]' '[:lower:]')"
        has_vector_remark=0
        [[ "$report_lower" == *"vectorized loop"* ]] && has_vector_remark=1
        has_simd_directive=0
        if grep -q "LS_OMP_SIMD_REDUCTION_PLUS(" "$vec_c" || grep -q "LS_OMP_SIMD" "$vec_c"; then
          has_simd_directive=1
        fi

        if [[ $has_vector_remark -eq 1 || $has_simd_directive -eq 1 ]]; then
          passed=$((passed + 1))
          if [[ $has_vector_remark -eq 1 ]]; then
            timings+=("[vectorization:report] vectorized loop remark detected")
          else
            timings+=("[vectorization:report] omp simd directive detected in probe loop")
          fi
        else
          failures+=("[vectorization:report] no vectorized loop remark or simd directive detected.\nreport:\n$vec_report")
        fi
      fi
    fi
  fi
else
  timings+=("[vectorization:report] skipped (backend compiler is not clang-compatible for -Rpass)")
fi

echo
echo "Stress summary: $passed / $total passed"
for line in "${timings[@]}"; do
  echo "$line"
done

if [[ ${#failures[@]} -gt 0 ]]; then
  echo
  echo "Stress failures:"
  for f in "${failures[@]}"; do
    echo "----------------------------------------"
    printf '%b\n' "$f"
  done
  exit 1
fi

if [[ $keep_artifacts -eq 0 ]]; then
  rm -rf "$artifact_dir"
fi

echo "All deterministic stress tests passed."
