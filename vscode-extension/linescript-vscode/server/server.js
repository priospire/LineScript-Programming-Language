"use strict";

const { execFile } = require("child_process");
const fs = require("fs");
const os = require("os");
const path = require("path");
const {
  createConnection,
  ProposedFeatures,
  TextDocuments,
  DiagnosticSeverity,
  CompletionItemKind,
  InsertTextFormat,
  TextDocumentSyncKind,
  CodeActionKind,
  SymbolKind
} = require("vscode-languageserver/node");
const { TextDocument } = require("vscode-languageserver-textdocument");
const { URI } = require("vscode-uri");
const { formatLineScript } = require("./formatter");
const { collectHeuristicDiagnostics } = require("./hints");

const connection = createConnection(ProposedFeatures.all);
const documents = new TextDocuments(TextDocument);

const defaultSettings = {
  lscPath: "",
  backendCompiler: "",
  maxSpeedDiagnostics: false,
  checkOnType: true,
  checkOnSave: true,
  checkTimeoutMs: 8000,
  extraCheckArgs: [],
  hintsEnabled: true,
  styleHintsEnabled: false,
  maxHintsPerFile: 120
};

let hasConfigurationCapability = false;
const documentSettings = new Map();
const validationTimers = new Map();

const keywordDocs = new Map([
  ["declare", "Declare a variable. Example: declare x: i64 = 10"],
  ["const", "Immutable variable modifier used with declare const."],
  ["owned", "Owned handle variable with deterministic scope-exit release rules."],
  ["if", "Conditional block: if <cond> do ... end"],
  ["elif", "Else-if branch inside if blocks."],
  ["else", "Else branch inside if blocks."],
  ["unless", "Inverse conditional: unless <cond> do ... end"],
  ["while", "While loop."],
  ["for", "Range loop: for i in a..b [step s] do ... end"],
  ["parallel", "Parallel loop modifier: parallel for ..."],
  ["class", "Class declaration (C++-style OOP)."],
  ["throws", "Function throws contract declaration."],
  ["do", "Block opener for LineScript block syntax."],
  ["end", "Block terminator for LineScript block syntax."]
]);

