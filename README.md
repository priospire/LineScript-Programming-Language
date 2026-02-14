# LineScript

LineScript is a speed-first compiled language with high-level syntax.

Design goals:
- `#1` runtime speed
- `#2` power, modularity, and clean syntax

LineScript supports two native backends:
- custom x86 ASM backend (`--backend asm`, or `--backend auto` to prefer ASM first)
- optimized C backend (`--backend c`)

Recent change history: `docs/CHANGELOG.md`.

Documentation map:
- syntax: `docs/SYNTAX.md`
- OOP (classes/inheritance/modifiers): `docs/OOP.md`
- standard library: `docs/STDLIB.md`
- language guide: `docs/LANGUAGE_GUIDE.md`
- complete API reference: `docs/API_REFERENCE.md`
- IDE setup (including VSCode extension): `docs/IDE_SETUP.md`
- ready configs for other IDEs/editors: `editor-configs/README.md`

VSCode extension source:
- `vscode-extension/linescript-vscode`
- compiler-authenticated diagnostics by default (`lsc --check`), plus optional heuristic hints/suggestions.
- includes quick fixes, snippets, and `.lsc/.ls` icon theme support.
- one-time install scripts: `scripts/install_vscode_extension.ps1` and `scripts/install_vscode_extension.sh`

## Quick Start

Build and run a program in one command:

```powershell
.\linescript.cmd examples\benchmark.lsc -O4 --cc clang
```

Linux/macOS:

```bash
./linescript.sh examples/benchmark.lsc -O4 --cc clang
```

Release rule:
- always use `-O4` for real performance runs and distributable binaries.
- `--max-speed` remains supported as a compatibility alias.
- plain builds are primarily for debugging and compile validation.

Compile multiple modules together:

```powershell
.\linescript.cmd examples\module_math.lsc examples\module_main.lsc -O4 --cc clang
```

Linux notes:
- LineScript compiler/runtime is supported on Linux with `clang` or `gcc`.
- spawn/await threading links with `-pthread` automatically when needed.
- built-in HTTP server/client works on Linux.
- game/window key polling is currently native on Windows; on Linux game APIs run in safe headless mode.
- use `linescript.sh` and `*.sh` test/package scripts (no PowerShell dependency required).

## Install and Project Setup

### Method 1 (your workflow): copy from `dist` into your project root

1. Put the `dist` folder inside your workspace/project folder.
2. Open `dist`, then open the version folder inside it (example: `LineScript-win64-20260214`).
3. Copy everything inside that version folder.
4. Paste those files/folders into your project root (outside `dist`).
5. Delete `dist` when done.
6. Create your program file (example: `main.lsc`) in project root.
7. Run:

```powershell
.\linescript.cmd .\main.lsc -O4 --cc clang
```

If you prefer direct compiler usage:

```powershell
.\lsc.exe .\main.lsc --run -O4 --cc clang -o .\main.exe
```

### Method 2: unzip release directly

1. Extract `LineScript-win64-YYYYMMDD.zip` into your project root.
2. Make sure `lsc.exe` and `linescript.cmd` are in the same folder as your `.lsc` file.
3. Run `.\linescript.cmd .\main.lsc -O4 --cc clang`.

### Method 3: build from source

1. Build compiler:

```powershell
clang++ -std=c++20 -O3 -Wall -Wextra -pedantic src/lsc.cpp -o lsc.exe
```

2. Run your file:

```powershell
.\lsc.exe .\main.lsc --run -O4 --cc clang -o .\main.exe
```

### First file quick check

Create `main.lsc`:

```linescript
print("hello from LineScript")
```

Run it:

```powershell
.\linescript.cmd .\main.lsc -O4 --cc clang
```

## Interactive Shell (REPL)

Python-style interactive shell is built into `lsc`:

```powershell
.\lsc.exe
```

or:

```powershell
.\lsc.exe --repl
```

You can also run:

```powershell
.\linescript.cmd
```

REPL commands:
- `:help` show help
- `:reset` clear session state
- `:whoami` show current shell identity
- `su.verbosity.1` ... `su.verbosity.5` set superuser verbosity level (superuser mode only)
- `su.help` show superuser-only shell commands and privileged APIs (requires superuser mode)
- `:exit` / `:quit` leave shell

