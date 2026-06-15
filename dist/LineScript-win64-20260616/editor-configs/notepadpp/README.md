# Notepad++ Setup

## 1. Syntax Highlighting

1. Open Notepad++.
2. Go to `Language -> User Defined Language -> Define your language...`.
3. Click `Import...`.
4. Import `LineScript-UDL.xml`.
5. Restart Notepad++.

## 2. Run Current File (NppExec)

1. Install plugin: `Plugins Admin -> NppExec`.
2. Open `Plugins -> NppExec -> Execute...`.
3. Paste contents of `NppExec-LineScript-Run.txt`.
4. Save script as `LineScript-Run`.
5. Optional: bind it to a shortcut from `Settings -> Shortcut Mapper -> Plugin commands`.

Run command used:
- `linescript.cmd "<current file>" -O4 --cc clang`

## 3. Optional LSP

Notepad++ does not provide built-in LSP. If you use an LSP plugin:
- command: `node`
- args: `<workspace>/vscode-extension/linescript-vscode/server/server.js`
- file extensions: `.lsc`, `.ls`