const builtinSignatures = [
  "print(x) -> void",
  "println(x) -> void",
  "input() -> str",
  "input(prompt: str) -> str",
  "input_i64() -> i64",
  "input_f64() -> f64",
  "clock_ms() -> i64",
  "clock_us() -> i64",
  ".format() -> void",
  ".stateSpeed() -> void",
  ".freeConsole() -> void",
  "superuser() -> void",
  "su.trace.on() -> void",
  "su.trace.off() -> void",
  "su.capabilities() -> str",
  "su.memory.inspect() -> str",
  "su.limit.set(step_limit: i64, mem_limit: i64) -> void",
  "su.compiler.inspect() -> str",
  "su.ir.dump() -> void",
  "su.debug.hook(tag: str) -> void",
  "http_server_listen(port: i64) -> i64",
  "http_server_accept(server: i64) -> i64",
  "http_server_read(client: i64) -> str",
  "http_server_respond_text(client: i64, status: i64, body: str) -> void",
  "http_client_connect(host: str, port: i64) -> i64",
  "http_client_send(client: i64, data: str) -> void",
  "http_client_read(client: i64) -> str",
  "len(s: str) -> i64",
  "contains(s: str, part: str) -> bool",
  "replace(s: str, from: str, to: str) -> str",
  "array_new() -> i64",
  "array_len(arr: i64) -> i64",
  "array_push(arr: i64, value: str) -> void",
  "array_get(arr: i64, idx: i64) -> str",
  "array_set(arr: i64, idx: i64, value: str) -> void",
  "dict_new() -> i64",
  "dict_set(d: i64, key: str, value: str) -> void",
  "dict_get(d: i64, key: str) -> str",
  "option_some(value: str) -> i64",
  "option_none() -> i64",
  "result_ok(value: str) -> i64",
  "result_err(type: str, message: str) -> i64",
  "mem_alloc(bytes: i64) -> i64",
  "mem_realloc(ptr: i64, bytes: i64) -> i64",
  "mem_free(ptr: i64) -> void",
  "gfx_new(w: i64, h: i64) -> i64",
  "gfx_clear(g: i64, r: i64, g: i64, b: i64) -> void",
  "gfx_line(...) -> void",
  "gfx_rect(...) -> void",
  "game_new(width: i64, height: i64, title: str, visible: bool) -> i64",
  "game_begin(game: i64) -> void",
  "game_end(game: i64) -> void",
  "game_should_close(game: i64) -> bool",
  "game_mouse_x(game: i64) -> f64",
  "game_mouse_y(game: i64) -> f64",
  "game_mouse_norm_x(game: i64) -> f64",
  "game_mouse_norm_y(game: i64) -> f64",
  "game_scroll_x(game: i64) -> f64",
  "game_scroll_y(game: i64) -> f64",
  "game_mouse_down(game: i64, button: i64) -> bool",
  "game_mouse_down_name(game: i64, name: str) -> bool",
  "pg_init(...) -> i64",
  "pg_key_down(code: i64) -> bool",
  "pg_key_down_name(name: str) -> bool",
  "pg_scroll_x(game: i64) -> f64",
  "pg_scroll_y(game: i64) -> f64",
  "pg_mouse_down(game: i64, button: i64) -> bool",
  "pg_mouse_down_name(game: i64, name: str) -> bool",
  "phys_new(x: f64, y: f64, z: f64, mass: f64, soft: bool) -> i64",
  "phys_apply_force(obj: i64, fx: f64, fy: f64, fz: f64) -> void",
  "phys_step(dt: f64) -> void",
  "camera_bind(obj: i64) -> void",
  "camera_target() -> i64",
  "camera_set_offset(x: f64, y: f64, z: f64) -> void",
  "key_down(code: i64) -> bool",
  "key_down_name(name: str) -> bool",
  "np_new(n: i64) -> i64",
  "np_sum(v: i64) -> f64",
  "np_dot(a: i64, b: i64) -> f64",
  "parse_i64(s: str) -> i64",
  "parse_f64(s: str) -> f64",
  "to_i32(x: i64) -> i32",
  "to_i64(x: f64) -> i64",
  "to_f32(x: f64) -> f32",
  "to_f64(x: i64) -> f64",
  "gcd(a: i64, b: i64) -> i64",
  "lcm(a: i64, b: i64) -> i64",
  "max(a, b) -> numeric",
  "min(a, b) -> numeric",
  "abs(x) -> numeric",
  "clamp(x, lo, hi) -> numeric",
  "sqrt(x: f64) -> f64",
  "sin(x: f64) -> f64",
  "cos(x: f64) -> f64",
  "tan(x: f64) -> f64",
  "pow(a: f64, b: f64) -> f64"
];

const builtinDocs = new Map(
  builtinSignatures.map((sig) => {
    const name = sig.split("(")[0].trim();
    return [name, sig];
  })
);

const completionItems = [
  ...[
    "declare",
    "const",
    "owned",
    "return",
    "if",
    "elif",
    "else",
    "unless",
    "while",
    "for",
    "parallel",
    "in",
    "step",
    "do",
    "end",
    "class",
    "throws",
    "break",
    "continue",
    "and",
    "or",
    "not",
    "true",
    "false"
  ].map((k) => ({
    label: k,
    kind: CompletionItemKind.Keyword,
    detail: "keyword"
  })),
  ...[
    "i32",
    "i64",
    "f32",
    "f64",
    "bool",
    "str",
    "void"
  ].map((t) => ({
    label: t,
    kind: CompletionItemKind.TypeParameter,
    detail: "type"
  })),
  ...builtinSignatures.map((sig) => {
    const name = sig.split("(")[0].trim();
    return {
      label: name,
      kind: CompletionItemKind.Function,
      detail: sig,
      insertText: name.includes(".") ? name : `${name}($1)`,
      insertTextFormat: name.includes(".") ? InsertTextFormat.PlainText : InsertTextFormat.Snippet
    };
  }),
  {
    label: "if...do...end",
    kind: CompletionItemKind.Snippet,
    detail: "if block snippet",
    insertText: "if ${1:condition} do\n  ${0}\nend",
    insertTextFormat: InsertTextFormat.Snippet
  },
  {
    label: "for...do...end",
    kind: CompletionItemKind.Snippet,
    detail: "for range snippet",
    insertText: "for ${1:i} in ${2:0}..${3:n} do\n  ${0}\nend",
    insertTextFormat: InsertTextFormat.Snippet
  },
  {
    label: "function",
    kind: CompletionItemKind.Snippet,
    detail: "function snippet",
    insertText: "${1:name}(${2}) -> ${3:i64} do\n  ${0:return 0}\nend",
    insertTextFormat: InsertTextFormat.Snippet
  }
];

