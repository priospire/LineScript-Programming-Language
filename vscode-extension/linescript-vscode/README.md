# LineScript VSCode Extension

Language support for `.lsc` and `.ls`:
- syntax highlighting
- snippets
- autocomplete
- hover docs
- go-to-definition
- document symbols
- diagnostics from `lsc --check`
- suggestions + quick fixes for common mistakes
- document formatting (supports format-on-save)

## Diagnostics and Suggestions

The language server highlights malformed/wrong code and also gives suggestions, including:
- suspicious assignment in condition (`if x = y do`) with message:
- `Are you sure this is correct? ... Did you mean '=='?`
- typo suggestion: `printIn(...)` -> `println(...)`
- ignored `input(...)` return value hints
- unmatched/extra `end` block hints
- `else if` style hint -> suggests `elif`
- `...` range typo hint -> suggests `..`
- missing `do` hints on block starters
- Python-style `:` block hints -> suggests `do`
- condition simplification hints (`== true` / `== false`)
- semicolon/compound-assignment QoL hints
- undeclared variable usage/assignment warnings
- const reassignment warnings
- duplicate declaration warnings
- `break` / `continue` outside-loop warnings
- unreachable statement warnings after `return`/`break`/`continue`
- compiler-level parse/type errors via `lsc --check` (authoritative red squiggles)

Diagnostic model:
- red error squiggles come from compiler diagnostics (`lsc --check`)
- heuristic/editor diagnostics are suggestions/warnings and are intentionally non-fatal

## Settings

Configure in VS Code settings:
- `linescript.lscPath`
- `linescript.backendCompiler`
- `linescript.maxSpeedDiagnostics`
- `linescript.checkOnType`
- `linescript.checkOnSave`
- `linescript.checkTimeoutMs`
- `linescript.extraCheckArgs`
- `linescript.hintsEnabled`
- `linescript.styleHintsEnabled`
- `linescript.maxHintsPerFile`

## File Icon Support

This extension includes a file icon theme for `.lsc` and `.ls`.

To enable:
1. Open Command Palette.
2. Run `Preferences: File Icon Theme`.
3. Select `LineScript + Seti Icons`.

## Use Your Own Icon

If you want your custom LineScript logo as the file icon:
1. Replace `icons/linescript-file.svg` with your design.
2. Reload VS Code window.

## Install For Daily Use (No Extension Host)

From repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install_vscode_extension.ps1
```

Linux/macOS:

```bash
bash ./scripts/install_vscode_extension.sh
```

After install, reload VS Code and set:
- `Preferences: File Icon Theme` -> `LineScript + Seti Icons` (recommended)
- `LineScript` as default formatter for LineScript files (or use workspace settings)

## Local Development

```bash
cd vscode-extension/linescript-vscode
npm install
```

Press `F5` only if you are developing/debugging the extension itself.
