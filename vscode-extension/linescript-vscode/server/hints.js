"use strict";

const { DiagnosticSeverity } = require("vscode-languageserver/node");

const HINT_SEVERITIES = new Set([
  DiagnosticSeverity.Hint,
  DiagnosticSeverity.Information
]);

const KEYWORDS = new Set([
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
  "constructor",
  "throws",
  "break",
  "continue",
  "and",
  "or",
  "not",
  "true",
  "false",
  "flag",
  "inline",
  "extern",
  "fn",
  "func"
]);

const TYPE_NAMES = new Set([
  "i32",
  "i64",
  "f32",
  "f64",
  "bool",
  "str",
  "void"
]);

const CORE_BUILTINS = new Set([
  "print",
  "println",
  "input",
  "input_i64",
  "input_f64",
  "clock_ms",
  "clock_us",
  "len",
  "contains",
  "includes",
  "replace",
  "trim",
  "lower",
  "upper",
  "substring",
  "repeat",
  "reverse",
  "byte_at",
  "ord",
  "chr",
  "bytes_len",
  "is_empty",
  "starts_with",
  "ends_with",
  "find",
  "array_join",
  "max_f64",
  "clamp_f64",
  "clamp_i64",
  "deg_to_rad",
  "rad_to_deg",
  "sum_sq",
  "sqrt",
  "sin",
  "cos",
  "tan",
  "asin",
  "acos",
  "atan",
  "atan2",
  "exp",
  "log",
  "log10",
  "floor",
  "ceil",
  "round",
  "pow",
  "gcd",
  "lcm",
  "max",
  "min",
  "abs",
  "clamp",
  "parse_i64",
  "parse_f64",
  "to_i32",
  "to_i64",
  "to_f32",
  "to_f64",
  "stateSpeed",
  "formatOutput",
  "FormatOutput",
  "superuser",
  "this",
  "pi",
  "tau",
  "true",
  "false"
]);

const BUILTIN_PREFIXES = [
  "array_",
  "dict_",
  "map_",
  "object_",
  "option_",
  "result_",
  "mem_",
  "gfx_",
  "game_",
  "pg_",
  "np_",
  "phys_",
  "camera_",
  "key_",
  "http_",
  "su_"
];

const STRICT_RESERVED_FUNCTION_NAMES = new Set([
  "print",
  "println",
  "input",
  "input_i64",
  "input_f64",
  "formatOutput",
  "FormatOutput",
  "stateSpeed",
  "superuser"
]);

const BUILTIN_ARITY = new Map([
  ["print", { min: 1, max: 1 }],
  ["println", { min: 1, max: 1 }],
  ["input", { min: 0, max: 1 }],
  ["input_i64", { min: 0, max: 1 }],
  ["input_f64", { min: 0, max: 1 }],
  ["clock_ms", { min: 0, max: 0 }],
  ["clock_us", { min: 0, max: 0 }],
  ["len", { min: 1, max: 1 }],
  ["contains", { min: 2, max: 2 }],
  ["includes", { min: 2, max: 2 }],
  ["replace", { min: 3, max: 3 }],
  ["trim", { min: 1, max: 1 }],
  ["lower", { min: 1, max: 1 }],
  ["upper", { min: 1, max: 1 }],
  ["substring", { min: 3, max: 3 }],
  ["repeat", { min: 2, max: 2 }],
  ["reverse", { min: 1, max: 1 }],
  ["byte_at", { min: 2, max: 2 }],
  ["ord", { min: 1, max: 1 }],
  ["chr", { min: 1, max: 1 }],
  ["bytes_len", { min: 1, max: 1 }],
  ["is_empty", { min: 1, max: 1 }],
  ["starts_with", { min: 2, max: 2 }],
  ["ends_with", { min: 2, max: 2 }],
  ["find", { min: 2, max: 2 }],
  ["array_join", { min: 2, max: 2 }],
  ["max_f64", { min: 2, max: 2 }],
  ["clamp_f64", { min: 3, max: 3 }],
  ["clamp_i64", { min: 3, max: 3 }],
  ["deg_to_rad", { min: 1, max: 1 }],
  ["rad_to_deg", { min: 1, max: 1 }],
  ["sqrt", { min: 1, max: 1 }],
  ["sin", { min: 1, max: 1 }],
  ["cos", { min: 1, max: 1 }],
  ["tan", { min: 1, max: 1 }],
  ["asin", { min: 1, max: 1 }],
  ["acos", { min: 1, max: 1 }],
  ["atan", { min: 1, max: 1 }],
  ["atan2", { min: 2, max: 2 }],
  ["exp", { min: 1, max: 1 }],
  ["log", { min: 1, max: 1 }],
  ["log10", { min: 1, max: 1 }],
  ["floor", { min: 1, max: 1 }],
  ["ceil", { min: 1, max: 1 }],
  ["round", { min: 1, max: 1 }],
  ["pow", { min: 2, max: 2 }],
  ["gcd", { min: 2, max: 2 }],
  ["lcm", { min: 2, max: 2 }],
  ["max", { min: 2, max: 2 }],
  ["min", { min: 2, max: 2 }],
  ["abs", { min: 1, max: 1 }],
  ["clamp", { min: 3, max: 3 }],
  ["parse_i64", { min: 1, max: 1 }],
  ["parse_f64", { min: 1, max: 1 }],
  ["to_i32", { min: 1, max: 1 }],
  ["to_i64", { min: 1, max: 1 }],
  ["to_f32", { min: 1, max: 1 }],
  ["to_f64", { min: 1, max: 1 }],
  ["stateSpeed", { min: 0, max: 0 }],
  ["formatOutput", { min: 1, max: 1 }],
  ["FormatOutput", { min: 1, max: 1 }],
  ["superuser", { min: 0, max: 0 }],
  ["spawn", { min: 1, max: 1 }],
  ["await", { min: 1, max: 1 }],
  ["await_all", { min: 0, max: 0 }],
  ["array_new", { min: 0, max: 0 }],
  ["array_len", { min: 1, max: 1 }],
  ["array_push", { min: 2, max: 2 }],
  ["array_get", { min: 2, max: 2 }],
  ["array_set", { min: 3, max: 3 }],
  ["dict_new", { min: 0, max: 0 }],
  ["dict_set", { min: 3, max: 3 }],
  ["dict_get", { min: 2, max: 2 }]
]);

