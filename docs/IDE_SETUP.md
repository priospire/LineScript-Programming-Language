# LineScript IDE Setup

This guide shows exactly how to start a fresh LineScript project in VS Code and other IDEs.

## 1. Prerequisites

- `lsc.exe`
- `linescript.cmd` and `linescript.ps1`
- a backend C compiler in `PATH` (`clang` recommended, `gcc` supported)

Linux:
- build `lsc` from `src/lsc.cpp`
- run `./lsc ./main.lsc -O4 --cc clang` (or `--cc gcc`)

## 2. Fresh Project From `dist` (Copy-Paste Ready)

1. Extract `dist\LineScript-win64-<date>.zip`.
2. Copy the extracted contents into your new project folder.
3. Create a `.lsc` file (for example `main.lsc`).
4. Run:

```powershell
.\linescript.cmd .\main.lsc -O4 --cc clang
```

This works without `src/lsc.cpp` because the packaged `lsc.exe` is already included.

Linux equivalent:

```bash
./lsc ./main.lsc -O4 --cc clang
```

## 3. Fresh Project (Any Editor)

1. Create a folder:

```powershell
mkdir MyLineScriptApp
cd MyLineScriptApp
```

2. Copy into that folder:
- `lsc.exe`
- `linescript.cmd`
- `linescript.ps1`
- `linescript.sh` (Linux/macOS)

3. Create `main.lsc`:

```linescript
main() -> i64 do
  declare x: i64 = 42
  print("x = ")
  println(x)
  println("Hello from LineScript")
  return 0
end
```

4. Run:

```powershell
.\linescript.cmd .\main.lsc -O4 --cc clang
```

Linux/macOS:

```bash
./linescript.sh ./main.lsc -O4 --cc clang
```

5. Check syntax/types only:

```powershell
.\linescript.cmd .\main.lsc --check
```

## 4. VS Code Setup (Recommended)

If you are using VS Code, copy this into your project root:

1. Copy `.vscode/tasks.json` and `.vscode/launch.json` from this repository.
2. Open your project folder in VS Code.
3. Open a `.lsc` or `.ls` file.
4. Run tasks:
- `Terminal -> Run Task -> LineScript: Run Current File`
- `Terminal -> Run Task -> LineScript: Check Current File`
- `Ctrl+Shift+B` runs the default build task (`LineScript: Run Current File`).
5. Run/Debug integration:
- open `Run and Debug` in VS Code
- select `LineScript: Run Current File (via Task)` or `LineScript: Check Current File (via Task)`
- press `F5` to execute the same LineScript task automatically

If you are **not** using VS Code, you can ignore `.vscode/tasks.json` and `.vscode/launch.json`.

Recommended `.vscode/settings.json`:

```json
{
  "files.associations": {
    "*.lsc": "linescript",
    "*.ls": "linescript"
  },
  "[linescript]": {
    "editor.defaultFormatter": "linescript.linescript-vscode",
    "editor.formatOnSave": true
  }
}
```

Optional (only if you want LineScript-specific file icons for the whole workspace):

```json
{
  "workbench.iconTheme": "linescript-seti-icons"
}
```

## 4.1 Install the LineScript VSCode Extension (LSP + Suggestions)

The repository now includes:
- `vscode-extension/linescript-vscode`

This extension adds:
- syntax highlighting for `.lsc` and `.ls`
- autocomplete + snippets
- hover docs
- go-to-definition and outline symbols
- live diagnostics from `lsc --check`
- suggestion diagnostics (for example: `Are you sure this is correct?` on suspicious condition assignments)
- quick fixes for common mistakes

Normal use (recommended, one-time install):

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install_vscode_extension.ps1
```

Linux/macOS:

```bash
bash ./scripts/install_vscode_extension.sh
```

This installs a VSIX directly into your VS Code profile. After that:
- formatting works with `Shift+Alt+F`
- format-on-save works for `.lsc`/`.ls`
- LineScript file icons are applied

`F5` / Extension Development Host is only needed when you are actively developing the extension itself.

Key extension settings:
- `linescript.lscPath`
- `linescript.backendCompiler`
- `linescript.checkOnType`
- `linescript.checkOnSave`
- `linescript.checkTimeoutMs`
- `linescript.extraCheckArgs`
- `linescript.hintsEnabled`
- `linescript.maxHintsPerFile`

File icons:
- the extension includes:
- `LineScript + Seti Icons` (recommended): keeps normal per-language icons and maps `.lsc`/`.ls` to the LineScript logo.
- enable via `Preferences: File Icon Theme`.
- to use your own logo exactly, replace `vscode-extension/linescript-vscode/icons/linescript-file.svg`.

## 5. Other IDEs

Use the same command in a Run/External Tool configuration:

```text
.\linescript.cmd .\main.lsc -O4 --cc clang
```

Linux equivalent:

```text
./lsc ./main.lsc -O4 --cc clang
```

For validation-only builds:

```text
.\linescript.cmd .\main.lsc --check
```

Examples:
- Visual Studio: External Tools / custom build step
- JetBrains IDEs: External Tool or Run Configuration
- Sublime/TextMate/Notepad++: custom build command

## 6. Multi-File Project

Compile modules in one run command:

```powershell
.\linescript.cmd .\src\math.lsc .\src\main.lsc -O4 --cc clang
```

All functions across listed files are compiled into one native program.
