"use strict";

const assert = require("assert");
const { FULL_BUILTIN_NAMES } = require("./builtin_catalog");

const required = [
  "substring",
  "repeat",
  "reverse",
  "ord",
  "chr",
  "bytes_len",
  "array_set",
  "dict_get",
  "np_sum",
  "game_scroll_y",
  "game_set_window_mode",
  "game_interpolated_delta",
  "pg_set_windowed_fullscreen",
  "renderer_select_accelerated",
  "renderer_set_hardware_acceleration",
  "task_hardware_threads",
  "task_set_hyperthreading",
  "su.capabilities"
];

for (const name of required) {
  assert.ok(
    FULL_BUILTIN_NAMES.includes(name),
    `completion catalog missing '${name}'`
  );
}

assert.ok(
  FULL_BUILTIN_NAMES.length >= 200,
  "completion catalog unexpectedly small; expected full builtin coverage"
);

console.log("completion_catalog.test.js: completion catalog coverage checks passed");
