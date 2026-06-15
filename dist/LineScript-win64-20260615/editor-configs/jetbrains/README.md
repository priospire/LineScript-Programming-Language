# JetBrains Setup (IntelliJ, CLion, PyCharm, WebStorm, etc.)

## 1. File type association

Go to:
- `Settings -> Editor -> File Types`

Add patterns:
- `*.lsc`
- `*.ls` (optional; use only if not conflicting in your setup)

## 2. External tools (run/check)

Option A:
- import `externalTools.xml` from this folder into your JetBrains config directory.

Option B:
- create tools manually from the values in that XML.

## 3. LSP support

Install plugin:
- `LSP4IJ`

Create an LSP server entry:
- Name: `LineScript`
- Command: `node`
- Args: `$ProjectFileDir$/vscode-extension/linescript-vscode/server/server.js`
- File types/patterns: `*.lsc`, `*.ls`

Recommended project-level settings:
- `linescript.lscPath = $ProjectFileDir$/lsc.exe` (or `lsc` on Linux/macOS)
- `linescript.backendCompiler = clang`

## 4. Run command (quick reference)

Windows:
- `$ProjectFileDir$/linescript.cmd $FilePath$ -O4 --cc clang`

Linux/macOS:
- `$ProjectFileDir$/linescript.sh $FilePath$ -O4 --cc clang`
