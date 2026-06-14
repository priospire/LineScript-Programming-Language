"use strict";

const assert = require("assert");
const fs = require("fs");
const path = require("path");
const { collectHeuristicDiagnostics } = require("./hints");

const repoRoot = path.resolve(__dirname, "..", "..", "..");

const shouldBeClean = [
  "examples/benchmark.lsc",
  "examples/control_flow.lsc",
  "examples/collections_input.lsc",
  "examples/fib.ls",
  "examples/fib.lsc",
  "examples/format_block.lsc",
  "examples/format_marker.lsc",
  "examples/format_mode_input.lsc",
  "examples/if_elif.lsc",
  "examples/math_showcase.lsc",
  "examples/module_math.lsc",
  "examples/module_main.lsc",
  "tests/cases/runtime/array_join_pop_map.lsc",
  "tests/cases/runtime/class_inheritance_modifiers.lsc",
  "tests/cases/runtime/class_oop_cpp_style.lsc",
  "tests/cases/runtime/function_overload_resolution.lsc",
  "tests/cases/runtime/custom_flag_hyphen.lsc",
  "tests/cases/runtime/format_block.lsc",
  "tests/cases/runtime/format_clean_cli.lsc",
  "tests/cases/runtime/format_with_input.lsc",
  "tests/cases/runtime/free_console_call.lsc",
  "tests/cases/runtime/i32_f32_types.lsc",
  "tests/cases/runtime/math_builtin_checks.lsc",
  "tests/cases/runtime/module_main.lsc",
  "tests/cases/runtime/module_math.lsc",
  "tests/cases/runtime/string_byte_utils.lsc",
  "tests/cases/runtime/string_utils.lsc",
  "tests/cases/runtime/top_level_most_features.lsc",
  "tests/cases/runtime/throws_contract_ok.lsc"
];

for (const rel of shouldBeClean) {
  const file = path.join(repoRoot, rel);
  assert.ok(fs.existsSync(file), `Missing test fixture: ${rel}`);
  const src = fs.readFileSync(file, "utf8");
  const uri = "file:///" + file.replace(/\\/g, "/");
  const diags = collectHeuristicDiagnostics(
    {
      uri,
      getText: () => src
    },
    {}
  );
  if (diags.length > 0) {
    const summary = diags
      .map((d) => `${d.range.start.line + 1}:${d.range.start.character + 1} [${d.code}] ${d.message}`)
      .join("\n");
    assert.fail(`Expected zero diagnostics for ${rel}, found ${diags.length}\n${summary}`);
  }
}

console.log("false_positives.test.js: all clean fixtures passed");