Prompt behavior:
- default prompt is `<username>@LineScript>`.
- after successful `superuser()`, prompt switches to `superuser@LineScript>`.
- `superuser()` is a REPL toggle: run once to enable, run again to disable.
- verbosity profile: level 1 is concise stage/warning output, level 5 is full source->lexer->parser->build tracing.

Shell-only output behavior:
- in REPL mode, `print(...)` is treated as `println(...)` so output stays clean and does not merge with the prompt.
- this does **not** change file/script behavior in normal compilation; outside REPL, `print` remains non-newline.

## Core Syntax

Comments:

```linescript
// this is a comment
```

Function declarations (`func` keyword is not required):

```linescript
sum(a: i64, b: i64) -> i64 do
  return a + b
end
```

Top-level statements are also valid:

```linescript
print("milk")
```

Functions are optional, but callable when you need them:

```linescript
sudo() {
  print("hi")
}
sudo()
```

Entry resolution for `--build` / `--run`:
1. top-level statements, if present
2. otherwise `main()`
3. otherwise exactly one zero-argument function (any name)

Script-first workflow (no required `main()`):
- write normal top-level code and run it
- add functions only when you want reuse/organization
- mix top-level setup with function/class definitions as needed

Variable declarations use `declare`:

```linescript
declare x
declare y: f64
declare z = 10
declare name: str = "LineScript"
declare const limit: i64 = 1000
```

Printing:

```linescript
print(123)
println(3.14159)
println(true)
println("x")
```

Print behavior:
- `print(x)` prints the value of variable `x`.
- `print("x")` prints the literal text `x`.

Quick top-level example:

```linescript
declare sum = 0
for i in 0..5 do
  sum += i
end
println(sum)
```

OOP quick syntax:

```linescript
class Base do
  declare x: i64 = 0

  public fn constructor(seed: i64) do
    this.x = seed
  end

  public virtual fn value() -> i64 do
    return this.x
  end
end

class Derived extends Base do
  declare y: i64 = 0

  public fn constructor(a: i64, b: i64) : Base(a) do
    this.y = b
  end

  public override final fn value() -> i64 do
    return this.x + this.y
  end
end
```

For complete OOP rules and edge cases, see `docs/OOP.md`.

Input:

```linescript
declare name = input("Enter name: ")
println(name)
declare age = input_i64("Enter age: ")
declare weight = input_f64("Enter weight: ")
```

Native HTTP server + client:

```linescript
server_worker() -> void do
  declare srv = http_server_listen(18081)
  declare c = http_server_accept(srv)
  declare req = http_server_read(c)
  if contains(req, "GET /ping") do
    http_server_respond_text(c, 200, "pong")
  else do
    http_server_respond_text(c, 400, "bad")
  end
  http_client_close(c)
  http_server_close(srv)
end

main() -> i64 do
  declare t = spawn(server_worker())
  declare c = http_client_connect("127.0.0.1", 18081)
  http_client_send(c, "GET /ping HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
  println(http_client_read(c))
  http_client_close(c)
  await(t)
  return 0
end
```

Postfix increment/decrement:

```linescript
declare x = 1
x++
x--
x += 5
x -= 2
x *= 3
x /= 2
x %= 7
x ^= 2
x **= 2
```

Notes:
- `%=` is `i64`-only.
- `^=` and `**=` are power-assignment operators.

Simple concurrency:

```linescript
worker() -> void do
  println("done")
end

main() -> i64 do
  declare t = spawn(worker())
  await(t)
  await_all()
  return 0
end
```

Parallel loops:

```linescript
parallel for i in 0..1000000 do
  declare x = i * i
end
```

Formatting helper:

```linescript
println(formatOutput(42))
println(FormatOutput(3.5))
println(formatOutput("x"))
```

Deterministic owned handles (scope-exit cleanup, no GC):

```linescript
main() -> i64 do
  declare owned d = dict_new()
  dict_set(d, "name", "LineScript")
  println(dict_get(d, "name"))
  return 0
end
```

