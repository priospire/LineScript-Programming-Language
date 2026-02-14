# LineScript Changelog

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
