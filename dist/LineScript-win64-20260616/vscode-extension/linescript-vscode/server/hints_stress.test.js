"use strict";

const assert = require("assert");
const { performance } = require("perf_hooks");
const { collectHeuristicDiagnostics } = require("./hints");

function collect(text, settings) {
  return collectHeuristicDiagnostics(
    {
      uri: "file:///stress.lsc",
      getText: () => text
    },
    settings || {}
  );
}

function stableShape(diags) {
  return JSON.stringify(
    (diags || []).map((d) => ({
      code: d.code,
      sev: d.severity,
      msg: d.message,
      s: d.range && d.range.start ? [d.range.start.line, d.range.start.character] : null,
      e: d.range && d.range.end ? [d.range.end.line, d.range.end.character] : null
    }))
  );
}

function hasCode(diags, code) {
  return (diags || []).some((d) => d.code === code);
}

// Regression: this sequence must never be interpreted as a duplicate function declaration.
{
  const src = [
    "superuser()",
    "su.capabilites()"
  ].join("\n");
  for (let i = 0; i < 300; i += 1) {
    const diags = collect(src);
    assert.ok(!hasCode(diags, "ls-duplicate-declare"), "superuser() must never trigger duplicate declaration here");
    assert.ok(hasCode(diags, "ls-unknown-su-command"), "typoed su command should be diagnosed");
  }
}

// Edge-case snippets: malformed calls, malformed declarations, deep nesting, mixed statement styles.
{
  const snippets = [
    "superuser()",
    "superuser.capabilities()",
    "su.trace.on()",
    "su.limit.set(100, 200)",
    "su.capabilites(",
    "declare = 4",
    "if x = y do",
    "end",
    "for i in 0..10 step 0 do",
    "  println(i)",
    "end",
    "print(\"unterminated)",
    "print(((((1+2))))",
    "main() -> i64 do\n  declare x = 1\n  x += 2\n  return 0\nend"
  ];
  for (const src of snippets) {
    const diags = collect(src, { styleHintsEnabled: true });
    assert.ok(Array.isArray(diags), "diagnostics must be an array");
  }
}

// Stress corpus: large file + deterministic diagnostics across repeated runs.
{
  const lines = [];
  lines.push("main() -> i64 do");
  for (let i = 0; i < 6000; i += 1) {
    if (i % 17 === 0) {
      lines.push("  superuser()");
      lines.push("  su.capabilites()");
      continue;
    }
    if (i % 13 === 0) {
      lines.push(`  declare v${i}: i64 = ${i}`);
      lines.push(`  v${i} = v${i} + 1`);
      continue;
    }
    if (i % 11 === 0) {
      lines.push("  if v0 = v1 do");
      lines.push("    println(\"x\")");
      lines.push("  end");
      continue;
    }
    if (i % 7 === 0) {
      lines.push("  this is not valid");
      continue;
    }
    lines.push(`  println(${i})`);
  }
  lines.push("  return 0");
  lines.push("end");
  const src = lines.join("\n");

  const t0 = performance.now();
  const first = collect(src, { styleHintsEnabled: true, maxHintsPerFile: 1000 });
  const firstShape = stableShape(first);
  for (let i = 0; i < 20; i += 1) {
    const next = collect(src, { styleHintsEnabled: true, maxHintsPerFile: 1000 });
    assert.strictEqual(stableShape(next), firstShape, "diagnostics should be deterministic across runs");
    assert.ok(!hasCode(next, "ls-duplicate-declare"), "stress corpus must not produce superuser duplicate declaration");
  }
  const elapsedMs = performance.now() - t0;
  assert.ok(elapsedMs < 15000, `diagnostics stress test took too long: ${elapsedMs.toFixed(1)} ms`);
}

console.log("hints_stress.test.js: stress + edge diagnostics tests passed");
