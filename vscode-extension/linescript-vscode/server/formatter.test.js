"use strict";

const assert = require("assert");
const { formatLineScript } = require("./formatter");

function check(name, input, expected, options) {
  const actual = formatLineScript(input, options || { tabSize: 2, insertSpaces: true });
  assert.strictEqual(actual, expected, name);
}

check(
  "if/else/end indentation",
  "main() -> i64 do\nif x == 1 do\nprintln(\"a\")\nelse do\nprintln(\"b\")\nend\nreturn 0\nend\n",
  "main() -> i64 do\n  if x == 1 do\n    println(\"a\")\n  else do\n    println(\"b\")\n  end\n  return 0\nend\n"
);

check(
  "top-level statements stay top-level",
  "print(\"milk\")\ndeclare x = 1\nif x == 1 do\nprint(x)\nend\n",
  "print(\"milk\")\ndeclare x = 1\nif x == 1 do\n  print(x)\nend\n"
);

check(
  "brace blocks",
  "sudo() {\nprint(\"hi\")\nif true {\nprint(\"x\")\n}\n}\n",
  "sudo() {\n  print(\"hi\")\n  if true {\n    print(\"x\")\n  }\n}\n"
);

check(
  "strings and comments do not affect block depth",
  "main() -> i64 do\nprint(\"do not indent end\") // do end\nprintln(\"ok\")\nend\n",
  "main() -> i64 do\n  print(\"do not indent end\") // do end\n  println(\"ok\")\nend\n"
);

console.log("formatter.test.js: all tests passed");

