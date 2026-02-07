# LineScript IDE Setup

This guide shows exactly how to start a fresh LineScript project in VS Code and other IDEs.

## 1. Prerequisites

- `lsc.exe`
- `linescript.cmd` and `linescript.ps1`
- a backend C compiler in `PATH` (`clang` recommended, `gcc` supported)

## 2. Fresh Project From `dist` (Copy-Paste Ready)

1. Extract `dist\LineScript-win64-<date>.zip`.
2. Copy the extracted contents into your new project folder.
3. Create a `.lsc` file (for example `main.lsc`).
4. Run:

```powershell
.\linescript.cmd .\main.lsc --max-speed --cc clang
```

This works without `src/lsc.cpp` because the packaged `lsc.exe` is already included.

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
.\linescript.cmd .\main.lsc --max-speed --cc clang
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

Optional file association in `.vscode/settings.json`:

```json
{
  "files.associations": {
    "*.lsc": "javascript",
    "*.ls": "javascript"
  }
}
```

## 5. Other IDEs

Use the same command in a Run/External Tool configuration:

```text
.\linescript.cmd .\main.lsc --max-speed --cc clang
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
.\linescript.cmd .\src\math.lsc .\src\main.lsc --max-speed --cc clang
```

All functions across listed files are compiled into one native program.