function getDefaultLscPath() {
  return os.platform() === "win32" ? "lsc.exe" : "lsc";
}

connection.onInitialize((params) => {
  hasConfigurationCapability = !!(params.capabilities.workspace && params.capabilities.workspace.configuration);
  return {
    capabilities: {
      textDocumentSync: TextDocumentSyncKind.Incremental,
      completionProvider: {
        resolveProvider: false
      },
      hoverProvider: true,
      definitionProvider: true,
      documentSymbolProvider: true,
      documentFormattingProvider: true,
      codeActionProvider: {
        codeActionKinds: [CodeActionKind.QuickFix]
      }
    }
  };
});

connection.onInitialized(() => {
  if (hasConfigurationCapability) {
    connection.client.register("workspace/didChangeConfiguration");
  }
});

connection.onDidChangeConfiguration((change) => {
  if (hasConfigurationCapability) {
    documentSettings.clear();
  } else {
    Object.assign(defaultSettings, (change.settings && change.settings.linescript) || {});
  }
  documents.all().forEach((doc) => scheduleValidate(doc, true));
});

documents.onDidClose((event) => {
  documentSettings.delete(event.document.uri);
  validationTimers.forEach((timer, key) => {
    if (key === event.document.uri) {
      clearTimeout(timer);
      validationTimers.delete(key);
    }
  });
  connection.sendDiagnostics({ uri: event.document.uri, diagnostics: [] });
});

documents.onDidOpen((event) => {
  scheduleValidate(event.document, true);
});

documents.onDidSave((event) => {
  scheduleValidate(event.document, true);
});

documents.onDidChangeContent((change) => {
  scheduleValidate(change.document, false);
});

connection.onCompletion(() => completionItems);

connection.onHover((params) => {
  const doc = documents.get(params.textDocument.uri);
  if (!doc) return null;
  const token = getTokenAtPosition(doc, params.position);
  if (!token) return null;

  if (builtinDocs.has(token)) {
    return {
      contents: {
        kind: "markdown",
        value: `\`${builtinDocs.get(token)}\``
      }
    };
  }
  if (keywordDocs.has(token)) {
    return {
      contents: {
        kind: "markdown",
        value: `**${token}**: ${keywordDocs.get(token)}`
      }
    };
  }
  return null;
});

connection.onDefinition((params) => {
  const doc = documents.get(params.textDocument.uri);
  if (!doc) return null;
  const token = getTokenAtPosition(doc, params.position);
  if (!token) return null;
  const text = doc.getText();
  const lines = text.split(/\r?\n/);
  const escaped = escapeRegex(token);

  const patterns = [
    new RegExp(`^\\s*(?:inline\\s+|extern\\s+|fn\\s+|func\\s+)*${escaped}\\s*\\(`),
    new RegExp(`^\\s*flag\\s+${escaped}\\s*\\(`),
    new RegExp(`^\\s*class\\s+${escaped}\\b`)
  ];

  for (let i = 0; i < lines.length; i += 1) {
    for (const re of patterns) {
      if (re.test(lines[i])) {
        const col = Math.max(0, lines[i].indexOf(token));
        return {
          uri: doc.uri,
          range: {
            start: { line: i, character: col },
            end: { line: i, character: col + token.length }
          }
        };
      }
    }
  }
  return null;
});