Explicit throw contracts:

```linescript
load() -> i64 throws IOError do
  return 1
end

main() -> i64 throws IOError do
  return load()
end
```

Option/Result enum-style handles:

```linescript
declare o = option_some("hello")
declare r = result_err("IOError", "missing file")
println(option_unwrap_or(o, "fallback"))
println(result_error_type(r))
option_free(o)
result_free(r)
```

LineScript also accepts a few compatibility-style CLI flags intended for quick checks and diagnostics.

Format whole output blocks with optional end suffix:

```linescript
formatOutput("\n") do
  print("value=")
  print(42)
end
```

Native graphics (built-in 2D raster, no external package):

```linescript
declare canvas = gfx_new(320, 200)
gfx_clear(canvas, 20, 20, 30)
gfx_line(canvas, 0, 0, 319, 199, 255, 0, 0)
gfx_rect(canvas, 20, 20, 80, 50, 0, 200, 255, false)
println(gfx_save_ppm(canvas, "frame.ppm"))
gfx_free(canvas)
```

Pygame-like aliases:

```linescript
declare g = pg_init(320, 180, "Demo", true)
pg_begin(g)
pg_clear(g, 18, 22, 30)
pg_draw_rect(g, 30, 30, 80, 50, 255, 170, 60, true)
pg_end(g)
pg_quit(g)
```

NumPy-like vectors:

```linescript
declare a = np_from_range(0.0, 5.0, 1.0)
declare b = np_linspace(1.0, 5.0, 5)
declare c = np_add(a, b)
println(to_i64(round(np_dot(a, b))))
np_free(a)
np_free(b)
np_free(c)
```

Native physics + camera + key polling:

```linescript
declare player = phys_new(0.0, 2.0, 0.0, 1.0, false)
camera_bind(player)
camera_set_offset(0.0, 1.8, -6.0)

if key_down_name("SPACE") do
  phys_apply_force(player, 0.0, 8.0, 0.0)
end

phys_step(0.016)
println(camera_target())
phys_free(player)
```

Native 2D game runtime (window + frame loop + deterministic headless mode):

```linescript
declare game = game_new(320, 180, "LineScript Game", true)
game_set_target_fps(game, 60)

while not game_should_close(game) do
  game_begin(game)
  declare mx = to_i64(round(game_mouse_x(game)))
  declare my = to_i64(round(game_mouse_y(game)))
  game_clear(game, 18, 22, 30)
  game_rect(game, mx - 4, my - 4, 8, 8, 120, 255, 120, true)
  game_rect(game, 30, 30, 80, 50, 255, 170, 60, true)
  game_line(game, 0, 0, 319, 179, 90, 220, 255)
  game_end(game)
end

game_free(game)
```

Input polling in game mode:
- keyboard: `key_down(code)` / `key_down_name("SPACE")`
- mouse buttons: `game_mouse_down(game, 1)` or `game_mouse_down_name(game, "LEFT")`
- wheel: `game_scroll_x(game)` / `game_scroll_y(game)` (per-frame deltas)
- pygame aliases: `pg_mouse_down*`, `pg_scroll_x/y`

Inline format marker (no closing statement):

```linescript
main() -> i64 do
  .format()
  declare name = input("Name: ")
  println(name)
  return 0
end
```

Behavior:
- `.format()` can be placed anywhere inside a function.
- `.format()` keeps runtime output visible and suppresses compiler/build status chatter.
- in `--run` mode with `.format()`, output stays clean (program output only).
- when launched from an existing terminal, input/output still use that parent terminal.

Superuser mode (advanced diagnostics):

```linescript
superuser()
su.trace.on()
println(su.capabilities())
su.trace.off()
```

Behavior:
- any `su.*` call without `superuser()` fails with `Not privileged`.
- enabling `superuser()` prints a terminal warning because checks are intentionally relaxed.
- with `.format()`, superuser diagnostics are routed to debug/error output while keeping normal program output clean.
- superuser helpers include `su.memory.inspect()`, `su.limit.set(...)`, `su.compiler.inspect()`, `su.ir.dump()`, and `su.debug.hook("tag")`.
- in superuser mode, LineScript guardrails are relaxed and downgraded to warnings where possible, then backend compiler/toolchain validation continues.

