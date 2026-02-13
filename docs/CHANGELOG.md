# LineScript Changelog

## 2026-02-13 (LineScript v1.4.5)

### Added
- `superuser()` developer mode.
- privileged `su.*` namespace:
- `su.trace.on()` / `su.trace.off()`
- `su.capabilities()`
- `su.memory.inspect()`
- `su.limit.set(step_limit, mem_limit)`
- `su.compiler.inspect()`
- `su.ir.dump()`
- `su.debug.hook(tag)`
- terminal warning when `superuser()` is active.
- compatibility CLI aliases for quick diagnostics.

### Changed
- top-level parser now supports dotted `su.*` calls without requiring class receivers.
- type-checking now enforces privilege boundary:
- `su.*` without `superuser()` returns `Not privileged`.
- in superuser mode, selected safety checks are relaxed with explicit warnings (not silent).
- compile/build/run pipeline now prints verbose superuser diagnostics.
- with `.format()`, superuser diagnostics route to debug/error stream to keep normal program output clean.
- `.format()` no longer switches to GUI-subsystem linking; it now keeps stdout visible and only suppresses toolchain chatter.
- runtime includes superuser step/memory inspection and debug tracing hooks.
- built-in alias flags (`--super-speed`, `--what`, `--hlep`, `--max-sped`, `--LineScript`) no longer stop script execution.
- added `flag name() do ... end` custom CLI flag functions (`flag hello-world()` maps to `--hello-world`).
- undefined or malformed `--flags` now emit warnings and are ignored without stopping program execution.

### Fixed
- parser ambiguity where `formatOutput(\"...\") do ... end` could be misread as a function declaration.
- output ordering reliability for compiler status lines before execution.
- Windows frontend build macro conflict (`min`/`max`) after adding debug-window support.

### Tests
- added runtime test: `tests/cases/runtime/superuser_privileged.lsc`.
- added compile-fail test: `tests/cases/compile_fail/superuser_not_privileged.lsc`.
- added/updated top-level script tests (`top_level_function_call`, `top_level_most_features`, format CLI checks).
- deterministic suite: `103 / 103` passed.

### Distribution
- rebuilt `dist/LineScript-win64-20260213` and `dist/LineScript-win64-20260213.zip`.
- removed the public demo-only extras file from distribution.
