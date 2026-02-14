# LineScript Changelog

## 2026-02-14 (Documentation Sync)

### Changed
- refreshed documentation cross-links so OOP syntax is discoverable from root docs:
- added `docs/OOP.md` link in `README.md` documentation map.
- expanded OOP syntax examples in `README.md` and `docs/LANGUAGE_GUIDE.md` with:
- `extends`
- `public` / `protected` / `private`
- `static` / `virtual` / `override` / `final`
- constructor init-list base calls (`constructor(...) : Base(...)`).
- updated `docs/SYNTAX.md` and `docs/API_REFERENCE.md` with current class/OOP language forms.
- updated `docs/IDE_SETUP.md` to document current VSCode diagnostics defaults:
- `linescript.compilerDiagnosticsOnly`
- `linescript.hintsEnabled`
- `linescript.styleHintsEnabled`
- local compiler pinning via `linescript.lscPath`.

## 2026-02-14 (Roadmap Phase Update)

### Added
- advanced class model support:
- single inheritance with `extends`
- access modifiers (`public`, `protected`, `private`)
- method modifiers (`static`, `virtual`, `override`, `final`)
- constructor base init-list calls (`constructor(...) : Base(...) do ... end`)
- method overload declarations (duplicate-signature protection)
- top-level overload resolution with exact-match then safe-widening selection.
- runtime/compile-fail regression cases for inheritance/modifier behavior:
- `tests/cases/runtime/class_inheritance_modifiers.lsc`
- `tests/cases/runtime/function_overload_resolution.lsc`
- `tests/cases/compile_fail/class_bad_override.lsc`
- `tests/cases/compile_fail/class_override_final.lsc`
- `tests/cases/compile_fail/class_static_instance_call.lsc`
- max-speed pipeline flags:
- `--pgo-generate`
- `--pgo-use <profile-dir>`
- `--bolt-use <fdata>`

### Changed
- class member/field lookup now walks base classes.
- access and override/final validation now enforced in parser/type flow.
- `tests/run_zig_comparison.ps1` defaults now follow stricter gauntlet settings:
- `WarmupRuns=3`
- `MeasuredRuns=9`
- `ConfidenceMarginPercent=2.0`
- per-test domination gate with blocking/non-blocking edge handling.
- `tests/run_zig_comparison.sh` now mirrors domination-margin controls.
- benchmark docs updated with domination methodology and confidence gate details.

### Tests
- deterministic suite (`tests/run_tests.sh`) pass: `164/164`.
- targeted runtime checks for inheritance/modifier and overload behavior passed.
- targeted compile-fail checks for override/final/static-call constraints passed.

## 2026-02-14 (LineScript v1.4.6)

### Added
- interactive LineScript shell (REPL) inside `lsc`:
- launch with no args (`lsc.exe`) or explicit `--repl` / `--shell`.
- shell meta-commands: `:help`, `:reset`, `:whoami`, `:exit` / `:quit`.
- superuser REPL verbosity command: `su.verbosity.1` to `su.verbosity.5` (privileged only).
- multiline block input support with continuation prompt (`...`) for `do/end` and `{}` blocks.
- session-state replay for persisted commands.
- user-style prompt (`<user>@LineScript>`) that switches to `superuser@LineScript>` after successful `superuser()` execution.
- cross-editor config bundle at `editor-configs/`:
- universal LSP template
- JetBrains, Notepad++, Neovim, Helix, Sublime Text, Vim (coc.nvim), Emacs (eglot), and Zed.
- REPL privileged helper command: `su.help` (superuser-only) to list shell controls and privileged APIs.
- C++-style class operator methods:
- `fn operator <op>(rhs: T) -> T do ... end`
- runtime regression tests:
- `tests/cases/runtime/operator_override_basic.lsc`
- `tests/cases/runtime/operator_override_member_precedence.lsc`
- compile-fail regression tests:
- `tests/cases/compile_fail/operator_override_bad_arity.lsc`
- `tests/cases/compile_fail/operator_method_bad_arity.lsc`