Inline console detach marker (no closing statement):

```linescript
main() -> i64 do
  .freeConsole()
  declare game = game_new(320, 180, "LineScript Game", true)
  while not game_should_close(game) do
    game_begin(game)
    game_clear(game, 18, 22, 30)
    game_end(game)
  end
  game_free(game)
  return 0
end
```

Behavior:
- `.freeConsole()` and `FreeConsole()` can be placed anywhere inside a function.
- on Windows they detach the console window.
- on non-Windows targets they are safe no-ops.

Inline speed probe inside any function (no `end` block required):

```linescript
main() -> i64 do
  declare total: i64 = 0
  for i in 0..100000 do
    total = total + i
  end
  .stateSpeed() // prints: speed_us=<elapsed-microseconds>
  return 0
end
```

## Quick Glossary

- `declare`: define a variable.
- `const`: immutable declared variable.
- `class`: C++-style class declaration with fields/methods/constructor.
- `this`: current class instance inside methods/constructor.
- `i64`: 64-bit integer.
- `i32`: 32-bit integer.
- `f64`: 64-bit floating-point number.
- `f32`: 32-bit floating-point number.
- `bool`: boolean type.
- `str`: string/text type.
- `.stateSpeed()`: prints elapsed microseconds since the current function started.
- `.format()`: enables clean output mode (suppresses compiler/build chatter, keeps program output).
- `.freeConsole()` / `FreeConsole()`: detach the console window for windowed runs on Windows.
- `input_i64()` / `input_f64()`: typed numeric input helpers.
- `array_new` / `dict_new`: dynamic collection handles (`i64`) for arrays/maps.
 - `object_new`: low-level object-handle alias over dictionary storage.
- `gfx_new`: creates a native graphics canvas handle (`i64`).
- `game_new`: creates a native 2D game/runtime handle (`i64`).
- `phys_new`: creates a native physics object handle (`i64`).
- `camera_bind` / `key_down_name`: camera and input polling primitives.

## Build and Run

Manual compiler build:

```powershell
clang++ -std=c++20 -O3 -Wall -Wextra -pedantic src/lsc.cpp -o lsc.exe
```

Manual compile + run:

```powershell
.\lsc.exe examples\high_level.lsc --run --cc clang -O4 -o examples\high_level.exe
```

## CLI

```text
lsc <file1.lsc> [file2.lsc ...] [options]
```

Options:
- `--check` parse/type-check/optimize only (no code generation)
- `--build` compile to native binary
- `--run` build and execute binary
- `--cc <name>` select native toolchain compiler command used by backend pipelines
- `--backend <x>` select backend mode: `auto` (asm-first), `c`, `asm`
- `--passes <n>` greedy optimizer passes
- `-O4` strongest speed profile (recommended default for release/perf)
- `--max-speed` compatibility alias for `-O4`
- `--pgo-generate` build an instrumented binary for profile collection (max-speed pipeline)
- `--pgo-use <profile-dir>` use collected PGO profiles from `<profile-dir>`
- `--bolt-use <fdata>` apply BOLT profile optimization when `llvm-bolt` is available
- `--keep-c` keep generated C output
- `-o <path>` output path
- custom `--flag-name` arguments are supported via `flag flag-name() do ... end` in source.
- undefined custom flags print a warning and are ignored.
- malformed flags (for example `---bad`) print a warning and are ignored.
- grouped custom parser arguments are supported (for example `-O [ -p max -X [ --beta-features ] ]`).
- grouped tokens are strict: unbalanced `[` / `]` is a hard CLI parse error.
- grouped/inline parser tokens are exposed to scripts via:
  - `cli_token_count()`
  - `cli_token(idx)`
  - `cli_has(flag)`
  - `cli_value(flag)`
- unknown long options in grouped/custom-parser mode do not trigger undefined-flag warnings unless they are explicit top-level custom flag invocations.

## Speed Features