connection.onDocumentSymbol((params) => {
  const doc = documents.get(params.textDocument.uri);
  if (!doc) return [];
  const lines = doc.getText().split(/\r?\n/);
  const out = [];

  for (let i = 0; i < lines.length; i += 1) {
    const line = lines[i];
    let m = line.match(/^\s*class\s+([A-Za-z_][A-Za-z0-9_]*)\b/);
    if (m) {
      const n = m[1];
      const c = line.indexOf(n);
      out.push({
        name: n,
        kind: SymbolKind.Class,
        range: {
          start: { line: i, character: 0 },
          end: { line: i, character: line.length }
        },
        selectionRange: {
          start: { line: i, character: c },
          end: { line: i, character: c + n.length }
        }
      });
      continue;
    }

    m = line.match(/^\s*flag\s+([A-Za-z_][A-Za-z0-9_\-]*)\s*\(/);
    if (m) {
      const n = m[1];
      const c = line.indexOf(n);
      out.push({
        name: `flag ${n}`,
        kind: SymbolKind.Function,
        range: {
          start: { line: i, character: 0 },
          end: { line: i, character: line.length }
        },
        selectionRange: {
          start: { line: i, character: c },
          end: { line: i, character: c + n.length }
        }
      });
      continue;
    }

    m = line.match(/^\s*(?:inline\s+|extern\s+|fn\s+|func\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*\(/);
    if (m) {
      const n = m[1];
      const c = line.indexOf(n);
      out.push({
        name: n,
        kind: SymbolKind.Function,
        range: {
          start: { line: i, character: 0 },
          end: { line: i, character: line.length }
        },
        selectionRange: {
          start: { line: i, character: c },
          end: { line: i, character: c + n.length }
        }
      });
    }
  }
  return out;
});

connection.onCodeAction((params) => {
  const actions = [];
  const pushQuickFix = (diag, title, newText) => {
    if (typeof newText !== "string") return;
    actions.push({
      title,
      kind: CodeActionKind.QuickFix,
      diagnostics: [diag],
      edit: {
        changes: {
          [params.textDocument.uri]: [
            {
              range: diag.range,
              newText
            }
          ]
        }
      }
    });
  };

  for (const d of params.context.diagnostics) {
    if (d.data && d.data.quickFix && typeof d.data.quickFix.newText === "string") {
      pushQuickFix(
        d,
        d.data.quickFix.title || "Apply suggested fix",
        d.data.quickFix.newText
      );
      continue;
    }

    if (d.code === "ls-assign-in-condition") {
      pushQuickFix(d, "Use '==' for comparison", "==");
    } else if (d.code === "ls-printin-typo") {
      pushQuickFix(d, "Replace 'printIn' with 'println'", "println");
    }
  }
  return actions;
});

connection.onDocumentFormatting((params) => {
  const doc = documents.get(params.textDocument.uri);
  if (!doc) return [];

  const formatted = formatLineScript(doc.getText(), params.options || {});
  const text = doc.getText();
  if (formatted === text) return [];

  return [
    {
      range: {
        start: { line: 0, character: 0 },
        end: doc.positionAt(text.length)
      },
      newText: formatted
    }
  ];
});

function scheduleValidate(document, forceCompiler) {
  const key = document.uri;
  const old = validationTimers.get(key);
  if (old) clearTimeout(old);
  const timer = setTimeout(() => {
    validationTimers.delete(key);
    validateTextDocument(document, !!forceCompiler).catch((err) => {
      connection.console.error("LineScript validate error: " + (err && err.message ? err.message : String(err)));
    });
  }, 220);
  validationTimers.set(key, timer);
}

async function getDocumentSettings(resource) {
  if (!hasConfigurationCapability) return defaultSettings;
  let result = documentSettings.get(resource);
  if (!result) {
    result = connection.workspace.getConfiguration({
      scopeUri: resource,
      section: "linescript"
    });
    documentSettings.set(resource, result);
  }
  return result;
}

function parseLineColDiagnostics(output, severity) {
  const diagnostics = [];
  const re = /line\s+(\d+)\s*,\s*col\s+(\d+)\s*:\s*([^\r\n]+)/g;
  let m;
  while ((m = re.exec(output)) !== null) {
    const line = Math.max(0, Number(m[1]) - 1);
    const col = Math.max(0, Number(m[2]) - 1);
    const msg = String(m[3]).trim();
    diagnostics.push({
      severity,
      source: "lsc",
      message: msg,
      range: {
        start: { line, character: col },
        end: { line, character: col + 1 }
      }
    });
  }
  return diagnostics;
}

function runCompilerCheck(lscPath, args, timeoutMs) {
  return new Promise((resolve) => {
    execFile(lscPath, args, { timeout: timeoutMs }, (error, stdout, stderr) => {
      const out = String(stdout || "") + "\n" + String(stderr || "");
      if (error) {
        resolve({ ok: false, output: out.trim(), code: typeof error.code === "number" ? error.code : 1 });
        return;
      }
      resolve({ ok: true, output: out.trim(), code: 0 });
    });
  });
}

function normalizeHeuristicDiagnostics(diagnostics) {
  const out = [];
  for (const d of diagnostics || []) {
    if (!d || typeof d !== "object") continue;
    const copy = Object.assign({}, d);
    if (copy.severity === DiagnosticSeverity.Error) {
      // Red squiggles are reserved for compiler-authenticated failures.
      copy.severity = DiagnosticSeverity.Warning;
    }
    if (!copy.source) {
      copy.source = "lsp";
    }
    out.push(copy);
  }
  return out;
}

async function validateTextDocument(document, forceCompiler) {
  const settings = await getDocumentSettings(document.uri);
  const merged = Object.assign({}, defaultSettings, settings || {});
  const heuristicDiagnostics = collectHeuristicDiagnostics(document, merged);
  const diagnostics = normalizeHeuristicDiagnostics(heuristicDiagnostics);

  const shouldRunCompiler = forceCompiler
    ? merged.checkOnSave
    : merged.checkOnType;

  const isFileDoc = document.uri.startsWith("file:");
  if (shouldRunCompiler && isFileDoc) {
    let filePath = "";
    try {
      filePath = URI.parse(document.uri).fsPath;
    } catch (_err) {
      filePath = "";
    }

    if (filePath && fs.existsSync(filePath)) {
      const lscPath = (merged.lscPath || "").trim() || getDefaultLscPath();
      const args = [filePath, "--check"];
      if (merged.maxSpeedDiagnostics) args.push("--max-speed");
      if ((merged.backendCompiler || "").trim()) {
        args.push("--cc", merged.backendCompiler.trim());
      }
      if (Array.isArray(merged.extraCheckArgs)) {
        for (const a of merged.extraCheckArgs) {
          if (typeof a === "string" && a.trim()) args.push(a.trim());
        }
      }

      const check = await runCompilerCheck(lscPath, args, Number(merged.checkTimeoutMs) || 8000);
      const compilerErrors = parseLineColDiagnostics(check.output, DiagnosticSeverity.Error);
      diagnostics.push(...compilerErrors);

      if (!check.ok && compilerErrors.length === 0) {
        const msg = (check.output || "LineScript check failed.").split(/\r?\n/).find((x) => x && x.trim()) ||
          "LineScript check failed.";
        diagnostics.push({
          severity: DiagnosticSeverity.Error,
          source: "lsc",
          message: msg.trim(),
          range: {
            start: { line: 0, character: 0 },
            end: { line: 0, character: 1 }
          }
        });
      }
    }
  }

  connection.sendDiagnostics({
    uri: document.uri,
    diagnostics
  });
}

function getTokenAtPosition(document, position) {
  const line = document.getText({
    start: { line: position.line, character: 0 },
    end: { line: position.line + 1, character: 0 }
  });
  const idx = Math.min(Math.max(position.character, 0), line.length);
  let start = idx;
  let end = idx;

  const isWord = (ch) => /[A-Za-z0-9_.-]/.test(ch);
  while (start > 0 && isWord(line[start - 1])) start -= 1;
  while (end < line.length && isWord(line[end])) end += 1;
  const token = line.slice(start, end).trim();
  return token || null;
}

function escapeRegex(s) {
  return s.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

documents.listen(connection);
connection.listen();
