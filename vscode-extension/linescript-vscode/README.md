# LineScript VSCode Extension

Language support for `.lsc` and `.ls`:
- syntax highlighting
- snippets
- autocomplete
- hover docs
- go-to-definition
- document symbols
- diagnostics from `lsc --check`
- optional suggestions + quick fixes for common mistakes
- document formatting (supports format-on-save)

## Diagnostics and Suggestions

Default behavior is compiler-authoritative:
- diagnostics come from `lsc --check`
- no heuristic guessing by the editor

If you want heuristic editor hints/suggestions, set:
- `linescript.compilerDiagnosticsOnly = false`
- `linescript.hintsEnabled = true`

Diagnostic model:
- compiler diagnostics are authoritative
- optional heuristic/editor diagnostics are non-authoritative and disabled by default

## Settings

Configure in VS Code settings:
- `linescript.lscPath`
- `linescript.backendCompiler`
- `linescript.maxSpeedDiagnostics`
- `linescript.compilerDiagnosticsOnly`
- `linescript.checkOnType`
- `linescript.checkOnSave`
- `linescript.checkTimeoutMs`
- `linescript.extraCheckArgs`
- `linescript.hintsEnabled`
- `linescript.styleHintsEnabled`
- `linescript.maxHintsPerFile`

Compiler path resolution:
- if `linescript.lscPath` is empty, the extension first looks for a local `lsc.exe`/`lsc` by walking up from the current file, then falls back to PATH.

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