- AOT pipeline (ASM path): LineScript -> optimized C -> x86 ASM -> native binary
- AOT pipeline (C path): LineScript -> optimized C -> native binary
- custom x86 backend mode (`--backend asm`) with automatic C++ then C fallback
- no mandatory VM and no GC runtime
- greedy multi-pass optimizer (constant folding, DCE, branch/loop simplification, inlining)
- constant small-trip loop unrolling in optimizer
- aggressive native flags in `-O4` mode
- optional PGO stage: `--pgo-generate` -> collect workload profiles -> `--pgo-use <dir>`
- optional BOLT stage: `--bolt-use <fdata>` for post-link hot-path layout tuning when available
- on Windows, eligible `-O4` programs use an ultra-minimal no-CRT backend path for lower launch overhead
- loop vectorization/unroll hints in emitted C
- `parallel for` maps to OpenMP parallel+SIMD when available (serial fallback otherwise)
- low-overhead C emission
- static typing with predictable codegen

## Safety and Robustness

- input files must use `.lsc` or `.ls`
- hardened `--cc` validation blocks shell metacharacters
- compile-time rejection of constant `division/modulo by zero`
- `for` loops with runtime `step == 0` terminate safely (zero iterations)
- `parallel for` rejects `break`/`continue` and outer-variable assignments
- deterministic regression suite for runtime + compile-failure behavior
- undefined/malformed custom CLI flags are warned and ignored (non-fatal)

## Math Library

Built-in helpers include:
- input/text: `input`, `len`, `bytes_len`, `is_empty`, `contains`, `includes`, `starts_with`, `ends_with`, `find`
- typed numeric input: `input_i64`, `input_f64`
- text transforms: `lower`, `upper`, `trim`, `replace`, `substring`, `repeat`, `reverse`
- bytewise: `byte_at`, `ord`, `chr`
- manual memory (no GC): `mem_alloc`, `mem_realloc`, `mem_free`, `mem_set`, `mem_copy`, `mem_read_i64`, `mem_write_i64`, `mem_read_f64`, `mem_write_f64`
- collections: `array_new`, `array_len`, `array_free`, `array_push`, `array_get`, `array_set`, `array_pop`, `array_join`, `array_includes`
- maps/dicts: `dict_new`, `dict_len`, `dict_free`, `dict_set`, `dict_get`, `dict_has`, `dict_remove`, plus `map_*`/`object_*` aliases including `map_free` and `object_free`
- graphics: `gfx_new`, `gfx_free`, `gfx_width`, `gfx_height`, `gfx_clear`, `gfx_set`, `gfx_get`, `gfx_line`, `gfx_rect`, `gfx_save_ppm`
- game runtime: `game_new`, `game_free`, `game_width`, `game_height`, `game_set_target_fps`, `game_set_fixed_dt`, `game_should_close`, `game_begin`, `game_poll`, `game_present`, `game_end`, `game_delta`, `game_frame`, `game_clear`, `game_set`, `game_get`, `game_line`, `game_rect`, `game_draw_gfx`, `game_save_ppm`, `game_checksum`
- pygame-like aliases: `pg_init`, `pg_quit`, `pg_should_quit`, `pg_begin`, `pg_end`, `pg_set_target_fps`, `pg_set_fixed_dt`, `pg_clear`, `pg_draw_pixel`, `pg_draw_line`, `pg_draw_rect`, `pg_blit`, `pg_get_pixel`, `pg_save_ppm`, `pg_checksum`, `pg_mouse_x/y/norm_x/norm_y`, `pg_delta`, `pg_frame`, `pg_key_down`, `pg_key_down_name`, `pg_surface_*`
- numpy-like vectors: `np_new`, `np_free`, `np_len`, `np_get`, `np_set`, `np_copy`, `np_fill`, `np_from_range`, `np_linspace`, `np_sum`, `np_mean`, `np_min`, `np_max`, `np_dot`, `np_add`, `np_sub`, `np_mul`, `np_div`, `np_add_scalar`, `np_mul_scalar`, `np_clip`, `np_abs`
- normalized mouse input: `game_mouse_x`, `game_mouse_y`, `game_mouse_norm_x`, `game_mouse_norm_y`
- physics/camera/input: `phys_new`, `phys_free`, `phys_set_position`, `phys_set_velocity`, `phys_move`, `phys_apply_force`, `phys_step`, `phys_get_x/y/z`, `phys_get_vx/vy/vz`, `phys_is_soft`, `camera_bind`, `camera_target`, `camera_set_offset`, `camera_get_x/y/z`, `key_down`, `key_down_name`
- `sqrt`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`
- `exp`, `log`, `log10`, `floor`, `ceil`, `round`, `pow`
- `pi`, `tau`, `deg_to_rad`, `rad_to_deg`
- conversion/integer utils: `parse_i64`, `parse_f64`, `to_i32`, `to_i64`, `to_f32`, `to_f64`, `gcd`, `lcm`
- `max_i64`, `min_i64`, `abs_i64`, `clamp_i64`
- `max_f64`, `min_f64`, `abs_f64`, `clamp_f64`
- ad-hoc generic numeric helpers: `max`, `min`, `abs`, `clamp`

## Programming Models

LineScript supports multiple styles:
- OOP-style: use `class` syntax (`this.field`, `obj.method()`) or low-level `object_*`/`dict_*` handles.
- class fields can optionally use `i32`/`f32`; `i64`/`f64` are still fully supported.
- Functional style: pure functions + immutable locals via `declare const`.
- Generic style: ad-hoc generic numeric helpers (`max/min/abs/clamp`) with type inference.

## Packaging

PowerShell packager:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
```

