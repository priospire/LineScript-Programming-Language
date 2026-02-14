# LineScript Editor Configs

Ready-to-use configs for common IDEs/editors.

These configs assume this repository layout and that the LineScript language server is at:

`vscode-extension/linescript-vscode/server/server.js`

Requirements:
- `node` installed
- run `npm install` inside `vscode-extension/linescript-vscode` once
- `lsc`/`lsc.exe` available in PATH (or set an absolute path in editor settings)

## Included

- universal template:
  - `universal/lsp-template.json`
- JetBrains:
  - `jetbrains/externalTools.xml`
  - `jetbrains/README.md`
- Notepad++:
  - `notepadpp/LineScript-UDL.xml`
  - `notepadpp/NppExec-LineScript-Run.txt`
  - `notepadpp/README.md`
- Neovim:
  - `neovim/linescript.lua`
- Helix:
  - `helix/languages.toml`
- Sublime Text:
  - `sublime/LineScript.sublime-build`
  - `sublime/LSP-linescript.sublime-settings`
- Vim (coc.nvim):
  - `vim/coc-settings.json`
- Emacs (eglot):
  - `emacs/linescript-eglot.el`
- Zed:
  - `zed/settings.json`

## Notes

- syntax highlighting quality depends on each IDE/editor engine.
- diagnostics/completions/hover/formatting come from the shared LSP server.
- `.ls` can conflict with other languages in some tools; prefer `.lsc` if needed.
