"use strict";

function normalizeNewlines(text) {
  return String(text || "").replace(/\r\n/g, "\n").replace(/\r/g, "\n");
}

function stripStringsAndComments(line) {
  let out = "";
  let inString = false;
  let escaping = false;

  for (let i = 0; i < line.length; i += 1) {
    const ch = line[i];
    const next = i + 1 < line.length ? line[i + 1] : "";

    if (!inString && ch === "/" && next === "/") {
      break;
    }

    if (inString) {
      if (escaping) {
        escaping = false;
      } else if (ch === "\\") {
        escaping = true;
      } else if (ch === "\"") {
        inString = false;
      }
      out += " ";
      continue;
    }

    if (ch === "\"") {
      inString = true;
      out += " ";
      continue;
    }

    out += ch;
  }

  return out;
}

function startsWithWord(text, word) {
  const re = new RegExp("^" + word + "\\b");
  return re.test(text);
}

function hasLeadingCloser(maskedTrimmed) {
  return (
    maskedTrimmed === "}" ||
    startsWithWord(maskedTrimmed, "end") ||
    startsWithWord(maskedTrimmed, "elif") ||
    startsWithWord(maskedTrimmed, "else")
  );
}

function opensBlock(maskedTrimmed) {
  if (!maskedTrimmed) return false;
  if (maskedTrimmed.endsWith("{")) return true;
  return /\bdo\b/.test(maskedTrimmed);
}

function buildIndent(depth, tabSize, insertSpaces) {
  if (depth <= 0) return "";
  if (!insertSpaces) return "\t".repeat(depth);
  return " ".repeat(depth * tabSize);
}

function formatLineScript(text, options) {
  const normalized = normalizeNewlines(text);
  const hadTrailingNewline = normalized.endsWith("\n");
  const lines = normalized.split("\n");
  if (hadTrailingNewline && lines.length > 0 && lines[lines.length - 1] === "") {
    lines.pop();
  }
  const tabSize = Number(options && options.tabSize) > 0 ? Number(options.tabSize) : 2;
  const insertSpaces = options && typeof options.insertSpaces === "boolean" ? options.insertSpaces : true;

  let depth = 0;
  const out = [];

  for (const rawLine of lines) {
    const trimmed = rawLine.trim();
    if (!trimmed) {
      out.push("");
      continue;
    }

    const masked = stripStringsAndComments(trimmed);
    const maskedTrimmed = masked.trim();
    const leadingCloser = hasLeadingCloser(maskedTrimmed);

    let lineDepth = depth;
    if (leadingCloser) {
      lineDepth = Math.max(0, lineDepth - 1);
    }

    out.push(buildIndent(lineDepth, tabSize, insertSpaces) + trimmed);

    const opened = opensBlock(maskedTrimmed) ? 1 : 0;
    depth = lineDepth + opened;
  }

  let result = out.join("\n");
  if (hadTrailingNewline) {
    result += "\n";
  }
  return result;
}

module.exports = {
  formatLineScript,
  stripStringsAndComments
};