### Changed
- bumped runtime/compiler version output to `1.4.6` (`--LineScript` now reports `LineScript version 1.4.6`).
- updated CLI usage text with shell-mode help (`--repl`, `--shell`, and no-arg shell behavior).
- `linescript.ps1` and `linescript.sh` now default to REPL when run without source file args.
- REPL superuser handling is session-based and no longer depends on persisting literal `superuser()` lines as normal state.
- REPL `superuser()` now toggles ON/OFF mode.
- superuser warning emission now occurs on activation, not on every REPL command.
- REPL-only print behavior now normalizes `print(...)` to newline output (`println(...)`) so shell prompts never concatenate with output.
- superuser logging now supports levels 1..5 with increasing detail (high levels include syntax summaries and full compile commands).
- superuser verbosity depth now includes lex/parse tracing:
- level 1 stays concise.
- level 5 now logs source stats and token-by-token lexer output in addition to full compile command tracing.
- superuser mode guardrail relaxation was expanded in type-check paths so more LineScript safety checks downgrade to warnings while backend compiler/toolchain validation continues.
- operator overload resolution order is now:
- class member operator overload
- free/global operator overload
- built-in operator fallback
- custom CLI parser now supports strict grouped token streams for advanced layouts (balanced `[` / `]` required).
- grouped/custom parser options no longer emit undefined-flag warnings during normal script execution.
- deterministic harness parity:
- `tests/run_tests.ps1` and `tests/run_tests.sh` both cover grouped CLI parser and operator overload cases.
- packaging scripts now enforce latest-only dist output by cleaning old `LineScript-*` bundles and zips first.
- updated VSCode grammar precedence so `fn` / `inline` keep modifier highlighting before `main()`.
- updated syntax coloring so:
- `main` remains red and non-bold.
- `superuser()` has its own cyan scope.
- updated LineScript VSCode extension package version to `1.4.6`.
- updated API reference header to `v1.4.6`.

### Tests
- updated CLI version assertions in `tests/run_tests.ps1` and `tests/run_tests.sh` for `--LineScript`.
- manual REPL smoke tests:
- no-arg `lsc.exe` starts shell and exits cleanly with `:exit`.
- declare/assign/print flow works interactively.
- `superuser()` changes prompt identity to `superuser`.
- added runtime regression: `tests/cases/runtime/superuser_relaxed_if_non_bool.lsc`.
- added REPL regressions:
- `repl_superuser_help_not_privileged`
- `repl_superuser_help_privileged`
- deterministic suite pass: `226/226`.
- deterministic stress suite pass: `21/21`.

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
- game input expansion:
- `game_scroll_x()` / `game_scroll_y()` for wheel deltas.
- `game_mouse_down(game, button)` / `game_mouse_down_name(game, name)` for mouse buttons.
- pygame-style aliases:
- `pg_scroll_x()` / `pg_scroll_y()`
- `pg_mouse_down(game, button)` / `pg_mouse_down_name(game, name)`
- complete API catalog: `docs/API_REFERENCE.md`.
- VSCode extension scaffold at `vscode-extension/linescript-vscode`:
- language registration (`.lsc`, `.ls`)
- TextMate syntax highlighting
- snippets + completion + hover + definition + symbols
- LSP diagnostics from `lsc --check`
- suggestion diagnostics with quick fixes (including “Are you sure this is correct?” style hints)
- optional file icon theme support for `.lsc/.ls`

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
- docs cross-linked to the complete API reference from `README.md`, `docs/STDLIB.md`, `docs/SYNTAX.md`, and `docs/LANGUAGE_GUIDE.md`.
- `docs/IDE_SETUP.md` now includes VSCode extension install/config steps.
- packaging scripts now include `vscode-extension/linescript-vscode` in distribution.
- VSCode language server now supports `DocumentFormatting` for `.lsc/.ls` files.
- workspace defaults now set LineScript file association, default formatter, format-on-save, and icon theme.
- VSCode flow now uses one-time VSIX install scripts for daily use (Extension Development Host is for extension development only).
- VSCode diagnostics now include expanded user-friendly QoL hints:
- `else if` -> `elif`, `...` -> `..`, missing `do`, Python `:` block style, condition simplification, semicolon and compound-assignment hints.
- added configurable hint controls: `linescript.hintsEnabled`, `linescript.maxHintsPerFile`.
- added semantic lint warnings/errors for undeclared variable usage, undeclared assignment, const reassignment, duplicate declarations, invalid `break`/`continue` context, and unreachable statements.
- added `LineScript + Seti Icons` theme so `.lsc/.ls` can use LineScript logos without turning other file types into generic icons.

### Fixed
- parser ambiguity where `formatOutput(\"...\") do ... end` could be misread as a function declaration.
- output ordering reliability for compiler status lines before execution.
- Windows frontend build macro conflict (`min`/`max`) after adding debug-window support.

### Tests
- added runtime test: `tests/cases/runtime/superuser_privileged.lsc`.
- added compile-fail test: `tests/cases/compile_fail/superuser_not_privileged.lsc`.
- added/updated top-level script tests (`top_level_function_call`, `top_level_most_features`, format CLI checks).
- added runtime test: `tests/cases/runtime/game_scroll_mouse_inputs.lsc`.
- added compile-fail tests:
- `tests/cases/compile_fail/game_scroll_bad_types.lsc`
- `tests/cases/compile_fail/game_mouse_down_bad_types.lsc`
- deterministic suite: `212 / 212` passed.
- extension formatter unit tests: `formatter.test.js` passed.

### Distribution
- rebuilt `dist/LineScript-win64-20260213` and `dist/LineScript-win64-20260213.zip`.
- removed the public demo-only extras file from distribution.
