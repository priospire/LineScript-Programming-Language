-- Requires: nvim-lspconfig
-- Place in your Neovim config and load it.

local lspconfig = require("lspconfig")

lspconfig.linescript_lsp = {
  default_config = {
    name = "linescript_lsp",
    cmd = { "node", "vscode-extension/linescript-vscode/server/server.js" },
    filetypes = { "linescript" },
    root_dir = lspconfig.util.root_pattern(".git", "lsc.cpp", "src/lsc.cpp", "README.md"),
    settings = {
      linescript = {
        lscPath = "lsc.exe",
        backendCompiler = "clang",
        checkOnType = true,
        checkOnSave = true,
        checkTimeoutMs = 8000,
        hintsEnabled = true,
        maxHintsPerFile = 120
      }
    }
  }
}

lspconfig.linescript_lsp.setup({})

-- Optional filetype mapping if needed:
vim.filetype.add({
  extension = {
    lsc = "linescript",
    ls = "linescript"
  }
})
