#!/usr/bin/env bash
set -euo pipefail

frontend_compiler="clang++"
backend_compiler="clang"
max_speed=0
keep_artifacts=0
output_assurance_rounds=8

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
    --max-speed)
      max_speed=1
      shift
      ;;
    --keep-artifacts)
      keep_artifacts=1
      shift
      ;;
    --output-assurance-rounds)
      output_assurance_rounds="$2"
      shift 2
      ;;
    *)
      echo "Unknown option: $1" >&2
      echo "Usage: ./tests/run_tests.sh [--frontend-compiler clang++] [--backend-compiler clang] [--max-speed] [--keep-artifacts] [--output-assurance-rounds N]" >&2
      exit 1
      ;;
  esac
done

if ! [[ "$output_assurance_rounds" =~ ^[0-9]+$ ]] || [[ "$output_assurance_rounds" -lt 1 ]]; then
  echo "output_assurance_rounds must be >= 1" >&2
  exit 1
fi

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

contains_normalized() {
  local haystack="$1"
  local needle="$2"
  local h n
  h="$(normalize_text "$haystack" | tr '[:upper:]' '[:lower:]' | tr -s '[:space:]' ' ')"
  n="$(normalize_text "$needle" | tr '[:upper:]' '[:lower:]' | tr -s '[:space:]' ' ')"
  [[ "$h" == *"$n"* ]]
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

run_lsc_capture() {
  local out
  out=$("$compiler_bin" "$@" 2>&1)
  local rc=$?
  printf '%s' "$out"
  return $rc
}

run_wrapper_capture() {
  local wrapper="$root/linescript.sh"
  if [[ ! -f "$wrapper" ]]; then
    printf '%s' "missing wrapper: $wrapper"
    return 127
  fi
  local out
  out=$(bash "$wrapper" "$@" 2>&1)
  local rc=$?
  printf '%s' "$out"
  return $rc
}

build_frontend "$frontend_compiler"

artifact_dir="$root/tests/artifacts"
rm -rf "$artifact_dir"
mkdir -p "$artifact_dir"

# Representative deterministic suite for cross-platform stability.
runtime_tests=(
  "arithmetic_sum|tests/cases/runtime/arithmetic_sum.lsc|55||0"
  "declare_defaults|tests/cases/runtime/declare_defaults.lsc|0\\n0\\nfalse||0"
  "pow_ops|tests/cases/runtime/pow_ops.lsc|32\\n81\\n1||0"
  "string_utils|tests/cases/runtime/string_utils.lsc|10\\ntrue\\ntrue\\ntrue\\ntrue\\n4\\n-1||0"
  "print_strings|tests/cases/runtime/print_strings.lsc|x=5\\nx\\nhello||0"
  "top_level_print|tests/cases/runtime/top_level_print.lsc|milk||0"
  "top_level_if|tests/cases/runtime/top_level_if.lsc|ok||0"
  "top_level_function_call|tests/cases/runtime/top_level_function_call.lsc|milk||0"
  "top_level_most_features|tests/cases/runtime/top_level_most_features.lsc|ok:12\\n14\\n3\\n2\\nmilk\\nLineScript\\nhuman!||0"
  "superuser_privileged|tests/cases/runtime/superuser_privileged.lsc|[superuser] developer privileges enabled\\n[superuser] trace disabled\\ntrue\\ntrue\\ntrue\\n[superuser-hook] probe\\n0||0"
  "superuser_alias_prefix|tests/cases/runtime/superuser_alias_prefix.lsc|[superuser] developer privileges enabled\\n[superuser] trace disabled\\ntrue\\ntrue\\ntrue\\n[superuser-hook] alias\\n1||0"
  "superuser_relaxed_if_non_bool|tests/cases/runtime/superuser_relaxed_if_non_bool.lsc|[superuser] developer privileges enabled\\n0||0"
  "function_name_shadowing|tests/cases/runtime/function_name_shadowing.lsc|ok\\n42\\n4\\n100\\n9\\n7\\n5||0"
  "auto_entry_single_fn|tests/cases/runtime/auto_entry_single_fn.lsc|hi||0"
  "class_oop_cpp_style|tests/cases/runtime/class_oop_cpp_style.lsc|15\\nmain\\nfalse\\n12||0"
  "class_inheritance_modifiers|tests/cases/runtime/class_inheritance_modifiers.lsc|12\\n6\\n5||0"
  "class_public_private_access|tests/cases/runtime/class_public_private_access.lsc|42\\n7||0"
  "function_overload_resolution|tests/cases/runtime/function_overload_resolution.lsc|32\\n64\\n640||0"
  "operator_override_basic|tests/cases/runtime/operator_override_basic.lsc|105\\n108\\n12||0"
  "operator_override_member_precedence|tests/cases/runtime/operator_override_member_precedence.lsc|1010\\n105||0"
  "operator_unary_override_basic|tests/cases/runtime/operator_unary_override_basic.lsc|105\\ntrue||0"
  "operator_unary_method_basic|tests/cases/runtime/operator_unary_method_basic.lsc|1007||0"
  "macro_expand_expr|tests/cases/runtime/macro_expand_expr.lsc|42||0"
  "delete_array_handle|tests/cases/runtime/delete_array_handle.lsc|7||0"
  "i32_f32_types|tests/cases/runtime/i32_f32_types.lsc|12\\n1.75\\n5\\n2.5||0"
  "manual_memory_control|tests/cases/runtime/manual_memory_control.lsc|123\\n2.5\\n123\\n0||0"
  "for_step_runtime_zero|tests/cases/runtime/for_step_runtime_zero.lsc|0||0"
  "spawn_await|tests/cases/runtime/spawn_await.lsc|worker\\n1\\n1||0"
  "http_server_client_roundtrip|tests/cases/runtime/http_server_client_roundtrip.lsc|true\\ntrue||0"
  "game_headless_basic|tests/cases/runtime/game_headless_basic.lsc|1\\n16\\n16\\n10\\n65280\\n255\\n16777215\\n2446448900070348069\\n1\\nfalse||0"
  "bitmap_text_renderer|tests/cases/runtime/bitmap_text_renderer.lsc|4\\n3\\n660510\\ntrue\\n4\\n660510\\n660510\\n12\\n660510\\n12\\nsoftware\\ntrue\\ntrue\\nvulkan\\nfalse||0"
  "multicore_window_rendering|tests/cases/runtime/multicore_window_rendering.lsc|true\\ntrue\\nfalse\\n2\\ntrue\\nsoftware\\ntrue\\nopengl\\ntrue\\ntrue\\ntrue\\nfalse\\nwindowed\\ntrue\\nwindowed_fullscreen\\nfalse\\nfullscreen\\nwindowed\\ntrue\\n25\\n175\\nwindowed_fullscreen\\nfalse||0"
  "game_fullscreen_api|tests/cases/runtime/game_fullscreen_api.lsc|false\\nfalse\\nfalse\\nfalse\\nfalse||0"
  "game_scroll_mouse_inputs|tests/cases/runtime/game_scroll_mouse_inputs.lsc|0\\n0\\nfalse\\nfalse\\nfalse\\nfalse\\n0\\n0||0"
  "input_numeric|tests/cases/runtime/input_numeric.lsc|17\\n300|10\\n2.25\\n7\\n0.75\\n|0"
  "format_with_input|tests/cases/runtime/format_with_input.lsc|Neo|Neo\\n|0"
  "custom_flag_script_noarg|tests/cases/runtime/custom_flag_script.lsc|script-ok||0"
  "state_speed|tests/cases/runtime/state_speed.lsc|^49995000\\nspeed_us=[0-9]+$||1"
  "multi_module|tests/cases/runtime/module_math.lsc,tests/cases/runtime/module_main.lsc|55||0"
  "preset_library_demo|libraries/math/math_extras.lsc,libraries/text/text_extras.lsc,libraries/collections/collections_extras.lsc,libraries/game/game_helpers.lsc,examples/library_demo.lsc|49\\nhahaha\\n3\\ntrue\\n24||0"
  "game_engine_libraries_demo|libraries/window/window_helpers.lsc,libraries/ui/ui_tools.lsc,libraries/graphics2d/graphics2d_helpers.lsc,libraries/engine3d/engine3d_helpers.lsc,libraries/physics/physics_helpers.lsc,examples/game_engine_libraries_demo.lsc|96\\n54\\n1\\n2\\n1||0"
  "ls_extension|tests/cases/runtime/ls_extension.ls|123||0"
)

compile_fail_tests=(
  "const_no_init|tests/cases/compile_fail/const_no_init.lsc|declare const requires an initializer"
  "mod_zero_const|tests/cases/compile_fail/mod_zero_const.lsc|modulo by zero"
  "class_unknown_field|tests/cases/compile_fail/class_unknown_field.lsc|has no field"
  "class_bad_override|tests/cases/compile_fail/class_bad_override.lsc|has no base method to override"
  "class_override_final|tests/cases/compile_fail/class_override_final.lsc|cannot override final base method"
  "class_static_instance_call|tests/cases/compile_fail/class_static_instance_call.lsc|must be called via class name"
  "class_private_field_access|tests/cases/compile_fail/class_private_field_access.lsc|field 'secret' is not accessible in this context"
  "bad_call_arity|tests/cases/compile_fail/bad_call_arity.lsc|function 'sqrt' expects 1 args"
  "game_fullscreen_bad_types|tests/cases/compile_fail/game_fullscreen_bad_types.lsc|arg 1 cannot convert 'str' to 'i64'"
  "http_bad_arity|tests/cases/compile_fail/http_bad_arity.lsc|function 'http_server_listen' expects 1 args"
  "game_scroll_bad_types|tests/cases/compile_fail/game_scroll_bad_types.lsc|arg 1 cannot convert 'str' to 'i64'"
  "game_mouse_down_bad_types|tests/cases/compile_fail/game_mouse_down_bad_types.lsc|arg 2 cannot convert 'str' to 'i64'"
  "multicore_window_bad_types|tests/cases/compile_fail/multicore_window_bad_types.lsc|arg 1 cannot convert 'str' to 'i64'"
  "parallel_for_break|tests/cases/compile_fail/parallel_for_break.lsc|parallel for does not support break/continue"
  "not_non_bool|tests/cases/compile_fail/not_non_bool.lsc|unary '!' requires bool"
  "operator_override_bad_arity|tests/cases/compile_fail/operator_override_bad_arity.lsc|expects exactly 2 parameters"
  "operator_method_bad_arity|tests/cases/compile_fail/operator_method_bad_arity.lsc|expects exactly 1 parameter"
  "operator_unary_override_bad_arity|tests/cases/compile_fail/operator_unary_override_bad_arity.lsc|operator override 'unary -' expects exactly 1 parameter"
  "operator_unary_method_bad_arity|tests/cases/compile_fail/operator_unary_method_bad_arity.lsc|operator method 'unary -' expects exactly 0 parameters"
  "macro_ret_stmt_not_supported|tests/cases/compile_fail/macro_ret_stmt_not_supported.lsc|macro return kinds stmt/item are not implemented yet"
  "delete_unknown_var|tests/cases/compile_fail/delete_unknown_var.lsc|unknown variable 'ghost'"
  "ptr_inner_type_not_supported|tests/cases/compile_fail/ptr_inner_type_not_supported.lsc|unsupported ptr inner type 'weird_t'"
  "superuser_not_privileged|tests/cases/compile_fail/superuser_not_privileged.lsc|Not privileged"
)

build_fail_tests=(
  "ambiguous_entry|tests/cases/compile_fail/ambiguous_entry.lsc --build --cc $backend_compiler -o $artifact_dir/ambiguous_entry${exe_suffix}|ambiguous entry point"
)

run_cli_tests=(
  "format_clean_output|tests/cases/runtime/format_clean_cli.lsc|milk|"
  "custom_flag_defined|tests/cases/runtime/custom_flag_script.lsc|flag-hello\\nscript-ok|--hello"
  "custom_flag_hyphen_defined|tests/cases/runtime/custom_flag_hyphen.lsc|flag-hyphen\\nscript-ok|--hello-world"
  "cli_fancy_parser|tests/cases/runtime/cli_fancy_parser.lsc|16\\ntrue\\ntrue\\ntrue\\ntrue\\nmax\\nprog.scl\\nturbo|-O [ -p max -X [ --beta-features ] ] -W [ --not-strict ] -i prog.scl --profile=turbo"
)

output_assurance_tests=(
  "print_hi|tests/cases/runtime/output_assurance_print_hi.lsc|hi"
  "format_output_fn|tests/cases/runtime/output_assurance_format_output.lsc|A=1\\nB=2"
  "format_output_block|tests/cases/runtime/output_assurance_format_block.lsc|line=42\\ndone!"
  "top_level_if|tests/cases/runtime/top_level_if.lsc|ok"
  "runtime_format_block|tests/cases/runtime/format_block.lsc|A1\\nB!"
  "format_clean_marker|tests/cases/runtime/format_clean_cli.lsc|milk"
)

cli_info_tests=(
  "superuser_terminal_warning|tests/cases/runtime/superuser_privileged.lsc --check|Warning: superuser() enabled"
  "flag_super_speed|--super-speed|super speed activated, hold your horses.."
  "flag_what|--what|what? are you asking me?"
  "flag_hlep|--hlep|i think you made a little spelling mistake in your flag there"
  "flag_max_sped|--max-sped|hurga durga doo! max sped activated!"
  "flag_linescript_version|--LineScript|LineScript version 1.5.1 (Velocity update)"
  "flag_undefined_warning|tests/cases/runtime/custom_flag_script.lsc --run --cc $backend_compiler -o $artifact_dir/flag_undefined_warning${exe_suffix} --ghost|Warning: undefined flag '--ghost'"
  "flag_bad_warning|tests/cases/runtime/custom_flag_script.lsc --run --cc $backend_compiler -o $artifact_dir/flag_bad_warning${exe_suffix} ---bad|Warning: bad flag '---bad' ignored"
)

cli_hardening_tests=(
  "unsafe_cc_injection|tests/cases/runtime/arithmetic_sum.lsc --check --cc clang;echo hacked|unsafe character in --cc value"
  "bad_extension|tests/cases/runtime/arithmetic_sum.txt --check|input file must use .lsc or .ls extension"
)

repl_tests=(
  "repl_superuser_help_not_privileged|su.help\\n:exit\\n|Not privileged: enable superuser() first"
  "repl_superuser_help_privileged|superuser()\\nsu.help\\n:exit\\n|superuser REPL commands:"
  "repl_superuser_help_alias_privileged|superuser()\\nsuperuser.help\\n:exit\\n|superuser REPL commands:"
  "repl_superuser_verbosity_alias|superuser()\\nsuperuser.verbosity.5\\n:exit\\n|verbosity set to 5"
  "repl_print_after_superuser_toggle|superuser()\\nsuperuser()\\nprint(\"shell-print-still-works\")\\n:exit\\n|shell-print-still-works"
  "repl_builtin_not_confused_by_var_name|declare print = 9\\nprint(\"call-ok\")\\n:exit\\n|call-ok"
)

failures=()
passed=0
total=0

for t in "${runtime_tests[@]}"; do
  IFS='|' read -r name sources_csv expected_esc input_esc is_regex <<<"$t"
  expected=$(printf '%b' "$expected_esc")
  input_data=$(printf '%b' "$input_esc")

  total=$((total + 1))
  exe="$artifact_dir/${name}${exe_suffix}"

  IFS=',' read -r -a sources <<<"$sources_csv"
  build_args=("${sources[@]}" --build --no-cache --cc "$backend_compiler" -o "$exe")
  if [[ $max_speed -eq 1 ]]; then
    build_args+=(--max-speed)
  fi

  if ! build_out=$(run_lsc_capture "${build_args[@]}"); then
    failures+=("[runtime:$name] build failed:\n$build_out")
    continue
  fi

  if [[ -n "$input_data" ]]; then
    program_output=$(printf '%s' "$input_data" | "$exe" 2>&1 || true)
  else
    program_output=$("$exe" 2>&1 || true)
  fi
  actual=$(normalize_text "$program_output")

  if [[ "$is_regex" == "1" ]]; then
    if ! printf '%s' "$actual" | grep -Eq "$expected"; then
      failures+=("[runtime:$name] output mismatch\nexpected regex:\n$expected\nactual:\n$actual")
      continue
    fi
  else
    expected_norm=$(normalize_text "$expected")
    if [[ "$actual" != "$expected_norm" ]]; then
      failures+=("[runtime:$name] output mismatch\nexpected:\n$expected_norm\nactual:\n$actual")
      continue
    fi
  fi

  passed=$((passed + 1))
done

for t in "${compile_fail_tests[@]}"; do
  IFS='|' read -r name source contains <<<"$t"
  total=$((total + 1))
  set +e
  out=$(run_lsc_capture "$source" --check)
  rc=$?
  set -e
  if [[ $rc -eq 0 ]]; then
    failures+=("[compile-fail:$name] expected non-zero exit code")
    continue
  fi
  if ! contains_normalized "$out" "$contains"; then
    failures+=("[compile-fail:$name] expected message containing: $contains\nactual:\n$out")
    continue
  fi
  passed=$((passed + 1))
done

for t in "${build_fail_tests[@]}"; do
  IFS='|' read -r name args_str contains <<<"$t"
  total=$((total + 1))
  # shellcheck disable=SC2206
  args=( $args_str )
  set +e
  out=$(run_lsc_capture "${args[@]}")
  rc=$?
  set -e
  if [[ $rc -eq 0 ]]; then
    failures+=("[build-fail:$name] expected non-zero exit code")
    continue
  fi
  if ! contains_normalized "$out" "$contains"; then
    failures+=("[build-fail:$name] expected message containing: $contains\nactual:\n$out")
    continue
  fi
  passed=$((passed + 1))
done

for t in "${run_cli_tests[@]}"; do
  IFS='|' read -r name source expected_esc extra_args_str <<<"$t"
  expected=$(printf '%b' "$expected_esc")
  total=$((total + 1))
  exe="$artifact_dir/${name}${exe_suffix}"

  args=("$source" --run --no-cache --cc "$backend_compiler" -o "$exe")
  if [[ -n "$extra_args_str" ]]; then
    # shellcheck disable=SC2206
    set -f
    extra_args=( $extra_args_str )
    set +f
    args+=("${extra_args[@]}")
  fi
  if [[ $max_speed -eq 1 ]]; then
    args+=(--max-speed)
  fi

  set +e
  out=$(run_lsc_capture "${args[@]}")
  rc=$?
  set -e
  if [[ $rc -ne 0 ]]; then
    failures+=("[cli-run:$name] command failed:\n$out")
    continue
  fi

  actual=$(normalize_text "$out")
  expected_norm=$(normalize_text "$expected")
  if [[ "$actual" != "$expected_norm" ]]; then
    failures+=("[cli-run:$name] output mismatch\nexpected:\n$expected_norm\nactual:\n$actual")
    continue
  fi
  passed=$((passed + 1))
done

for t in "${output_assurance_tests[@]}"; do
  IFS='|' read -r name source expected_esc <<<"$t"
  expected=$(printf '%b' "$expected_esc")
  for ((round = 1; round <= output_assurance_rounds; round++)); do
    for mode in direct wrapper; do
      total=$((total + 1))
      exe="$artifact_dir/output_assurance_${name}_${mode}_${round}${exe_suffix}"

      if [[ "$mode" == "direct" ]]; then
        set +e
        out=$(run_lsc_capture "$source" --run --no-cache --max-speed --cc "$backend_compiler" -o "$exe")
        rc=$?
        set -e
      else
        set +e
        out=$(run_wrapper_capture "$source" --no-cache --max-speed --cc "$backend_compiler" -o "$exe")
        rc=$?
        set -e
      fi

      if [[ $rc -ne 0 ]]; then
        failures+=("[output-assurance:$name:$mode:round-$round] command failed:\n$out")
        continue
      fi
      if ! contains_normalized "$out" "$expected"; then
        failures+=("[output-assurance:$name:$mode:round-$round] missing expected output\nexpected to contain:\n$expected\nactual:\n$out")
        continue
      fi
      passed=$((passed + 1))
    done
  done
done

for t in "${cli_info_tests[@]}"; do
  IFS='|' read -r name args_str contains <<<"$t"
  total=$((total + 1))
  # shellcheck disable=SC2206
  args=( $args_str )
  set +e
  out=$(run_lsc_capture "${args[@]}")
  rc=$?
  set -e
  if [[ $rc -ne 0 ]]; then
    failures+=("[cli-info:$name] expected zero exit code\nactual:\n$out")
    continue
  fi
  if ! contains_normalized "$out" "$contains"; then
    failures+=("[cli-info:$name] expected message containing: $contains\nactual:\n$out")
    continue
  fi
  passed=$((passed + 1))
done

for t in "${cli_hardening_tests[@]}"; do
  IFS='|' read -r name args_str contains <<<"$t"
  total=$((total + 1))
  if [[ "$name" == "unsafe_cc_injection" ]]; then
    args=("tests/cases/runtime/arithmetic_sum.lsc" "--check" "--cc" "clang;echo hacked")
  else
    # shellcheck disable=SC2206
    args=( $args_str )
  fi
  set +e
  out=$(run_lsc_capture "${args[@]}")
  rc=$?
  set -e
  if [[ $rc -eq 0 ]]; then
    failures+=("[cli:$name] expected non-zero exit code")
    continue
  fi
  if ! contains_normalized "$out" "$contains"; then
    failures+=("[cli:$name] expected message containing: $contains\nactual:\n$out")
    continue
  fi
  passed=$((passed + 1))
done

for t in "${repl_tests[@]}"; do
  IFS='|' read -r name input_esc contains <<<"$t"
  input_data=$(printf '%b' "$input_esc")
  total=$((total + 1))
  set +e
  out=$(printf '%s' "$input_data" | "$compiler_bin" --shell 2>&1)
  rc=$?
  set -e
  if [[ $rc -ne 0 ]]; then
    failures+=("[repl:$name] expected zero exit code\nactual:\n$out")
    continue
  fi
  if ! contains_normalized "$out" "$contains"; then
    failures+=("[repl:$name] expected message containing: $contains\nactual:\n$out")
    continue
  fi
  passed=$((passed + 1))
done

echo
echo "Test summary: $passed / $total passed"
if [[ ${#failures[@]} -gt 0 ]]; then
  echo
  echo "Failures:"
  for f in "${failures[@]}"; do
    echo "----------------------------------------"
    printf '%b\n' "$f"
  done
  exit 1
fi

if [[ $keep_artifacts -eq 0 ]]; then
  rm -rf "$artifact_dir"
fi

echo "All deterministic tests passed."
