"use strict";

const assert = require("assert");
const { collectHeuristicDiagnostics } = require("./hints");

function collect(text, settings) {
  return collectHeuristicDiagnostics(
    {
      getText: () => text
    },
    settings || {}
  );
}

function hasCode(diags, code) {
  return diags.some((d) => d.code === code);
}

{
  const src = [
    "if x = y do",
    "  printIn(\"bad\")",
    "end"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-assign-in-condition"), "assignment-in-condition hint missing");
  assert.ok(hasCode(diags, "ls-printin-typo"), "printIn typo hint missing");
}

{
  const src = [
    "else if x == 1 do",
    "end",
    "for i in 0...10 do",
    "end"
  ].join("\n");
  const diags = collect(src, { styleHintsEnabled: true });
  assert.ok(hasCode(diags, "ls-else-if-style"), "else-if style hint missing");
  assert.ok(hasCode(diags, "ls-triple-dot-range"), "triple-dot range warning missing");
}

{
  const src = [
    "if x == true do",
    "  input(\"name?\")",
    "end",
    "count = count + 1;",
    "if x:",
    "  println(\"python\")",
    "end"
  ].join("\n");
  const diags = collect(src, { styleHintsEnabled: true });
  assert.ok(hasCode(diags, "ls-compare-true"), "compare-true hint missing");
  assert.ok(hasCode(diags, "ls-input-ignored"), "ignored input hint missing");
  assert.ok(hasCode(diags, "ls-compound-assignment"), "compound assignment hint missing");
  assert.ok(hasCode(diags, "ls-unneeded-semicolon"), "semicolon hint missing");
  assert.ok(hasCode(diags, "ls-python-colon-block"), "python-colon warning missing");
}

{
  const src = [
    "if x == 1",
    "  println(x)",
    "end"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-missing-do"), "missing-do warning missing");
}

{
  const src = "count = count + 1;";
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-compound-assignment"), "style hints should be disabled by default");
  assert.ok(!hasCode(diags, "ls-unneeded-semicolon"), "semicolon style hint should be disabled by default");
}

{
  const src = [
    "input(\"x\")",
    "input_i64()"
  ].join("\n");
  const diags = collect(src, { hintsEnabled: false });
  assert.ok(!hasCode(diags, "ls-input-ignored"), "hints should be disabled");
}

{
  const src = [
    "declare x = 1",
    "y = x + 1"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-undeclared-assign"), "undeclared assignment warning missing");
}

{
  const src = [
    "declare const n = 1",
    "n = 2"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-const-reassign"), "const reassignment error missing");
}

{
  const src = [
    "declare x = 1",
    "declare x = 2"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-duplicate-declare"), "duplicate declaration warning missing");
}

{
  const src = [
    "break",
    "continue"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-break-outside-loop"), "break-outside-loop error missing");
  assert.ok(hasCode(diags, "ls-continue-outside-loop"), "continue-outside-loop error missing");
}

{
  const src = [
    "main() -> i64 do",
    "  declare x = 1",
    "  return 0",
    "  x = x + 1",
    "end"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-unreachable-statement"), "unreachable statement warning missing");
}

{
  const src = [
    "print() {",
    "}",
    "print()",
    "mystery_call(1, 2)"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-reserved-builtin-name"), "reserved builtin function name warning missing");
  assert.ok(hasCode(diags, "ls-builtin-signature-mismatch"), "builtin signature mismatch warning missing");
  assert.ok(hasCode(diags, "ls-function-arity"), "function arity warning missing");
  assert.ok(hasCode(diags, "ls-unknown-function"), "unknown function warning missing");
}

{
  const src = [
    "fn pair(a b) -> i64 do",
    "  return a + b",
    "end",
    "pair(1)"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-malformed-params"), "malformed parameter warning missing");
  assert.ok(hasCode(diags, "ls-function-arity"), "user function arity warning missing");
}

{
  const src = [
    "for i in 0..10 step 0 do",
    "  println(i)",
    "end",
    "declare = 42",
    "value = 10 / 0",
    "this is not valid"
  ].join("\n");
  const diags = collect(src);
  assert.ok(hasCode(diags, "ls-zero-step"), "zero-step loop error missing");
  assert.ok(hasCode(diags, "ls-malformed-declare"), "malformed declare error missing");
  assert.ok(hasCode(diags, "ls-divide-by-zero-literal"), "divide-by-zero literal error missing");
  assert.ok(hasCode(diags, "ls-unknown-statement"), "unknown statement warning missing");
}

{
  const src = [
    "main() -> i64 do",
    "  declare arr = array_new()",
    "  array_set(arr, 1, \"LineScript\")",
    "  declare profile = dict_new()",
    "  dict_set(profile, \"name\", array_get(arr, 0))",
    "  println(dict_get(profile, \"name\"))",
    "  return 0",
    "end"
  ].join("\n");
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-malformed-call-args"), "valid nested call args should not be flagged malformed");
  assert.ok(!hasCode(diags, "ls-function-arity"), "valid nested call arity should not be flagged");
}

{
  const src = [
    "worker() -> i64 throws IOError do",
    "  return 7",
    "end",
    "main() -> i64 throws IOError do",
    "  println(worker())",
    "  return 0",
    "end"
  ].join("\n");
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-malformed-function-signature"), "throws contract function should parse cleanly");
  assert.ok(!hasCode(diags, "ls-unknown-function"), "declared function in throws contract should not be unknown");
}

{
  const src = [
    ".format()",
    ".freeConsole()"
  ].join("\n");
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-unknown-function"), "dot marker calls should not be treated as unknown plain functions");
}

{
  const src = [
    "superuser()",
    "su.capabilites()"
  ].join("\n");
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-duplicate-declare"), "superuser call should not be treated as a duplicate declaration");
  assert.ok(hasCode(diags, "ls-unknown-su-command"), "misspelled superuser command should produce a dedicated warning");
}

{
  const src = [
    "superuser()",
    "su.capabilities()",
    "superuser.capabilities()"
  ].join("\n");
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-unknown-su-command"), "valid superuser commands should not be flagged unknown");
  assert.ok(!hasCode(diags, "ls-duplicate-declare"), "superuser calls should never look like duplicate declarations");
}

{
  const src = [
    "class Counter do",
    "  constructor(start: i64, label: str) do",
    "    this.value = start",
    "    this.name = label",
    "  end",
    "end"
  ].join("\n");
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-undeclared-var"), "constructor params should not be undeclared");
  assert.ok(!hasCode(diags, "ls-unknown-statement"), "member assignments should not be unknown statements");
}

{
  const src = [
    "helper(a: i64) -> i64 do",
    "  declare total: i64 = a + 1",
    "  total = total + 2",
    "  println(total)",
    "  return total",
    "end"
  ].join("\n");
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-undeclared-var"), "function-local declared variables should stay in scope");
  assert.ok(!hasCode(diags, "ls-undeclared-assign"), "function-local declared assignment should not be flagged");
}

{
  const src = [
    "class Base do",
    "  public virtual fn value() -> i64 do",
    "    return 1",
    "  end",
    "end",
    "class Derived extends Base do",
    "  public override final fn value() -> i64 do",
    "    return 2",
    "  end",
    "end"
  ].join("\n");
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-undeclared-var"), "OOP modifiers must not be treated as variables");
  assert.ok(!hasCode(diags, "ls-unknown-statement"), "OOP modifier method declarations must be recognized statements");
  assert.ok(!hasCode(diags, "ls-unknown-function"), "overridden method declarations must not be treated as unknown calls");
}

{
  const src = [
    "flag hello-world() do",
    "  println(\"ok\")",
    "end"
  ].join("\n");
  const diags = collect(src);
  assert.ok(!hasCode(diags, "ls-malformed-flag-name"), "hyphenated flag names should be allowed");
  assert.ok(!hasCode(diags, "ls-unknown-function"), "hyphenated flag definition should not trigger unknown call");
}

console.log("hints.test.js: all tests passed");