Bash packager (Linux/macOS):

```bash
bash ./scripts/package.sh
```

This produces a distributable ZIP with:
- `lsc.exe`
- `linescript.cmd` / `linescript.ps1`
- `docs/`
- `examples/`
- latest-only dist policy: packaging removes older `LineScript-*` bundles/zips so `dist` only keeps the newest release.

For a new project, extract the ZIP and copy its contents into your project folder, then run:

```powershell
.\linescript.cmd .\main.lsc -O4 --cc clang
```

CMake packaging is also available with `CPack`:

```powershell
cmake -S . -B build
cmake --build build --config Release
cpack --config build\CPackConfig.cmake
```

## Testing

Run the deterministic test suite:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_tests.ps1
```

Linux/macOS:

```bash
bash ./tests/run_tests.sh
```

Run deterministic stress tests:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_stress_tests.ps1
```

Linux/macOS:

```bash
bash ./tests/run_stress_tests.sh
```

What it covers:
- runtime output checks
- compile-failure checks and diagnostics
- CLI hardening checks (`--cc` injection, extension validation)
- fixed-workload stress programs with exact expected outputs
- extreme stress coverage for memory-handle reuse, task concurrency reuse, HTTP burst roundtrips, and guarded edge-range execution
- vectorization report check (clang `-Rpass`) to verify vectorized loop remarks are present

## Examples

- `examples/high_level.lsc`
- `examples/control_flow.lsc`
- `examples/if_elif.lsc`
- `examples/fib.lsc`
- `examples/benchmark.lsc`
- `examples/print_demo.lsc`
- `examples/math_showcase.lsc`
- `examples/format_block.lsc`
- `examples/format_marker.lsc`
- `examples/state_speed.lsc`
- `examples/collections_input.lsc`
- `examples/format_mode_input.lsc`
- `examples/module_math.lsc`
- `examples/module_main.lsc`
- `examples/parallel_async.lsc`
- `examples/graphics_demo.lsc`
- `examples/physics_demo.lsc`
- `examples/game_headless_demo.lsc`
- `examples/game_window_demo.lsc`
- `examples/paradigms_demo.lsc`

## Documentation

- `docs/LANGUAGE_GUIDE.md`
- `docs/SYNTAX.md`
- `docs/STDLIB.md`
- `docs/BENCHMARK.md`
- `docs/IDE_SETUP.md`

VS Code note:
- Copy `.vscode/tasks.json` and `.vscode/launch.json` into your project if you use VS Code.
- If you use another editor/IDE, you can ignore it.