const DOT_BUILTINS = new Map([
  [".format", { min: 0, max: 0 }],
  [".formatOutput", { min: 0, max: 0 }],
  [".stateSpeed", { min: 0, max: 0 }],
  [".freeConsole", { min: 0, max: 0 }],
  ["su.trace.on", { min: 0, max: 0 }],
  ["su.trace.off", { min: 0, max: 0 }],
  ["su.capabilities", { min: 0, max: 0 }],
  ["su.memory.inspect", { min: 0, max: 0 }],
  ["su.compiler.inspect", { min: 0, max: 0 }],
  ["su.ir.dump", { min: 0, max: 0 }],
  ["su.debug.hook", { min: 1, max: 1 }],
  ["su.limit.set", { min: 2, max: 2 }]
]);

const FUNCTION_DECL_RE =
  /^\s*(?:(inline|extern|fn|func)\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*(?:->\s*([A-Za-z_][A-Za-z0-9_]*))?\s*(?:throws\s+[A-Za-z_][A-Za-z0-9_]*(?:\s*,\s*[A-Za-z_][A-Za-z0-9_]*)*)?\s*(do|\{)?\s*$/;
const FLAG_DECL_RE = /^\s*flag\s+([A-Za-z_][A-Za-z0-9_\-]*)\s*\(([^)]*)\)\s*(do|\{)?\s*$/;

function clampMaxHints(value) {
  const n = Number(value);
  if (!Number.isFinite(n) || n < 1) return 120;
  return Math.min(1000, Math.floor(n));
}

function maskCodePreserveLength(line) {
  const out = line.split("");
  let inString = false;
  let escaping = false;

  for (let i = 0; i < line.length; i += 1) {
    const ch = line[i];
    const next = i + 1 < line.length ? line[i + 1] : "";

    if (!inString && ch === "/" && next === "/") {
      for (let j = i; j < line.length; j += 1) out[j] = " ";
      break;
    }

    if (inString) {
      out[i] = " ";
      if (escaping) {
        escaping = false;
      } else if (ch === "\\") {
        escaping = true;
      } else if (ch === "\"") {
        inString = false;
      }
      continue;
    }

    if (ch === "\"") {
      out[i] = " ";
      inString = true;
    }
  }

  return out.join("");
}

function findCodeEnd(maskedLine) {
  for (let i = maskedLine.length - 1; i >= 0; i -= 1) {
    if (!/\s/.test(maskedLine[i])) return i + 1;
  }
  return 0;
}

function makeDiagnostic(line, start, end, severity, code, message, quickFix) {
  const d = {
    severity,
    source: "ls-lsp",
    code,
    message,
    range: {
      start: { line, character: Math.max(0, start) },
      end: { line, character: Math.max(Math.max(0, start), end) }
    }
  };
  if (quickFix && typeof quickFix.newText === "string") {
    d.data = {
      quickFix: {
        title: quickFix.title || "Apply suggested fix",
        newText: quickFix.newText
      }
    };
  }
  return d;
}

function startsWithWord(codePart, word) {
  return new RegExp("^\\s*" + word + "\\b").test(codePart);
}

function nextNonWsChar(text, idx) {
  for (let i = idx; i < text.length; i += 1) {
    if (!/\s/.test(text[i])) return text[i];
  }
  return "";
}

function prevNonWsIndex(text, idx) {
  for (let i = idx; i >= 0; i -= 1) {
    if (!/\s/.test(text[i])) return i;
  }
  return -1;
}

function isBuiltinToken(token) {
  if (CORE_BUILTINS.has(token)) return true;
  for (const p of BUILTIN_PREFIXES) {
    if (token.startsWith(p)) return true;
  }
  return false;
}

function findSingleEqualsInCondition(line) {
  const kw = line.match(/\b(if|elif|while|unless)\b/);
  if (!kw) return -1;
  const start = kw.index + kw[0].length;
  const doIdx = line.indexOf(" do", start);
  const end = doIdx >= 0 ? doIdx : line.length;
  for (let i = start; i < end; i += 1) {
    if (line[i] !== "=") continue;
    const prev = i > 0 ? line[i - 1] : "";
    const next = i + 1 < line.length ? line[i + 1] : "";
    if (prev === "=" || prev === "!" || prev === "<" || prev === ">" || next === "=") continue;
    return i;
  }
  return -1;
}

function createScope(kind) {
  return {
    kind,
    terminated: false,
    symbols: new Map()
  };
}

function findSymbol(scopeStack, name) {
  for (let i = scopeStack.length - 1; i >= 0; i -= 1) {
    const s = scopeStack[i];
    if (s.symbols.has(name)) return s.symbols.get(name);
  }
  return null;
}

function declareSymbol(scope, name, meta) {
  if (scope.symbols.has(name)) return false;
  scope.symbols.set(name, meta);
  return true;
}

function isLoopScopeActive(scopeStack) {
  for (let i = scopeStack.length - 1; i >= 0; i -= 1) {
    if (scopeStack[i].kind === "loop") return true;
  }
  return false;
}

function shouldTreatAsFunctionDeclaration(match) {
  if (!match) return false;
  const hasKeywordOrSig = !!(match[1] || match[4]);
  if (hasKeywordOrSig) return true;
  if (!match[5]) return false;
  const paramsInfo = analyzeParameterList(match[3]);
  return paramsInfo.malformedParts.length === 0;
}

function normalizeFunctionDeclPrefix(line) {
  if (!line) return line;
  let out = line;
  out = out.replace(/^(\s*)(?:inline|extern)\s+(fn|func)\s+/, "$1$2 ");
  out = out.replace(/^(\s*)fn\s+(?:inline|extern)\s+/, "$1fn ");
  out = out.replace(/^(\s*)func\s+(?:inline|extern)\s+/, "$1func ");
  return out;
}

function analyzeParameterList(text) {
  const out = {
    count: 0,
    names: [],
    malformedParts: [],
    duplicateNames: []
  };
  const seen = new Set();
  if (!text || !text.trim()) return out;

  const chunks = text.split(",");
  for (let idx = 0; idx < chunks.length; idx += 1) {
    const part = chunks[idx].trim();
    if (!part) {
      out.malformedParts.push(`parameter ${idx + 1} is empty`);
      continue;
    }
    const m = part.match(/^([A-Za-z_][A-Za-z0-9_]*)(?:\s*:\s*([A-Za-z_][A-Za-z0-9_]*))?$/);
    if (!m) {
      out.malformedParts.push(`'${part}'`);
      continue;
    }
    const name = m[1];
    out.names.push(name);
    if (seen.has(name)) {
      out.duplicateNames.push(name);
    } else {
      seen.add(name);
    }
    out.count += 1;
  }
  return out;
}

function splitTopLevelComma(text) {
  const parts = [];
  let current = "";
  let parenDepth = 0;
  let bracketDepth = 0;
  let braceDepth = 0;
  let inString = false;
  let escaping = false;
  let malformed = false;

  for (let i = 0; i < text.length; i += 1) {
    const ch = text[i];
    if (inString) {
      current += ch;
      if (escaping) {
        escaping = false;
      } else if (ch === "\\") {
        escaping = true;
      } else if (ch === "\"") {
        inString = false;
      }
      continue;
    }

    if (ch === "\"") {
      inString = true;
      current += ch;
      continue;
    }
    if (ch === "(") {
      parenDepth += 1;
      current += ch;
      continue;
    }
    if (ch === ")") {
      parenDepth -= 1;
      if (parenDepth < 0) malformed = true;
      current += ch;
      continue;
    }
    if (ch === "[") {
      bracketDepth += 1;
      current += ch;
      continue;
    }
    if (ch === "]") {
      bracketDepth -= 1;
      if (bracketDepth < 0) malformed = true;
      current += ch;
      continue;
    }
    if (ch === "{") {
      braceDepth += 1;
      current += ch;
      continue;
    }
    if (ch === "}") {
      braceDepth -= 1;
      if (braceDepth < 0) malformed = true;
      current += ch;
      continue;
    }

    if (ch === "," && parenDepth === 0 && bracketDepth === 0 && braceDepth === 0) {
      parts.push(current.trim());
      current = "";
      continue;
    }
    current += ch;
  }
  parts.push(current.trim());

  if (inString || parenDepth !== 0 || bracketDepth !== 0 || braceDepth !== 0) {
    malformed = true;
  }
  return { parts, malformed };
}

function analyzeCallArgs(argsText) {
  if (!argsText || !argsText.trim()) {
    return { count: 0, malformed: false, hasEmpty: false };
  }
  const split = splitTopLevelComma(argsText);
  const hasEmpty = split.parts.some((p) => !p);
  return {
    count: split.parts.filter((p) => !!p).length,
    malformed: split.malformed,
    hasEmpty
  };
}

function findMatchingParen(text, openIdx) {
  if (openIdx < 0 || openIdx >= text.length || text[openIdx] !== "(") return -1;
  let depth = 0;
  let inString = false;
  let escaping = false;
  for (let i = openIdx; i < text.length; i += 1) {
    const ch = text[i];
    if (inString) {
      if (escaping) {
        escaping = false;
      } else if (ch === "\\") {
        escaping = true;
      } else if (ch === "\"") {
        inString = false;
      }
      continue;
    }
    if (ch === "\"") {
      inString = true;
      continue;
    }
    if (ch === "(") depth += 1;
    if (ch === ")") {
      depth -= 1;
      if (depth === 0) return i;
      if (depth < 0) return -1;
    }
  }
  return -1;
}

function collectCallsInLine(codePart) {
  const out = [];
  const callRe = /([A-Za-z_][A-Za-z0-9_\.]*)\s*\(/g;
  let m;
  while ((m = callRe.exec(codePart)) !== null) {
    const name = m[1];
    const nameStart = m.index;
    const nameEnd = nameStart + name.length;
    const openIdx = codePart.indexOf("(", nameEnd);
    if (openIdx < 0) continue;
    const closeIdx = findMatchingParen(codePart, openIdx);
    if (closeIdx < 0) {
      out.push({
        name,
        nameStart,
        nameEnd,
        openIdx,
        closeIdx: -1,
        malformed: true,
        argInfo: { count: 0, malformed: true, hasEmpty: false }
      });
      continue;
    }
    const argsText = codePart.slice(openIdx + 1, closeIdx);
    out.push({
      name,
      nameStart,
      nameEnd,
      openIdx,
      closeIdx,
      malformed: false,
      argInfo: analyzeCallArgs(argsText)
    });
  }
  return out;
}

function collectDeclaredFunctionArities(lines) {
  const arities = new Map();
  for (let i = 0; i < lines.length; i += 1) {
    const raw = lines[i];
    const masked = maskCodePreserveLength(raw);
    const codeEnd = findCodeEnd(masked);
    const rawCodePart = raw.slice(0, codeEnd);
    const fnCandidate = normalizeFunctionDeclPrefix(rawCodePart);
    const fnMatch = fnCandidate.match(FUNCTION_DECL_RE);
    if (fnMatch && shouldTreatAsFunctionDeclaration(fnMatch)) {
      const name = fnMatch[2];
      const paramInfo = analyzeParameterList(fnMatch[3]);
      arities.set(name, { min: paramInfo.count, max: paramInfo.count, line: i });
    }
    const flagMatch = rawCodePart.match(FLAG_DECL_RE);
    if (flagMatch && /^[A-Za-z_][A-Za-z0-9_-]*$/.test(flagMatch[1])) {
      const paramInfo = analyzeParameterList(flagMatch[2]);
      arities.set(flagMatch[1], { min: paramInfo.count, max: paramInfo.count, line: i });
    }
  }
  return arities;
}

function collectHeuristicDiagnostics(document, settings) {
  const opts = Object.assign(
    {
      hintsEnabled: true,
      maxHintsPerFile: 120,
      styleHintsEnabled: false
    },
    settings || {}
  );

  const diagnostics = [];
  const lines = document.getText().split(/\r?\n/);
  const uri = typeof document.uri === "string" ? document.uri.toLowerCase() : "";
  const isLsFile = uri.endsWith(".ls");
  const styleHintsEnabled = opts.styleHintsEnabled === true && !isLsFile;
  const maxHints = clampMaxHints(opts.maxHintsPerFile);
  let hintCount = 0;

  const addDiagnostic = (diag) => {
    const isHint = HINT_SEVERITIES.has(diag.severity);
    if (isHint && opts.hintsEnabled === false) return;
    if (isHint) {
      if (hintCount >= maxHints) return;
      hintCount += 1;
    }
    diagnostics.push(diag);
  };

  const scopeStack = [createScope("global")];
  const knownFunctionArities = new Map(BUILTIN_ARITY);
  const declaredFunctionArities = collectDeclaredFunctionArities(lines);
  for (const [name, arity] of declaredFunctionArities.entries()) {
    knownFunctionArities.set(name, arity);
  }

  for (let i = 0; i < lines.length; i += 1) {
    const raw = lines[i];
    const masked = maskCodePreserveLength(raw);
    const codeEnd = findCodeEnd(masked);
    const codePart = masked.slice(0, codeEnd);
    const rawCodePart = raw.slice(0, codeEnd);
    const trimmed = codePart.trim();
    if (!trimmed) continue;
    let statementRecognized = false;
    let functionDeclInfo = null;

    const startsCloser =
      startsWithWord(codePart, "end") ||
      startsWithWord(codePart, "elif") ||
      startsWithWord(codePart, "else") ||
      /^\s*}/.test(codePart);

    if (startsCloser) {
      statementRecognized = true;
      if (scopeStack.length > 1) {
        scopeStack.pop();
      } else if (startsWithWord(codePart, "end") || /^\s*}/.test(codePart)) {
        const endCol = Math.max(0, codePart.indexOf("end"));
        addDiagnostic(
          makeDiagnostic(
            i,
            endCol,
            endCol + 3,
            DiagnosticSeverity.Error,
            "ls-unexpected-end",
            "Unexpected 'end': there is no open block to close here."
          )
        );
      }
    }

    const scope = scopeStack[scopeStack.length - 1];
    if (scope.terminated && !startsCloser) {
      const firstCode = Math.max(0, codePart.search(/\S/));
      addDiagnostic(
        makeDiagnostic(
          i,
          firstCode,
          firstCode + 1,
          DiagnosticSeverity.Warning,
          "ls-unreachable-statement",
          "This statement is unreachable because control flow already exits this block."
        )
      );
    }

    const pendingDeclsForNextScope = [];
    const declaredThisLine = new Set();
    let undeclaredAssignedName = "";
    let undeclaredAssignedStart = -1;

    const eqIdx = findSingleEqualsInCondition(codePart);
    if (eqIdx >= 0) {
      addDiagnostic(
        makeDiagnostic(
          i,
          eqIdx,
          eqIdx + 1,
          DiagnosticSeverity.Warning,
          "ls-assign-in-condition",
          "Are you sure this is correct? '=' in a condition usually means assignment. Did you mean '=='?",
          { title: "Use '==' for comparison", newText: "==" }
        )
      );
    }

    const printInIdx = codePart.indexOf("printIn(");
    if (printInIdx >= 0) {
      addDiagnostic(
        makeDiagnostic(
          i,
          printInIdx,
          printInIdx + "printIn".length,
          DiagnosticSeverity.Warning,
          "ls-printin-typo",
          "Looks like a typo. Did you mean 'println(...)'?",
          { title: "Replace with 'println'", newText: "println" }
        )
      );
    }

    const elseIfMatch = codePart.match(/^\s*else\s+if\b/);
    if (styleHintsEnabled && elseIfMatch && typeof elseIfMatch.index === "number") {
      const start = elseIfMatch.index + elseIfMatch[0].indexOf("else");
      addDiagnostic(
        makeDiagnostic(
          i,
          start,
          start + "else if".length,
          DiagnosticSeverity.Hint,
          "ls-else-if-style",
          "Style hint: use 'elif' in LineScript for cleaner branching.",
          { title: "Replace 'else if' with 'elif'", newText: "elif" }
        )
      );
    }

    const tripleDotIdx = codePart.indexOf("...");
    if (tripleDotIdx >= 0) {
      addDiagnostic(
        makeDiagnostic(
          i,
          tripleDotIdx,
          tripleDotIdx + 3,
          DiagnosticSeverity.Warning,
          "ls-triple-dot-range",
          "Range syntax uses '..' in LineScript. Did you mean '..' instead of '...'? ",
          { title: "Use '..' range operator", newText: ".." }
        )
      );
    }

    const openParens = (codePart.match(/\(/g) || []).length;
    const closeParens = (codePart.match(/\)/g) || []).length;
    if (openParens !== closeParens) {
      const badCol = openParens > closeParens ? codePart.lastIndexOf("(") : codePart.lastIndexOf(")");
      addDiagnostic(
        makeDiagnostic(
          i,
          Math.max(0, badCol),
          Math.max(0, badCol) + 1,
          DiagnosticSeverity.Warning,
          "ls-unbalanced-parens",
          "Unbalanced parentheses on this line. Check for a missing '(' or ')'."
        )
      );
    }

    const zeroDiv = codePart.match(/[\/%]\s*0(?:\b|[^0-9.])/);
    if (zeroDiv && typeof zeroDiv.index === "number") {
      addDiagnostic(
        makeDiagnostic(
          i,
          zeroDiv.index,
          zeroDiv.index + zeroDiv[0].length,
          DiagnosticSeverity.Error,
          "ls-divide-by-zero-literal",
          "This expression divides or modulo by literal 0, which is invalid at runtime."
        )
      );
    }

    const dotBuiltinOnly = rawCodePart.match(/^\s*(\.[A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*$/);
    if (dotBuiltinOnly) {
      statementRecognized = true;
      const dotName = dotBuiltinOnly[1];
      const argsInfo = analyzeCallArgs(dotBuiltinOnly[2]);
      const arity = DOT_BUILTINS.get(dotName);
      if (!arity) {
        addDiagnostic(
          makeDiagnostic(
            i,
            codePart.indexOf(dotName),
            codePart.indexOf(dotName) + dotName.length,
            DiagnosticSeverity.Warning,
            "ls-unknown-dot-call",
            `Unknown special call '${dotName}()'.`
          )
        );
      } else if (argsInfo.count < arity.min || argsInfo.count > arity.max) {
        addDiagnostic(
          makeDiagnostic(
            i,
            codePart.indexOf(dotName),
            codePart.indexOf(dotName) + dotName.length,
            DiagnosticSeverity.Warning,
            "ls-dot-call-arity",
            `'${dotName}()' takes ${arity.min} argument(s), but ${argsInfo.count} were provided.`
          )
        );
      }
    }

    const blockHead = codePart.match(/^\s*(if|elif|else|unless|while|for)\b/);
    const startsElseIf = /^\s*else\s+if\b/.test(codePart);
    const hasDo = /\bdo\b/.test(codePart);
    const hasBrace = /{\s*(?:$|\/\/)/.test(codePart);
    const endsWithColon = /:\s*$/.test(codePart);
    if (blockHead) statementRecognized = true;

    if (blockHead && !startsElseIf && endsWithColon) {
      const colonPos = codeEnd - 1;
      addDiagnostic(
        makeDiagnostic(
          i,
          colonPos,
          colonPos + 1,
          DiagnosticSeverity.Warning,
          "ls-python-colon-block",
          "LineScript blocks use 'do ... end', not ':'.",
          { title: "Replace ':' with ' do'", newText: " do" }
        )
      );
    } else if (blockHead && !startsElseIf && !hasDo && !hasBrace) {
      addDiagnostic(
        makeDiagnostic(
          i,
          codeEnd,
          codeEnd,
          DiagnosticSeverity.Warning,
          "ls-missing-do",
          "This block looks incomplete. Add 'do' to start the block.",
          { title: "Insert 'do'", newText: " do" }
        )
      );
    }

    const forZeroStep = codePart.match(/^\s*(?:parallel\s+)?for\s+[A-Za-z_][A-Za-z0-9_]*\s+in\b.*\bstep\s+([+\-]?\d+(?:\.\d+)?)\b/);
    if (forZeroStep) {
      const stepVal = Number(forZeroStep[1]);
      if (Number.isFinite(stepVal) && stepVal === 0) {
        const stepCol = codePart.indexOf("step");
        addDiagnostic(
          makeDiagnostic(
            i,
            stepCol,
            stepCol + 4,
            DiagnosticSeverity.Error,
            "ls-zero-step",
            "Loop step cannot be 0; this would cause a non-progressing loop."
          )
        );
      }
    }

    if (/^\s*input(?:_i64|_f64)?\s*\(/.test(codePart) && codePart.indexOf("=") === -1) {
      const word = codePart.match(/input(?:_i64|_f64)?/);
      if (word && typeof word.index === "number") {
        addDiagnostic(
          makeDiagnostic(
            i,
            word.index,
            word.index + word[0].length,
            DiagnosticSeverity.Hint,
            "ls-input-ignored",
            "Input value is ignored here. If intentional, ignore this hint; otherwise assign it to a variable."
          )
        );
      }
    }

    const trueCmp = codePart.match(/==\s*true\b/);
    if (styleHintsEnabled && trueCmp && typeof trueCmp.index === "number") {
      addDiagnostic(
        makeDiagnostic(
          i,
          trueCmp.index,
          trueCmp.index + trueCmp[0].length,
          DiagnosticSeverity.Hint,
          "ls-compare-true",
          "You can simplify this condition by removing '== true'.",
          { title: "Remove '== true'", newText: "" }
        )
      );
    }

    const falseCmp = codePart.match(/==\s*false\b/);
    if (styleHintsEnabled && falseCmp && typeof falseCmp.index === "number") {
      addDiagnostic(
        makeDiagnostic(
          i,
          falseCmp.index,
          falseCmp.index + falseCmp[0].length,
          DiagnosticSeverity.Hint,
          "ls-compare-false",
          "You can simplify this with 'not <expr>' rather than '== false'."
        )
      );
    }

    const shorthand = codePart.match(/^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*\1\s*([+\-*/%])\s*(.+)$/);
    if (styleHintsEnabled && shorthand) {
      const lhs = shorthand[1];
      const op = shorthand[2];
      const rhs = shorthand[3].trim();
      const lineStart = codePart.search(/\S/);
      const replacement = `${lhs} ${op}= ${rhs}`;
      const special =
        (op === "+" && rhs === "1") ? " You can also use '++'." :
          (op === "-" && rhs === "1") ? " You can also use '--'." : "";
      addDiagnostic(
        makeDiagnostic(
          i,
          lineStart,
          codeEnd,
          DiagnosticSeverity.Hint,
          "ls-compound-assignment",
          `QoL hint: prefer compound assignment ('${op}=') for readability.${special}`,
          { title: `Use '${op}=' shorthand`, newText: replacement }
        )
      );
    }

    if (styleHintsEnabled && /;\s*$/.test(codePart)) {
      const semiPos = codePart.lastIndexOf(";");
      addDiagnostic(
        makeDiagnostic(
          i,
          semiPos,
          semiPos + 1,
          DiagnosticSeverity.Hint,
          "ls-unneeded-semicolon",
          "Semicolons are optional in LineScript.",
          { title: "Remove trailing ';'", newText: "" }
        )
      );
    }

    if (styleHintsEnabled && /^\s*(if|while|for|unless)\s*\(/.test(codePart)) {
      const m = codePart.match(/^\s*(if|while|for|unless)\s*\(/);
      if (m && typeof m.index === "number") {
        addDiagnostic(
          makeDiagnostic(
            i,
            m.index,
            m.index + m[1].length,
            DiagnosticSeverity.Information,
            "ls-c-style-block",
            "Friendly reminder: canonical LineScript blocks use 'if <cond> do ... end'."
          )
        );
      }
    }

    const classMatch = codePart.match(/^\s*class\s+([A-Za-z_][A-Za-z0-9_]*)\b/);
    if (classMatch) {
      statementRecognized = true;
      const name = classMatch[1];
      const col = codePart.indexOf(name);
      const ok = declareSymbol(scope, name, { isConst: true, kind: "class", line: i });
      if (!ok) {
        addDiagnostic(
          makeDiagnostic(
            i,
            col,
            col + name.length,
            DiagnosticSeverity.Warning,
            "ls-duplicate-declare",
            `Duplicate declaration of '${name}' in the same scope.`
          )
        );
      }
      declaredThisLine.add(name);
    }

    const fnCandidate = normalizeFunctionDeclPrefix(rawCodePart);
    const fnMatch = fnCandidate.match(FUNCTION_DECL_RE);
    const isFunctionDecl = shouldTreatAsFunctionDeclaration(fnMatch);
    if (fnMatch && isFunctionDecl) {
      statementRecognized = true;
      const name = fnMatch[2];
      const col = codePart.indexOf(name);
      const paramsInfo = analyzeParameterList(fnMatch[3]);
      functionDeclInfo = {
        name,
        start: col,
        end: col + name.length
      };
      const allowKeywordFunctionName = name === "constructor";
      if (!KEYWORDS.has(name) || allowKeywordFunctionName) {
        const ok = declareSymbol(scope, name, {
          isConst: true,
          kind: "function",
          line: i,
          arity: { min: paramsInfo.count, max: paramsInfo.count }
        });
        if (!ok) {
          addDiagnostic(
            makeDiagnostic(
              i,
              col,
              col + name.length,
              DiagnosticSeverity.Warning,
              "ls-duplicate-declare",
              `Duplicate declaration of '${name}' in the same scope.`
            )
          );
        }
        if (STRICT_RESERVED_FUNCTION_NAMES.has(name)) {
          addDiagnostic(
            makeDiagnostic(
              i,
              col,
              col + name.length,
              DiagnosticSeverity.Error,
              "ls-reserved-builtin-name",
              `Function name '${name}' collides with a built-in API name. Rename it to avoid hard-to-debug behavior.`
            )
          );
          const builtinArity = BUILTIN_ARITY.get(name);
          if (builtinArity && (paramsInfo.count < builtinArity.min || paramsInfo.count > builtinArity.max)) {
            addDiagnostic(
              makeDiagnostic(
                i,
                col,
                col + name.length,
                DiagnosticSeverity.Error,
                "ls-builtin-signature-mismatch",
                `Built-in '${name}' expects ${builtinArity.min}` +
                (builtinArity.min === builtinArity.max ? "" : `-${builtinArity.max}`) +
                ` argument(s), but this declaration has ${paramsInfo.count}.`
              )
            );
          }
        }
        if (paramsInfo.malformedParts.length > 0) {
          addDiagnostic(
            makeDiagnostic(
              i,
              col,
              col + name.length,
              DiagnosticSeverity.Warning,
              "ls-malformed-params",
              `Malformed parameter list in function '${name}'. Check commas, names, and optional ': type' annotations.`
            )
          );
        }
        if (paramsInfo.duplicateNames.length > 0) {
          addDiagnostic(
            makeDiagnostic(
              i,
              col,
              col + name.length,
              DiagnosticSeverity.Warning,
              "ls-duplicate-param",
              `Duplicate parameter name(s): ${Array.from(new Set(paramsInfo.duplicateNames)).join(", ")}.`
            )
          );
        }
        declaredThisLine.add(name);
        for (const p of paramsInfo.names) {
          declaredThisLine.add(p);
        }
        if (fnMatch[5]) {
          for (const p of paramsInfo.names) {
            pendingDeclsForNextScope.push({ name: p, isConst: false, kind: "param", line: i });
          }
        }
      } else {
        for (const p of paramsInfo.names) {
          declaredThisLine.add(p);
        }
        if (fnMatch[5]) {
          for (const p of paramsInfo.names) {
            pendingDeclsForNextScope.push({ name: p, isConst: false, kind: "param", line: i });
          }
        }
      }
    } else {
      const looksLikeFnSig = fnCandidate.match(/^\s*(?:(inline|extern|fn|func)\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*\(/);
      if (looksLikeFnSig && (/\bdo\b/.test(codePart) || /{\s*(?:$|\/\/)/.test(codePart) || /\bfn\b|\bfunc\b|\binline\b|\bextern\b/.test(codePart))) {
        const name = looksLikeFnSig[2];
        if (KEYWORDS.has(name)) {
          // Block heads like `if (...) do` are not function signatures.
        } else {
          const explicitDeclKeyword = /^\s*(?:inline|extern|fn|func)\b/.test(fnCandidate);
          const openIdx = fnCandidate.indexOf("(", fnCandidate.indexOf(name));
          const closeIdx = openIdx >= 0 ? findMatchingParen(fnCandidate, openIdx) : -1;
          let paramsMalformed = true;
          if (openIdx >= 0 && closeIdx > openIdx) {
            const p = analyzeParameterList(fnCandidate.slice(openIdx + 1, closeIdx));
            paramsMalformed = p.malformedParts.length > 0;
          }
          if (!explicitDeclKeyword && paramsMalformed) {
            // Likely a normal call expression with a block argument (e.g. formatOutput("x") do).
          } else {
            const col = codePart.indexOf(name);
            addDiagnostic(
              makeDiagnostic(
                i,
                col,
                col + name.length,
                DiagnosticSeverity.Warning,
                "ls-malformed-function-signature",
                `Function signature for '${name}' looks malformed. Verify parentheses, parameters, return type, and block opener.`
              )
            );
          }
        }
      }
    }

    const flagMatch = rawCodePart.match(FLAG_DECL_RE);
    if (flagMatch) {
      statementRecognized = true;
      const name = flagMatch[1];
      const nameCol = codePart.indexOf(name);
      functionDeclInfo = {
        name,
        start: nameCol,
        end: nameCol + name.length
      };
      const paramsInfo = analyzeParameterList(flagMatch[2]);
      if (/^[A-Za-z_][A-Za-z0-9_-]*$/.test(name)) {
        const col = codePart.indexOf(name);
        const ok = declareSymbol(scope, name, {
          isConst: true,
          kind: "function",
          line: i,
          arity: { min: paramsInfo.count, max: paramsInfo.count }
        });
        if (!ok) {
          addDiagnostic(
            makeDiagnostic(
              i,
              col,
              col + name.length,
              DiagnosticSeverity.Warning,
              "ls-duplicate-declare",
              `Duplicate declaration of '${name}' in the same scope.`
            )
          );
        }
        declaredThisLine.add(name);
        for (const p of paramsInfo.names) {
          declaredThisLine.add(p);
        }
        if (paramsInfo.malformedParts.length > 0) {
          addDiagnostic(
            makeDiagnostic(
              i,
              col,
              col + name.length,
              DiagnosticSeverity.Warning,
              "ls-malformed-params",
              `Malformed parameter list in flag '${name}'.`
            )
          );
        }
        if (paramsInfo.duplicateNames.length > 0) {
          addDiagnostic(
            makeDiagnostic(
              i,
              col,
              col + name.length,
              DiagnosticSeverity.Warning,
              "ls-duplicate-param",
              `Duplicate parameter name(s): ${Array.from(new Set(paramsInfo.duplicateNames)).join(", ")}.`
            )
          );
        }
      }
      if (flagMatch[3]) {
        for (const p of paramsInfo.names) {
          pendingDeclsForNextScope.push({ name: p, isConst: false, kind: "param", line: i });
        }
      }
    }

    const declareMatch = codePart.match(/^\s*declare\s+(const\s+)?(owned\s+)?([A-Za-z_][A-Za-z0-9_]*)\b/);
    if (declareMatch) {
      statementRecognized = true;
      const isConst = !!declareMatch[1];
      const name = declareMatch[3];
      const col = codePart.indexOf(name);
      const ok = declareSymbol(scope, name, { isConst, kind: "var", line: i });
      if (!ok) {
        addDiagnostic(
          makeDiagnostic(
            i,
            col,
            col + name.length,
            DiagnosticSeverity.Warning,
            "ls-duplicate-declare",
            `Duplicate declaration of '${name}' in the same scope.`
          )
        );
      }
      declaredThisLine.add(name);
    } else if (startsWithWord(codePart, "declare")) {
      statementRecognized = true;
      const col = Math.max(0, codePart.indexOf("declare"));
      addDiagnostic(
        makeDiagnostic(
          i,
          col,
          col + "declare".length,
          DiagnosticSeverity.Error,
          "ls-malformed-declare",
          "Malformed declaration. Use 'declare name', 'declare name = ...', or 'declare name: type = ...'."
        )
      );
    }

    const forMatch = codePart.match(/^\s*(?:parallel\s+)?for\s+([A-Za-z_][A-Za-z0-9_]*)\s+in\b/);
    if (forMatch) {
      statementRecognized = true;
      pendingDeclsForNextScope.push({ name: forMatch[1], isConst: false, kind: "iter", line: i });
      declaredThisLine.add(forMatch[1]);
    }

    const memberAssignMatch = codePart.match(
      /^\s*(?:this|[A-Za-z_][A-Za-z0-9_]*)\.[A-Za-z_][A-Za-z0-9_]*\s*(\+\+|--|\+=|-=|\*=|\/=|%=|\^=|\*\*=|=)\s*/
    );
    if (memberAssignMatch) {
      statementRecognized = true;
    }

    const assignMatch = codePart.match(/^\s*([A-Za-z_][A-Za-z0-9_]*)\s*(\+\+|--|\+=|-=|\*=|\/=|%=|\^=|\*\*=|=)\s*/);
    if (assignMatch && !startsWithWord(codePart, "declare")) {
      statementRecognized = true;
      const name = assignMatch[1];
      const op = assignMatch[2];
      const col = codePart.indexOf(name);
      const sym = findSymbol(scopeStack, name);
      if (!sym && !isBuiltinToken(name)) {
        undeclaredAssignedName = name;
        undeclaredAssignedStart = col;
        addDiagnostic(
          makeDiagnostic(
            i,
            col,
            col + name.length,
            DiagnosticSeverity.Error,
            "ls-undeclared-assign",
            `Variable '${name}' is assigned before declaration. Use 'declare ${name}' first.`
          )
        );
      } else if (sym && sym.isConst) {
        addDiagnostic(
          makeDiagnostic(
            i,
            col,
            col + name.length,
            DiagnosticSeverity.Error,
            "ls-const-reassign",
            `Cannot modify const variable '${name}'.`
          )
        );
      } else if ((op === "++" || op === "--") && !sym) {
        addDiagnostic(
          makeDiagnostic(
            i,
            col,
            col + name.length,
            DiagnosticSeverity.Error,
            "ls-undeclared-var",
            `Variable '${name}' is not declared in this scope.`
          )
        );
      }
    }

    if (startsWithWord(codePart, "break")) {
      statementRecognized = true;
      if (!isLoopScopeActive(scopeStack)) {
        addDiagnostic(
          makeDiagnostic(
            i,
            codePart.indexOf("break"),
            codePart.indexOf("break") + 5,
            DiagnosticSeverity.Error,
            "ls-break-outside-loop",
            "'break' can only be used inside loops."
          )
        );
      } else {
        scope.terminated = true;
      }
    }
    if (startsWithWord(codePart, "continue")) {
      statementRecognized = true;
      if (!isLoopScopeActive(scopeStack)) {
        addDiagnostic(
          makeDiagnostic(
            i,
            codePart.indexOf("continue"),
            codePart.indexOf("continue") + 8,
            DiagnosticSeverity.Error,
            "ls-continue-outside-loop",
            "'continue' can only be used inside loops."
          )
        );
      } else {
        scope.terminated = true;
      }
    }
    if (startsWithWord(codePart, "return")) {
      statementRecognized = true;
      scope.terminated = true;
    }

    const skipCallAnalysis = (fnMatch && isFunctionDecl) || !!flagMatch;
    const callSites = skipCallAnalysis ? [] : collectCallsInLine(rawCodePart);
    for (const call of callSites) {
      if (call.nameStart > 0 && rawCodePart[call.nameStart - 1] === ".") {
        continue;
      }
      if (functionDeclInfo && call.name === functionDeclInfo.name && call.nameStart === functionDeclInfo.start) {
        continue;
      }
      if (KEYWORDS.has(call.name)) continue;

      const localSym = findSymbol(scopeStack, call.name);
      const builtinArity = BUILTIN_ARITY.get(call.name) || DOT_BUILTINS.get(call.name);
      const knownArity = builtinArity || knownFunctionArities.get(call.name);

      statementRecognized = true;

      if (call.malformed) {
        addDiagnostic(
          makeDiagnostic(
            i,
            call.nameStart,
            call.nameEnd,
            DiagnosticSeverity.Warning,
            "ls-malformed-call",
            `Call to '${call.name}' appears malformed (unbalanced parentheses).`
          )
        );
        continue;
      }

      if (call.argInfo.malformed || call.argInfo.hasEmpty) {
        addDiagnostic(
          makeDiagnostic(
            i,
            call.nameStart,
            call.nameEnd,
            DiagnosticSeverity.Warning,
            "ls-malformed-call-args",
            `Arguments for '${call.name}(...)' look malformed. Check commas and nested expressions.`
          )
        );
      }

      if (!knownArity && !localSym && !call.name.includes(".") && !isBuiltinToken(call.name)) {
        addDiagnostic(
          makeDiagnostic(
            i,
            call.nameStart,
            call.nameEnd,
            DiagnosticSeverity.Warning,
            "ls-unknown-function",
            `'${call.name}(...)' is not a known function in this file or built-in API.`
          )
        );
        continue;
      }

      if (knownArity && (call.argInfo.count < knownArity.min || call.argInfo.count > knownArity.max)) {
        addDiagnostic(
          makeDiagnostic(
            i,
            call.nameStart,
            call.nameEnd,
            DiagnosticSeverity.Warning,
            "ls-function-arity",
            `'${call.name}(...)' expects ${knownArity.min}` +
            (knownArity.min === knownArity.max ? "" : `-${knownArity.max}`) +
            ` argument(s), but ${call.argInfo.count} were supplied.`
          )
        );
      }
    }

    const skipUndeclaredTokenScan = (fnMatch && isFunctionDecl) || !!flagMatch;
    const seenUndeclared = new Set();
    const tokenRe = /[A-Za-z_][A-Za-z0-9_]*/g;
    let m;
    while (!skipUndeclaredTokenScan && (m = tokenRe.exec(codePart)) !== null) {
      const token = m[0];
      const start = m.index;
      const end = start + token.length;
      if (declaredThisLine.has(token)) continue;
      if (KEYWORDS.has(token) || TYPE_NAMES.has(token)) continue;
      if (isBuiltinToken(token)) continue;

      const prevCh = start > 0 ? codePart[start - 1] : "";
      const nextCh = end < codePart.length ? codePart[end] : "";
      if (prevCh === "." || nextCh === ".") continue;

      const prevIdx = prevNonWsIndex(codePart, start - 1);
      if (prevIdx >= 0 && codePart[prevIdx] === ":") continue;

      const nextNonWs = nextNonWsChar(codePart, end);
      if (nextNonWs === "(") continue;

      const before = codePart.slice(0, start);
      if (/(->|throws)\s*$/.test(before)) continue;

      const sym = findSymbol(scopeStack, token);
      if (!sym) {
        if (token === undeclaredAssignedName && start === undeclaredAssignedStart) {
          continue;
        }
        const k = token + "@" + start;
        if (!seenUndeclared.has(k)) {
          seenUndeclared.add(k);
          addDiagnostic(
            makeDiagnostic(
              i,
              start,
              end,
              DiagnosticSeverity.Error,
              "ls-undeclared-var",
              `Variable '${token}' is used before declaration in this scope.`
            )
          );
        }
      }
    }

    if (!statementRecognized && !startsCloser && !/^\s*\{\s*$/.test(codePart)) {
      const firstCode = Math.max(0, codePart.search(/\S/));
      addDiagnostic(
        makeDiagnostic(
          i,
          firstCode,
          firstCode + 1,
          DiagnosticSeverity.Warning,
          "ls-unknown-statement",
          "This statement does not match known LineScript syntax. Check for typos or malformed code."
        )
      );
    }

    const opensBlock = /\bdo\b/.test(codePart) || /{\s*(?:$|\/\/)/.test(codePart);
    if (opensBlock) {
      const isLoop = /^\s*(?:parallel\s+)?for\b/.test(codePart) || /^\s*while\b/.test(codePart);
      const newScope = createScope(isLoop ? "loop" : "block");
      for (const d of pendingDeclsForNextScope) {
        declareSymbol(newScope, d.name, { isConst: d.isConst, kind: d.kind, line: d.line });
      }
      scopeStack.push(newScope);
    }
  }

  if (scopeStack.length > 1) {
    const lastLine = Math.max(0, lines.length - 1);
    addDiagnostic(
      makeDiagnostic(
        lastLine,
        lines[lastLine] ? lines[lastLine].length : 0,
        lines[lastLine] ? lines[lastLine].length : 0,
        DiagnosticSeverity.Warning,
        "ls-missing-end",
        "One or more blocks appear to be missing an 'end'."
      )
    );
  }

  return diagnostics;
}

module.exports = {
  collectHeuristicDiagnostics,
  maskCodePreserveLength
};
