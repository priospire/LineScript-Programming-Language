;; Put this in your init.el (or load from file).
;; Requires eglot.

(with-eval-after-load 'eglot
  (add-to-list 'auto-mode-alist '("\\.lsc\\'" . prog-mode))
  (add-to-list 'auto-mode-alist '("\\.ls\\'" . prog-mode))
  (add-to-list 'eglot-server-programs
               '(prog-mode . ("node" "vscode-extension/linescript-vscode/server/server.js"))))

;; Optional local variables / defaults:
;; (setq-default linescript-lsc-path "lsc.exe")
