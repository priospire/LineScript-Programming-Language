param(
  [string]$FrontendCompiler = "clang++",
  [string]$BackendCompiler = "clang",
  [switch]$MaxSpeed,
  [switch]$KeepArtifacts,
  [int]$OutputAssuranceRounds = 8
)

$ErrorActionPreference = "Stop"
if ($null -ne (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue)) {
  $PSNativeCommandUseErrorActionPreference = $false
}
if ($OutputAssuranceRounds -lt 1) {
  throw "OutputAssuranceRounds must be >= 1."
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

function Contains-Normalized {
  param(
    [string]$Haystack,
    [string]$Needle
  )
  $h = ((Normalize-Text $Haystack) -replace "\s+", " ").ToLowerInvariant()
  $n = ((Normalize-Text $Needle) -replace "\s+", " ").ToLowerInvariant()
  return $h.Contains($n)
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
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
      & $cxx -std=c++20 -O3 -Wall -Wextra -pedantic $compilerSrc -o $compilerExe 2>&1 | Out-Null
      if ($LASTEXITCODE -eq 0) {
        Write-Host "Frontend build OK with $cxx"
        return
      }
    } catch {
      # try next
    } finally {
      $ErrorActionPreference = $prev
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

function Run-LineScriptWrapper {
  param([string[]]$CliArgs)
  $wrapper = Join-Path $root "linescript.cmd"
  if (-not (Test-Path $wrapper)) {
    return [PSCustomObject]@{
      Output = "missing wrapper: $wrapper"
      ExitCode = 127
    }
  }
  $prev = $ErrorActionPreference
  $ErrorActionPreference = "Continue"
  try {
    $out = & $wrapper @CliArgs 2>&1 | Out-String
  } finally {
    $ErrorActionPreference = $prev
  }
  return [PSCustomObject]@{
    Output = $out
    ExitCode = $LASTEXITCODE
  }
}

Build-Frontend -Preferred $FrontendCompiler

$artifactDir = Join-Path $root "tests\\artifacts"
if (Test-Path $artifactDir) {
  Remove-Item -Recurse -Force $artifactDir
}
New-Item -ItemType Directory -Path $artifactDir | Out-Null

$runtimeTests = @(
  [PSCustomObject]@{ Name = "arithmetic_sum"; Sources = @("tests\\cases\\runtime\\arithmetic_sum.lsc"); Expected = "55" },
  [PSCustomObject]@{ Name = "declare_defaults"; Sources = @("tests\\cases\\runtime\\declare_defaults.lsc"); Expected = "0`n0`nfalse" },
  [PSCustomObject]@{ Name = "unless_flow"; Sources = @("tests\\cases\\runtime\\unless_flow.lsc"); Expected = "1`n2`n3" },
  [PSCustomObject]@{ Name = "pow_ops"; Sources = @("tests\\cases\\runtime\\pow_ops.lsc"); Expected = "32`n81`n1" },
  [PSCustomObject]@{ Name = "modulo_compare"; Sources = @("tests\\cases\\runtime\\modulo_compare.lsc"); Expected = "5`ntrue`nfalse" },
  [PSCustomObject]@{ Name = "compound_assign"; Sources = @("tests\\cases\\runtime\\compound_assign.lsc"); Expected = "8`n2.3333333333333335" },
  [PSCustomObject]@{ Name = "math_builtin_checks"; Sources = @("tests\\cases\\runtime\\math_builtin_checks.lsc"); Expected = "1`n1`n10`n1" },
  [PSCustomObject]@{ Name = "print_generic"; Sources = @("tests\\cases\\runtime\\print_generic.lsc"); Expected = "4210042`n1.5`ntrue" },
  [PSCustomObject]@{ Name = "print_strings"; Sources = @("tests\\cases\\runtime\\print_strings.lsc"); Expected = "x=5`nx`nhello" },
  [PSCustomObject]@{ Name = "top_level_print"; Sources = @("tests\\cases\\runtime\\top_level_print.lsc"); Expected = "milk" },
  [PSCustomObject]@{ Name = "top_level_if"; Sources = @("tests\\cases\\runtime\\top_level_if.lsc"); Expected = "ok" },
  [PSCustomObject]@{ Name = "top_level_function_call"; Sources = @("tests\\cases\\runtime\\top_level_function_call.lsc"); Expected = "milk" },
  [PSCustomObject]@{ Name = "top_level_most_features"; Sources = @("tests\\cases\\runtime\\top_level_most_features.lsc"); Expected = "ok:12`n14`n3`n2`nmilk`nLineScript`nhuman!" },
  [PSCustomObject]@{ Name = "superuser_privileged"; Sources = @("tests\\cases\\runtime\\superuser_privileged.lsc"); Expected = "[superuser] developer privileges enabled`n[superuser] trace disabled`ntrue`ntrue`ntrue`n[superuser-hook] probe`n0" },
  [PSCustomObject]@{ Name = "superuser_alias_prefix"; Sources = @("tests\\cases\\runtime\\superuser_alias_prefix.lsc"); Expected = "[superuser] developer privileges enabled`n[superuser] trace disabled`ntrue`ntrue`ntrue`n[superuser-hook] alias`n1" },
  [PSCustomObject]@{ Name = "superuser_relaxed_if_non_bool"; Sources = @("tests\\cases\\runtime\\superuser_relaxed_if_non_bool.lsc"); Expected = "[superuser] developer privileges enabled`n0" },
  [PSCustomObject]@{ Name = "function_name_shadowing"; Sources = @("tests\\cases\\runtime\\function_name_shadowing.lsc"); Expected = "ok`n42`n4`n100`n9`n7`n5" },
  [PSCustomObject]@{ Name = "auto_entry_single_fn"; Sources = @("tests\\cases\\runtime\\auto_entry_single_fn.lsc"); Expected = "hi" },
  [PSCustomObject]@{ Name = "string_equality"; Sources = @("tests\\cases\\runtime\\string_equality.lsc"); Expected = "true`ntrue" },
  [PSCustomObject]@{ Name = "string_utils"; Sources = @("tests\\cases\\runtime\\string_utils.lsc"); Expected = "10`ntrue`ntrue`ntrue`ntrue`n4`n-1" },
  [PSCustomObject]@{ Name = "increment_ops"; Sources = @("tests\\cases\\runtime\\increment_ops.lsc"); Expected = "2" },
  [PSCustomObject]@{ Name = "string_byte_utils"; Sources = @("tests\\cases\\runtime\\string_byte_utils.lsc"); Expected = "AbC`nhello`nHI`nline_script`ncde`nhahaha`ncba`ntrue`n90`n90`nA`n3`n1234`n2.5" },
  [PSCustomObject]@{ Name = "array_join_pop_map"; Sources = @("tests\\cases\\runtime\\array_join_pop_map.lsc"); Expected = "a,b,c`ntrue`nc`n2`n10`nfalse`n1" },
  [PSCustomObject]@{ Name = "graphics_basic"; Sources = @("tests\\cases\\runtime\\graphics_basic.lsc"); Expected = "65280`n65280`n255`n65280`n8`n8" },
  [PSCustomObject]@{ Name = "graphics_edge_cases"; Sources = @("tests\\cases\\runtime\\graphics_edge_cases.lsc"); Expected = "-1`n65292`n-1`n-1`n329223`ntrue`nfalse" },
  [PSCustomObject]@{ Name = "pygame_aliases"; Sources = @("tests\\cases\\runtime\\pygame_aliases.lsc"); Expected = "1`n65280`n3941241610301994966`n1`nfalse" },
  [PSCustomObject]@{ Name = "numpy_basic"; Sources = @("tests\\cases\\runtime\\numpy_basic.lsc"); Expected = "5`n10`n40`n5`n300`n2`n4`n5`n6" },
  [PSCustomObject]@{ Name = "paradigms_basics"; Sources = @("tests\\cases\\runtime\\paradigms_basics.lsc"); Expected = "45`nLineScript`ntrue`n2`n7`n1.5`n9`n2.5`n10`n1" },
  [PSCustomObject]@{ Name = "class_oop_cpp_style"; Sources = @("tests\\cases\\runtime\\class_oop_cpp_style.lsc"); Expected = "15`nmain`nfalse`n12" },
  [PSCustomObject]@{ Name = "class_inheritance_modifiers"; Sources = @("tests\\cases\\runtime\\class_inheritance_modifiers.lsc"); Expected = "12`n6`n5" },
  [PSCustomObject]@{ Name = "class_public_private_access"; Sources = @("tests\\cases\\runtime\\class_public_private_access.lsc"); Expected = "42`n7" },
  [PSCustomObject]@{ Name = "function_overload_resolution"; Sources = @("tests\\cases\\runtime\\function_overload_resolution.lsc"); Expected = "32`n64`n640" },
  [PSCustomObject]@{ Name = "operator_override_basic"; Sources = @("tests\\cases\\runtime\\operator_override_basic.lsc"); Expected = "105`n108`n12" },
  [PSCustomObject]@{ Name = "operator_override_member_precedence"; Sources = @("tests\\cases\\runtime\\operator_override_member_precedence.lsc"); Expected = "1010`n105" },
  [PSCustomObject]@{ Name = "operator_unary_override_basic"; Sources = @("tests\\cases\\runtime\\operator_unary_override_basic.lsc"); Expected = "105`ntrue" },
  [PSCustomObject]@{ Name = "operator_unary_method_basic"; Sources = @("tests\\cases\\runtime\\operator_unary_method_basic.lsc"); Expected = "1007" },
  [PSCustomObject]@{ Name = "macro_expand_expr"; Sources = @("tests\\cases\\runtime\\macro_expand_expr.lsc"); Expected = "42" },
  [PSCustomObject]@{ Name = "delete_array_handle"; Sources = @("tests\\cases\\runtime\\delete_array_handle.lsc"); Expected = "7" },
  [PSCustomObject]@{ Name = "i32_f32_types"; Sources = @("tests\\cases\\runtime\\i32_f32_types.lsc"); Expected = "12`n1.75`n5`n2.5" },
  [PSCustomObject]@{ Name = "game_headless_basic"; Sources = @("tests\\cases\\runtime\\game_headless_basic.lsc"); Expected = "1`n16`n16`n10`n65280`n255`n16777215`n2446448900070348069`n1`nfalse" },
  [PSCustomObject]@{ Name = "game_fullscreen_api"; Sources = @("tests\\cases\\runtime\\game_fullscreen_api.lsc"); Expected = "false`nfalse`nfalse`nfalse`nfalse" },
  [PSCustomObject]@{ Name = "game_edge_handles"; Sources = @("tests\\cases\\runtime\\game_edge_handles.lsc"); Expected = "0`n0`ntrue`n-1`n0`nfalse`n-1`n1`n66051" },
  [PSCustomObject]@{ Name = "game_mouse_normalization"; Sources = @("tests\\cases\\runtime\\game_mouse_normalization.lsc"); Expected = "0`n0`n0`n0" },
  [PSCustomObject]@{ Name = "game_scroll_mouse_inputs"; Sources = @("tests\\cases\\runtime\\game_scroll_mouse_inputs.lsc"); Expected = "0`n0`nfalse`nfalse`nfalse`nfalse`n0`n0" },
  [PSCustomObject]@{ Name = "physics_camera_input"; Sources = @("tests\\cases\\runtime\\physics_camera_input.lsc"); Expected = "0`ntrue`n1`n2`n4`n6`n1045`n0`n10`nfalse`nfalse" },
  [PSCustomObject]@{ Name = "if_not_neq"; Sources = @("tests\\cases\\runtime\\if_not_neq.lsc"); Expected = "1`n1" },
  [PSCustomObject]@{ Name = "key_poll_link"; Sources = @("tests\\cases\\runtime\\key_poll_link.lsc"); Expected = "1" },
  [PSCustomObject]@{ Name = "input_array_dict"; Sources = @("tests\\cases\\runtime\\input_array_dict.lsc"); Input = "Ava`n"; Expected = "name: Ava`nguest`nAva`nguest`ntrue`n2" },
  [PSCustomObject]@{ Name = "input_index_store"; Sources = @("tests\\cases\\runtime\\input_index_store.lsc"); Input = "kira`n"; Expected = "kira" },
  [PSCustomObject]@{ Name = "input_stability"; Sources = @("tests\\cases\\runtime\\input_stability.lsc"); Input = "alpha`nbeta`n"; Expected = "alpha`nbeta" },
  [PSCustomObject]@{ Name = "input_numeric"; Sources = @("tests\\cases\\runtime\\input_numeric.lsc"); Input = "10`n2.25`n7`n0.75`n"; Expected = "17`n300" },
  [PSCustomObject]@{ Name = "format_with_input"; Sources = @("tests\\cases\\runtime\\format_with_input.lsc"); Input = "Neo`n"; Expected = "Neo" },
  [PSCustomObject]@{ Name = "http_server_client_roundtrip"; Sources = @("tests\\cases\\runtime\\http_server_client_roundtrip.lsc"); Expected = "true`ntrue" },
  [PSCustomObject]@{ Name = "state_speed"; Sources = @("tests\\cases\\runtime\\state_speed.lsc"); ExpectedRegex = "^49995000`nspeed_us=[0-9]+$" },
  [PSCustomObject]@{ Name = "free_console_call"; Sources = @("tests\\cases\\runtime\\free_console_call.lsc"); Expected = "" },
  [PSCustomObject]@{ Name = "replace_and_string_stability"; Sources = @("tests\\cases\\runtime\\replace_and_string_stability.lsc"); Expected = "aa`nbaxx`ncdef" },
  [PSCustomObject]@{ Name = "common_numeric_utils"; Sources = @("tests\\cases\\runtime\\common_numeric_utils.lsc"); Expected = "3`n7`n6`n36" },
  [PSCustomObject]@{ Name = "manual_memory_control"; Sources = @("tests\\cases\\runtime\\manual_memory_control.lsc"); Expected = "123`n2.5`n123`n0" },
  [PSCustomObject]@{ Name = "owned_handles_raii"; Sources = @("tests\\cases\\runtime\\owned_handles_raii.lsc"); Expected = "1`n0" },
  [PSCustomObject]@{ Name = "result_option_basics"; Sources = @("tests\\cases\\runtime\\result_option_basics.lsc"); Expected = "true`ntrue`nhello`nfallback`ntrue`ntrue`nok`nIOError`nmissing`ndefault" },
  [PSCustomObject]@{ Name = "throws_contract_ok"; Sources = @("tests\\cases\\runtime\\throws_contract_ok.lsc"); Expected = "7" },
  [PSCustomObject]@{ Name = "format_output"; Sources = @("tests\\cases\\runtime\\format_output.lsc"); Expected = "42`n3.5`ntrue`nLineScript" },
  [PSCustomObject]@{ Name = "format_block"; Sources = @("tests\\cases\\runtime\\format_block.lsc"); Expected = "A1`nB!" },
  [PSCustomObject]@{ Name = "custom_flag_script_noarg"; Sources = @("tests\\cases\\runtime\\custom_flag_script.lsc"); Expected = "script-ok" },
  [PSCustomObject]@{ Name = "spawn_await"; Sources = @("tests\\cases\\runtime\\spawn_await.lsc"); Expected = "worker`n1`n1" },
  [PSCustomObject]@{ Name = "parallel_for_basic"; Sources = @("tests\\cases\\runtime\\parallel_for_basic.lsc"); Expected = "1" },
  [PSCustomObject]@{ Name = "small_loop_unroll"; Sources = @("tests\\cases\\runtime\\small_loop_unroll.lsc"); Expected = "56" },
  [PSCustomObject]@{ Name = "for_step_runtime_zero"; Sources = @("tests\\cases\\runtime\\for_step_runtime_zero.lsc"); Expected = "0" },
  [PSCustomObject]@{ Name = "multi_module"; Sources = @("tests\\cases\\runtime\\module_math.lsc", "tests\\cases\\runtime\\module_main.lsc"); Expected = "55" },
  [PSCustomObject]@{ Name = "preset_library_demo"; Sources = @("libraries\\math\\math_extras.lsc", "libraries\\text\\text_extras.lsc", "libraries\\collections\\collections_extras.lsc", "libraries\\game\\game_helpers.lsc", "examples\\library_demo.lsc"); Expected = "49`nhahaha`n3`ntrue`n24" },
  [PSCustomObject]@{ Name = "game_engine_libraries_demo"; Sources = @("libraries\\window\\window_helpers.lsc", "libraries\\ui\\ui_tools.lsc", "libraries\\graphics2d\\graphics2d_helpers.lsc", "libraries\\engine3d\\engine3d_helpers.lsc", "libraries\\physics\\physics_helpers.lsc", "examples\\game_engine_libraries_demo.lsc"); Expected = "96`n54`n1`n2`n1" },
  [PSCustomObject]@{ Name = "ls_extension"; Sources = @("tests\\cases\\runtime\\ls_extension.ls"); Expected = "123" },
  [PSCustomObject]@{ Name = "braces_style"; Sources = @("tests\\cases\\runtime\\braces_style.lsc"); Expected = "1" },
  [PSCustomObject]@{ Name = "for_negative_step"; Sources = @("tests\\cases\\runtime\\for_negative_step.lsc"); Expected = "30" }
)

$compileFailTests = @(
  [PSCustomObject]@{ Name = "const_no_init"; Source = "tests\\cases\\compile_fail\\const_no_init.lsc"; Contains = "declare const requires an initializer" },
  [PSCustomObject]@{ Name = "undeclared_assign"; Source = "tests\\cases\\compile_fail\\undeclared_assign.lsc"; Contains = "assignment to undeclared variable 'x'" },
  [PSCustomObject]@{ Name = "old_let_keyword"; Source = "tests\\cases\\compile_fail\\old_let_keyword.lsc"; Contains = "use 'declare <name>' for variable declarations" },
  [PSCustomObject]@{ Name = "mod_float"; Source = "tests\\cases\\compile_fail\\mod_float.lsc"; Contains = "'%' requires integer operands" },
  [PSCustomObject]@{ Name = "compound_assign_type_error"; Source = "tests\\cases\\compile_fail\\compound_assign_type_error.lsc"; Contains = "'%' requires integer operands" },
  [PSCustomObject]@{ Name = "if_not_bool"; Source = "tests\\cases\\compile_fail\\if_not_bool.lsc"; Contains = "if condition must be bool" },
  [PSCustomObject]@{ Name = "div_zero_const"; Source = "tests\\cases\\compile_fail\\div_zero_const.lsc"; Contains = "division by zero" },
  [PSCustomObject]@{ Name = "mod_zero_const"; Source = "tests\\cases\\compile_fail\\mod_zero_const.lsc"; Contains = "modulo by zero" },
  [PSCustomObject]@{ Name = "for_step_zero_literal"; Source = "tests\\cases\\compile_fail\\for_step_zero_literal.lsc"; Contains = "for range step cannot be zero" },
  [PSCustomObject]@{ Name = "class_unknown_field"; Source = "tests\\cases\\compile_fail\\class_unknown_field.lsc"; Contains = "has no field" },
  [PSCustomObject]@{ Name = "class_bad_override"; Source = "tests\\cases\\compile_fail\\class_bad_override.lsc"; Contains = "has no base method to override" },
  [PSCustomObject]@{ Name = "class_override_final"; Source = "tests\\cases\\compile_fail\\class_override_final.lsc"; Contains = "cannot override final base method" },
  [PSCustomObject]@{ Name = "class_static_instance_call"; Source = "tests\\cases\\compile_fail\\class_static_instance_call.lsc"; Contains = "must be called via class name" },
  [PSCustomObject]@{ Name = "class_private_field_access"; Source = "tests\\cases\\compile_fail\\class_private_field_access.lsc"; Contains = "field 'secret' is not accessible in this context" },
  [PSCustomObject]@{ Name = "bad_call_arity"; Source = "tests\\cases\\compile_fail\\bad_call_arity.lsc"; Contains = "function 'sqrt' expects 1 args" },
  [PSCustomObject]@{ Name = "http_bad_arity"; Source = "tests\\cases\\compile_fail\\http_bad_arity.lsc"; Contains = "function 'http_server_listen' expects 1 args" },
  [PSCustomObject]@{ Name = "string_arithmetic"; Source = "tests\\cases\\compile_fail\\string_arithmetic.lsc"; Contains = "arithmetic requires numeric" },
  [PSCustomObject]@{ Name = "len_wrong_type"; Source = "tests\\cases\\compile_fail\\len_wrong_type.lsc"; Contains = "arg 1 cannot convert 'i64' to 'str'" },
  [PSCustomObject]@{ Name = "parallel_for_break"; Source = "tests\\cases\\compile_fail\\parallel_for_break.lsc"; Contains = "parallel for does not support break/continue" },
  [PSCustomObject]@{ Name = "parallel_for_outer_assign"; Source = "tests\\cases\\compile_fail\\parallel_for_outer_assign.lsc"; Contains = "parallel for cannot assign to outer variables" },
  [PSCustomObject]@{ Name = "format_block_bad_end_type"; Source = "tests\\cases\\compile_fail\\format_block_bad_end_type.lsc"; Contains = "formatOutput block end argument must be str" },
  [PSCustomObject]@{ Name = "spawn_with_args"; Source = "tests\\cases\\compile_fail\\spawn_with_args.lsc"; Contains = "spawn target must not take arguments" },
  [PSCustomObject]@{ Name = "spawn_non_void"; Source = "tests\\cases\\compile_fail\\spawn_non_void.lsc"; Contains = "spawn target must return void" },
  [PSCustomObject]@{ Name = "state_speed_bad_arity"; Source = "tests\\cases\\compile_fail\\state_speed_bad_arity.lsc"; Contains = "function '.stateSpeed' expects 0 args" },
  [PSCustomObject]@{ Name = "free_console_bad_arity"; Source = "tests\\cases\\compile_fail\\free_console_bad_arity.lsc"; Contains = "function '.freeConsole' expects 0 args" },
  [PSCustomObject]@{ Name = "format_bad_arity"; Source = "tests\\cases\\compile_fail\\format_bad_arity.lsc"; Contains = "function '.format' expects 0 args" },
  [PSCustomObject]@{ Name = "owned_in_loop"; Source = "tests\\cases\\compile_fail\\owned_in_loop.lsc"; Contains = "declare owned is not allowed inside loops" },
  [PSCustomObject]@{ Name = "owned_assign"; Source = "tests\\cases\\compile_fail\\owned_assign.lsc"; Contains = "cannot assign to owned handle" },
  [PSCustomObject]@{ Name = "owned_bad_ctor"; Source = "tests\\cases\\compile_fail\\owned_bad_ctor.lsc"; Contains = "declare owned requires constructor call" },
  [PSCustomObject]@{ Name = "owned_return"; Source = "tests\\cases\\compile_fail\\owned_return.lsc"; Contains = "cannot return owned handle variable" },
  [PSCustomObject]@{ Name = "throws_contract_missing"; Source = "tests\\cases\\compile_fail\\throws_contract_missing.lsc"; Contains = "may throw 'IOError'" },
  [PSCustomObject]@{ Name = "input_bad_arity"; Source = "tests\\cases\\compile_fail\\input_bad_arity.lsc"; Contains = "function 'input' expects 0 or 1 args" },
  [PSCustomObject]@{ Name = "input_i64_bad_arity"; Source = "tests\\cases\\compile_fail\\input_i64_bad_arity.lsc"; Contains = "function 'input_i64' expects 0 or 1 args" },
  [PSCustomObject]@{ Name = "np_bad_types"; Source = "tests\\cases\\compile_fail\\np_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "array_bad_types"; Source = "tests\\cases\\compile_fail\\array_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "game_bad_types"; Source = "tests\\cases\\compile_fail\\game_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "game_fullscreen_bad_types"; Source = "tests\\cases\\compile_fail\\game_fullscreen_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "game_draw_gfx_bad_types"; Source = "tests\\cases\\compile_fail\\game_draw_gfx_bad_types.lsc"; Contains = "arg 2 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "game_mouse_bad_types"; Source = "tests\\cases\\compile_fail\\game_mouse_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "game_scroll_bad_types"; Source = "tests\\cases\\compile_fail\\game_scroll_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "game_mouse_down_bad_types"; Source = "tests\\cases\\compile_fail\\game_mouse_down_bad_types.lsc"; Contains = "arg 2 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "generic_numeric_bad_types"; Source = "tests\\cases\\compile_fail\\generic_numeric_bad_types.lsc"; Contains = "function 'max' requires numeric args" },
  [PSCustomObject]@{ Name = "operator_override_bad_arity"; Source = "tests\\cases\\compile_fail\\operator_override_bad_arity.lsc"; Contains = "expects exactly 2 parameters" },
  [PSCustomObject]@{ Name = "operator_method_bad_arity"; Source = "tests\\cases\\compile_fail\\operator_method_bad_arity.lsc"; Contains = "expects exactly 1 parameter" },
  [PSCustomObject]@{ Name = "operator_unary_override_bad_arity"; Source = "tests\\cases\\compile_fail\\operator_unary_override_bad_arity.lsc"; Contains = "operator override 'unary -' expects exactly 1 parameter" },
  [PSCustomObject]@{ Name = "operator_unary_method_bad_arity"; Source = "tests\\cases\\compile_fail\\operator_unary_method_bad_arity.lsc"; Contains = "operator method 'unary -' expects exactly 0 parameters" },
  [PSCustomObject]@{ Name = "macro_ret_stmt_not_supported"; Source = "tests\\cases\\compile_fail\\macro_ret_stmt_not_supported.lsc"; Contains = "macro return kinds stmt/item are not implemented yet" },
  [PSCustomObject]@{ Name = "delete_unknown_var"; Source = "tests\\cases\\compile_fail\\delete_unknown_var.lsc"; Contains = "unknown variable 'ghost'" },
  [PSCustomObject]@{ Name = "ptr_inner_type_not_supported"; Source = "tests\\cases\\compile_fail\\ptr_inner_type_not_supported.lsc"; Contains = "unsupported ptr inner type 'weird_t'" },
  [PSCustomObject]@{ Name = "gfx_bad_types"; Source = "tests\\cases\\compile_fail\\gfx_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "phys_bad_types"; Source = "tests\\cases\\compile_fail\\phys_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'f64'" },
  [PSCustomObject]@{ Name = "camera_bad_types"; Source = "tests\\cases\\compile_fail\\camera_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "key_bad_types"; Source = "tests\\cases\\compile_fail\\key_bad_types.lsc"; Contains = "arg 1 cannot convert 'str' to 'i64'" },
  [PSCustomObject]@{ Name = "not_non_bool"; Source = "tests\\cases\\compile_fail\\not_non_bool.lsc"; Contains = "unary '!' requires bool" },
  [PSCustomObject]@{ Name = "superuser_not_privileged"; Source = "tests\\cases\\compile_fail\\superuser_not_privileged.lsc"; Contains = "Not privileged" }
)

$buildFailTests = @(
  [PSCustomObject]@{
    Name = "ambiguous_entry"
    Args = @("tests\\cases\\compile_fail\\ambiguous_entry.lsc", "--build", "--cc", $BackendCompiler, "-o", (Join-Path $artifactDir "ambiguous_entry.exe"))
    Contains = "ambiguous entry point"
  }
)

$runCliTests = @(
  [PSCustomObject]@{
    Name = "format_clean_output"
    Source = "tests\\cases\\runtime\\format_clean_cli.lsc"
    Expected = "milk"
  },
  [PSCustomObject]@{
    Name = "custom_flag_defined"
    Source = "tests\\cases\\runtime\\custom_flag_script.lsc"
    Expected = "flag-hello`nscript-ok"
    ExtraArgs = @("--hello")
  },
  [PSCustomObject]@{
    Name = "custom_flag_hyphen_defined"
    Source = "tests\\cases\\runtime\\custom_flag_hyphen.lsc"
    Expected = "flag-hyphen`nscript-ok"
    ExtraArgs = @("--hello-world")
  },
  [PSCustomObject]@{
    Name = "cli_fancy_parser"
    Source = "tests\\cases\\runtime\\cli_fancy_parser.lsc"
    Expected = "16`ntrue`ntrue`ntrue`ntrue`nmax`nprog.scl`nturbo"
    ExtraArgs = @("-O", "[", "-p", "max", "-X", "[", "--beta-features", "]", "]", "-W", "[", "--not-strict", "]", "-i", "prog.scl", "--profile=turbo")
  }
)

$outputAssuranceTests = @(
  [PSCustomObject]@{
    Name = "print_hi"
    Source = "tests\\cases\\runtime\\output_assurance_print_hi.lsc"
    Expected = "hi"
  },
  [PSCustomObject]@{
    Name = "format_output_fn"
    Source = "tests\\cases\\runtime\\output_assurance_format_output.lsc"
    Expected = "A=1`nB=2"
  },
  [PSCustomObject]@{
    Name = "format_output_block"
    Source = "tests\\cases\\runtime\\output_assurance_format_block.lsc"
    Expected = "line=42`ndone!"
  },
  [PSCustomObject]@{
    Name = "top_level_if"
    Source = "tests\\cases\\runtime\\top_level_if.lsc"
    Expected = "ok"
  },
  [PSCustomObject]@{
    Name = "runtime_format_block"
    Source = "tests\\cases\\runtime\\format_block.lsc"
    Expected = "A1`nB!"
  },
  [PSCustomObject]@{
    Name = "format_clean_marker"
    Source = "tests\\cases\\runtime\\format_clean_cli.lsc"
    Expected = "milk"
  }
)

$cliInfoTests = @(
  [PSCustomObject]@{
    Name = "superuser_terminal_warning"
    Args = @("tests\\cases\\runtime\\superuser_privileged.lsc", "--check")
    Contains = "Warning: superuser() enabled"
  },
  [PSCustomObject]@{
    Name = "flag_super_speed"
    Args = @("--super-speed")
    Contains = "super speed activated, hold your horses.."
  },
  [PSCustomObject]@{
    Name = "flag_what"
    Args = @("--what")
    Contains = "what? are you asking me?"
  },
  [PSCustomObject]@{
    Name = "flag_hlep"
    Args = @("--hlep")
    Contains = "i think you made a little spelling mistake in your flag there"
  },
  [PSCustomObject]@{
    Name = "flag_max_sped"
    Args = @("--max-sped")
    Contains = "hurga durga doo! max sped activated!"
  },
  [PSCustomObject]@{
    Name = "flag_linescript_version"
    Args = @("--LineScript")
    Contains = "LineScript version 1.5.0 (Velocity update)"
  },
  [PSCustomObject]@{
    Name = "flag_undefined_warning"
    Args = @("tests\\cases\\runtime\\custom_flag_script.lsc", "--run", "--cc", $BackendCompiler, "-o",
      (Join-Path $artifactDir "flag_undefined_warning.exe"), "--ghost")
    Contains = "Warning: undefined flag '--ghost'"
  },
  [PSCustomObject]@{
    Name = "flag_bad_warning"
    Args = @("tests\\cases\\runtime\\custom_flag_script.lsc", "--run", "--cc", $BackendCompiler, "-o",
      (Join-Path $artifactDir "flag_bad_warning.exe"), "---bad")
    Contains = "Warning: bad flag '---bad' ignored"
  }
)

$cliHardeningTests = @(
  [PSCustomObject]@{
    Name = "unsafe_cc_injection"
    Args = @("tests\\cases\\runtime\\arithmetic_sum.lsc", "--check", "--cc", "clang;echo hacked")
    Contains = "unsafe character in --cc value"
  },
  [PSCustomObject]@{
    Name = "bad_extension"
    Args = @("tests\\cases\\runtime\\arithmetic_sum.txt", "--check")
    Contains = "input file must use .lsc or .ls extension"
  }
)

$replTests = @(
  [PSCustomObject]@{
    Name = "repl_superuser_help_not_privileged"
    Input = "su.help`n:exit`n"
    Contains = "Not privileged: enable superuser() first"
  },
  [PSCustomObject]@{
    Name = "repl_superuser_help_privileged"
    Input = "superuser()`nsu.help`n:exit`n"
    Contains = "superuser REPL commands:"
  },
  [PSCustomObject]@{
    Name = "repl_superuser_help_alias_privileged"
    Input = "superuser()`nsuperuser.help`n:exit`n"
    Contains = "superuser REPL commands:"
  },
  [PSCustomObject]@{
    Name = "repl_superuser_verbosity_alias"
    Input = "superuser()`nsuperuser.verbosity.5`n:exit`n"
    Contains = "verbosity set to 5"
  },
  [PSCustomObject]@{
    Name = "repl_print_after_superuser_toggle"
    Input = "superuser()`nsuperuser()`nprint(""shell-print-still-works"")`n:exit`n"
    Contains = "shell-print-still-works"
  },
  [PSCustomObject]@{
    Name = "repl_builtin_not_confused_by_var_name"
    Input = "declare print = 9`nprint(""call-ok"")`n:exit`n"
    Contains = "call-ok"
  }
)

$failures = New-Object System.Collections.Generic.List[string]
$passed = 0
$total = 0

foreach ($t in $runtimeTests) {
  $total += 1
  $exe = Join-Path $artifactDir ($t.Name + ".exe")
  $buildArgs = @()
  $buildArgs += $t.Sources
  $buildArgs += @("--build", "--no-cache", "--cc", $BackendCompiler, "-o", $exe)
  if ($MaxSpeed) { $buildArgs += "--max-speed" }

  $build = Run-Lsc -CliArgs $buildArgs
  if ($build.ExitCode -ne 0) {
    $failures.Add("[runtime:$($t.Name)] build failed:`n$($build.Output)")
    continue
  }

  $programOutput = ""
  if ($t.PSObject.Properties.Name -contains "Input") {
    $programOutput = $t.Input | & $exe 2>&1 | Out-String
  } else {
    $programOutput = & $exe 2>&1 | Out-String
  }
  $actual = Normalize-Text $programOutput
  if (($t.PSObject.Properties.Name -contains "ExpectedRegex") -and -not [string]::IsNullOrWhiteSpace($t.ExpectedRegex)) {
    if (-not [regex]::IsMatch($actual, $t.ExpectedRegex)) {
      $failures.Add("[runtime:$($t.Name)] output mismatch`nexpected regex:`n$($t.ExpectedRegex)`nactual:`n$actual")
      continue
    }
  } else {
    $expected = Normalize-Text $t.Expected
    if ($actual -ne $expected) {
      $failures.Add("[runtime:$($t.Name)] output mismatch`nexpected:`n$expected`nactual:`n$actual")
      continue
    }
  }
  $passed += 1
}

foreach ($t in $compileFailTests) {
  $total += 1
  $res = Run-Lsc -CliArgs @($t.Source, "--check")
  if ($res.ExitCode -eq 0) {
    $failures.Add("[compile-fail:$($t.Name)] expected non-zero exit code")
    continue
  }
  if (-not (Contains-Normalized -Haystack $res.Output -Needle $t.Contains)) {
    $failures.Add("[compile-fail:$($t.Name)] expected message containing: $($t.Contains)`nactual:`n$($res.Output)")
    continue
  }
  $passed += 1
}

foreach ($t in $buildFailTests) {
  $total += 1
  $res = Run-Lsc -CliArgs $t.Args
  if ($res.ExitCode -eq 0) {
    $failures.Add("[build-fail:$($t.Name)] expected non-zero exit code")
    continue
  }
  if (-not (Contains-Normalized -Haystack $res.Output -Needle $t.Contains)) {
    $failures.Add("[build-fail:$($t.Name)] expected message containing: $($t.Contains)`nactual:`n$($res.Output)")
    continue
  }
  $passed += 1
}

foreach ($t in $runCliTests) {
  $total += 1
  $exe = Join-Path $artifactDir ($t.Name + ".exe")
  $args = @($t.Source, "--run", "--no-cache", "--cc", $BackendCompiler, "-o", $exe)
  if ($t.PSObject.Properties.Name -contains "ExtraArgs") { $args += $t.ExtraArgs }
  if ($MaxSpeed) { $args += "--max-speed" }
  $res = Run-Lsc -CliArgs $args
  if ($res.ExitCode -ne 0) {
    $failures.Add("[cli-run:$($t.Name)] command failed:`n$($res.Output)")
    continue
  }
  $actual = Normalize-Text $res.Output
  $expected = Normalize-Text $t.Expected
  if ($actual -ne $expected) {
    $failures.Add("[cli-run:$($t.Name)] output mismatch`nexpected:`n$expected`nactual:`n$actual")
    continue
  }
  $passed += 1
}

foreach ($t in $outputAssuranceTests) {
  for ($round = 1; $round -le $OutputAssuranceRounds; $round++) {
    foreach ($mode in @("direct", "wrapper")) {
      $total += 1
      $exe = Join-Path $artifactDir ("output_assurance_" + $t.Name + "_" + $mode + "_" + $round + ".exe")
      if ($mode -eq "direct") {
        $args = @($t.Source, "--run", "--no-cache", "--max-speed", "--cc", $BackendCompiler, "-o", $exe)
        $res = Run-Lsc -CliArgs $args
      } else {
        $args = @($t.Source, "--no-cache", "--max-speed", "--cc", $BackendCompiler, "-o", $exe)
        $res = Run-LineScriptWrapper -CliArgs $args
      }
      if ($res.ExitCode -ne 0) {
        $failures.Add("[output-assurance:$($t.Name):$mode:round-$round] command failed:`n$($res.Output)")
        continue
      }
      if (-not (Contains-Normalized -Haystack $res.Output -Needle $t.Expected)) {
        $failures.Add("[output-assurance:$($t.Name):$mode:round-$round] missing expected output`nexpected to contain:`n$($t.Expected)`nactual:`n$($res.Output)")
        continue
      }
      $passed += 1
    }
  }
}

foreach ($t in $cliInfoTests) {
  $total += 1
  $res = Run-Lsc -CliArgs $t.Args
  if ($res.ExitCode -ne 0) {
    $failures.Add("[cli-info:$($t.Name)] expected zero exit code`nactual:`n$($res.Output)")
    continue
  }
  if (-not (Contains-Normalized -Haystack $res.Output -Needle $t.Contains)) {
    $failures.Add("[cli-info:$($t.Name)] expected message containing: $($t.Contains)`nactual:`n$($res.Output)")
    continue
  }
  $passed += 1
}

foreach ($t in $cliHardeningTests) {
  $total += 1
  $res = Run-Lsc -CliArgs $t.Args
  if ($res.ExitCode -eq 0) {
    $failures.Add("[cli:$($t.Name)] expected non-zero exit code")
    continue
  }
  if (-not (Contains-Normalized -Haystack $res.Output -Needle $t.Contains)) {
    $failures.Add("[cli:$($t.Name)] expected message containing: $($t.Contains)`nactual:`n$($res.Output)")
    continue
  }
  $passed += 1
}

foreach ($t in $replTests) {
  $total += 1
  $prev = $ErrorActionPreference
  $ErrorActionPreference = "Continue"
  try {
    $out = $t.Input | & .\lsc.exe --shell 2>&1 | Out-String
    $rc = $LASTEXITCODE
  } finally {
    $ErrorActionPreference = $prev
  }
  if ($rc -ne 0) {
    $failures.Add("[repl:$($t.Name)] expected zero exit code`nactual:`n$out")
    continue
  }
  if (-not (Contains-Normalized -Haystack $out -Needle $t.Contains)) {
    $failures.Add("[repl:$($t.Name)] expected message containing: $($t.Contains)`nactual:`n$out")
    continue
  }
  $passed += 1
}

Write-Host ""
Write-Host "Test summary: $passed / $total passed"
if ($failures.Count -gt 0) {
  Write-Host ""
  Write-Host "Failures:"
  foreach ($f in $failures) {
    Write-Host "----------------------------------------"
    Write-Host $f
  }
  exit 1
}

if (-not $KeepArtifacts) {
  Remove-Item -Recurse -Force $artifactDir
}

Write-Host "All deterministic tests passed."
exit 0
