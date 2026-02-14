# LineScript Syntax Reference

This file documents the current LineScript grammar and behavior.

For the exhaustive built-in API list and input polling details, see `docs/API_REFERENCE.md`.

## 1. File Extensions

- `.lsc` (recommended)
- `.ls` (supported)

## 2. Comments

Only C-style single-line comments are supported:

```linescript
// comment
```

## 3. Primitive Types

- `i64`
- `i32`
- `f64`
- `f32`
- `bool`
- `str`
- `void`

### Quick Glossary

- `declare`: defines a variable.
- `owned`: after `declare`, enables deterministic auto-free at scope exit for supported handle constructors.
- `const`: after `declare`, makes that variable immutable.
- `class`: declares a class with fields, constructor, and methods.
- `this`: current instance handle inside class methods/constructor.
- `i64`: integer numbers (for counters, indexes, ids).
- `i32`: 32-bit integer numbers (lighter integer storage).
- `f64`: floating-point numbers (for decimals/math).
- `f32`: 32-bit floating-point numbers.
- `bool`: logical true/false.
- `str`: text in double quotes, e.g. `"hello"`.

## 4. Function Declarations

Preferred high-level syntax:

```linescript
add(a: i64, b: i64) -> i64 do
  return a + b
end
```

Top-level script mode is also supported. You can run simple code without wrapping it in `main()`:

```linescript
print("milk")
```

You can also define and call functions from top level:

```linescript
sudo() {
  print("hi")
}

sudo()
```

Compatibility syntax:

```linescript
fn add(a: i64, b: i64) -> i64 do
  return a + b
end
```

Inline and extern:

```linescript
inline sqr(x: i64) -> i64 do
  return x * x
end

extern fn c_add(a: i64, b: i64) -> i64;
```

Return type:
- optional
- defaults to `void` when omitted

Explicit throw contract:

```linescript
load_config() -> i64 throws IOError do
  return 1
end

main() -> i64 throws IOError do
  return load_config()
end
```

Rules:
- `throws <ErrorName>[, <ErrorName>...]` is optional.
- if a function calls another function that declares `throws X`, the caller must also declare `throws X`.

Entry-point resolution when you build/run:
1. top-level statements (if present) run first as the program entry
2. otherwise `main()` is used if defined
3. otherwise, if there is exactly one zero-argument function, it is used as entry
4. otherwise build fails with an explicit entry-point error

### Script-First Mode (No Boilerplate)

You can write most programs directly at top level. Functions are optional.

```linescript
declare total: i64 = 0

for i in 0..5 do
  total += i
end

if total >= 10 do
  println(total)
end
```

Top-level supports:
- variable declarations and assignments
- `if` / `elif` / `else` and `unless`
- `for` and `while`
- class declarations plus object usage
- function definitions and function calls
- collection, math, input, graphics, and game builtins
- `formatOutput(...) do ... end`, `.format()`, and `.freeConsole()`

### C++-Style Classes

LineScript supports class syntax with typed fields, constructor, methods, and member access:

```linescript
class Counter do
  declare value: i64 = 0
  declare enabled: bool = true

  constructor(start: i64) do
    this.value = start
  end

  add(delta: i64) -> void do
    if this.enabled do
      this.value += delta
    end
  end

  get() -> i64 do
    return this.value
  end
end

main() -> i64 do
  declare c = Counter(10)
  c.add(5)
  println(c.get())
  return 0
end
```

Rules:
- class fields use `declare <name>: <type> [= <expr>]`.
- fields currently must be declared before class methods.
- constructor is optional; if omitted, a default constructor is generated.
- constructor name can be `constructor(...)` (recommended).
- single inheritance is supported with `class Child extends Parent`.
- derived constructors can call direct base constructor with init-list syntax:

```linescript
constructor(a: i64, b: i64) : Parent(a) do
  this.extra = b
end
```

- methods are declared as `name(...) -> type do ... end` (or `fn name(...)`).
- method modifiers are supported: `public`, `protected`, `private`, `static`, `virtual`, `override`, `final`.
- top-level functions support overloads with exact-match then safe numeric widening.
- class methods support overload declarations; call sites currently require distinct arity for unambiguous resolution.
- operator overloads support free and class-member forms:

```linescript
operator +(a: i64, b: i64) -> i64 do
  return a - (-b) - (-100)
end

class NumberBox do
  declare value: i64 = 0
  constructor(seed: i64) do
    this.value = seed
  end
  fn operator +(rhs: i64) -> i64 do
    return this.value - (-rhs)
  end
end
```

- operator resolution order is: class member overload, then free overload, then built-in operator.
- class operator methods must take exactly 1 parameter (the right-hand side); free operator overrides must take 2 parameters.
- use `this.field` / `this.method(...)` inside class members.
- use `obj.field`, `obj.field = ...`, `obj.field += ...`, and `obj.method(...)` outside.
- static methods must be called via class name: `ClassName.method(...)`.
- class names are valid type annotations and map to handle type `i64` internally.
- class fields can use `i32`/`f32` (optional), or stay with `i64`/`f64`.
- complete OOP reference and examples: `docs/OOP.md`.

## 5. Block Styles

`do/end`:

```linescript
main() -> i64 do
  return 0
end
```

Braces:

```linescript
main() -> i64 {
  return 0;
}
```

## 6. Variable Declarations

Declarations use `declare`.

```linescript
declare x
declare y: i64
declare z = 123
declare message: str = "hello"
declare const pi_value: f64 = 3.14159
declare owned cache = dict_new()
```

Rules:
- `declare const` requires an initializer
- untyped `declare x` defaults to `i64` with value `0`
- typed declarations without initializer default to `0`, `0.0`, or `false`
- assignment requires prior declaration
- `declare owned` requires an `i64` handle constructor with a matching free function
- `declare owned` is currently restricted to non-loop scopes for deterministic cleanup cost
- owned handle variables are not assignable and cannot be returned directly

Assignment:

```linescript
x = x + 1
```

## 7. Control Flow

### `if / elif / else`

```linescript
grade(score: i64) -> i64 do
  if score >= 90 do
    return 4
  elif score >= 80 do
    return 3
  else do
    return 2
  end
end
```

### `unless` (unique convenience syntax)

`unless cond` is equivalent to `if !cond`.

```linescript
ensure_positive(x: i64) -> i64 do
  unless x > 0 do
    return 0
  end
  return x
end
```

### `while`

```linescript
countdown(n: i64) -> i64 do
  declare i = n
  while i > 0 do
    i = i - 1
  end
  return 0
end
```

### `for` range loop

```text
for <name> in <start_expr>..<stop_expr> [step <step_expr>] do ... end
```

Examples:

```linescript
for i in 0..10 do
  println(i)
end

for j in 10..0 step -2 do
  println(j)
end
```

Rules:
- stop bound is exclusive
- step cannot be zero
- positive step iterates while `i < stop`
- negative step iterates while `i > stop`
- runtime `step == 0` executes zero iterations (safe no-op)

### `parallel for` (data parallel loop)

Use `parallel for` for independent loop work:

```linescript
parallel for i in 0..1000000 do
  declare x = i * i
  if x < 0 do
    println(x)
  end
end
```

Notes:
- compiled with OpenMP when available (`-fopenmp`)
- falls back to serial execution when OpenMP is unavailable
- `break` and `continue` are not allowed inside `parallel for`
- assigning outer variables inside `parallel for` is rejected

### `break` and `continue`

```linescript
scan(n: i64) -> i64 do
  declare count: i64 = 0
  for i in 0..n do
    if i % 2 == 0 do
      continue
    end
    if count > 100 do
      break
    end
    count = count + 1
  end
  return count
end
```

## 8. Return Statements

```linescript
return
return value
```

## 9. Expressions

Literals:
- `123`
- `3.14`
- `true`
- `false`
- `"text"`

String escapes:
- `\"` quote
- `\\` backslash
- `\n`, `\r`, `\t`

Unary:
- `-x`
- `!x`
- `not x`

Arithmetic:
- `+`, `-`, `*`, `/`, `%`
- power: `**` or `^`
- postfix updates: `x++`, `x--`
- compound assignment: `+=`, `-=`, `*=`, `/=`, `%=`, `^=`, `**=`

Comparison:
- `==`, `!=`, `<`, `<=`, `>`, `>=`

Logical:
- `&&`, `||`
- `and`, `or`, `not`

Function calls:

```linescript
declare value = add(2, 3)
println(value)
println("x")
```

Input:

```linescript
declare name = input()
declare city = input("City: ")
declare n = input_i64()
declare price = input_f64("Price: ")
```

Rules:
- `input()` returns `str`
- `input(prompt_str)` returns `str`
- `input_i64()` / `input_i64(prompt_str)` return `i64`
- `input_f64()` / `input_f64(prompt_str)` return `f64`

Async/task calls:

```linescript
worker() -> void do
  println("done")
end

main() -> i64 do
  declare task = spawn(worker())
  await(task)
  await_all()
  return 0
end
```

Formatting helper:

```linescript
println(formatOutput(42))
println(FormatOutput(3.14))
println(formatOutput("x"))
```

Format block syntax (`do/end` or braces):

```linescript
formatOutput("\n") do
  print("value=")
  print(42)
end
```

Notes:
- captures all `print`/`println` output inside the block
- emits the captured text once after the block
- optional argument must be `str` and is appended at the end of captured output

Inline speed probe (statement form, no block):

```linescript
main() -> i64 do
  declare x: i64 = 0
  for i in 0..10000 do
    x = x + i
  end
  .stateSpeed()
  return 0
end
```

Notes:
- `.stateSpeed()` takes zero arguments
- it can be placed anywhere inside a function body
- it prints `speed_us=<elapsed microseconds>` for the current function

Inline run-format marker (statement form, no block):

```linescript
main() -> i64 do
  .format()
  declare name = input("Name: ")
  println(name)
  return 0
end
```

Notes:
- `.format()` takes zero arguments
- no `end` is required
- `.format()` does not disable stdout; your program output still prints normally
- `lsc --run` output is clean: only your program output is shown (compiler status lines are suppressed)
- when launched from an existing terminal, input/output still use that parent terminal

Inline console detach (statement form, no block):

```linescript
main() -> i64 do
  .freeConsole()
  return 0
end
```

Notes:
- `.freeConsole()` and `FreeConsole()` both take zero arguments
- no `end` is required
- on Windows, they detach the process console (useful for visible game-window runs)
- on non-Windows targets, they are no-ops

### Superuser Developer Mode

Use `superuser()` when you explicitly want low-level diagnostics and relaxed guard rails for debugging.

```linescript
superuser()
su.trace.on()
println(su.capabilities())
su.trace.off()
```

Available privileged helpers:
- `su.trace.on()` / `su.trace.off()`: runtime statement tracing.
- `su.capabilities() -> str`: active privileges and limits.
- `su.memory.inspect() -> str`: current superuser memory counters/limits.
- `su.limit.set(step_limit: i64, mem_limit: i64)`: runtime step + memory limits.
- `su.compiler.inspect() -> str`: compiler/runtime inspection summary.
- `su.ir.dump()`: requests generated IR/C dump for the current build.
- `su.debug.hook(tag: str)`: debug hook marker for tooling/log streams.

Rules:
- calling `su.*` without `superuser()` fails with: `Not privileged`.
- when `superuser()` is present, LineScript prints a terminal warning that safety checks are relaxed.
- in `.format()` mode, superuser debug logs are sent to debug/error output instead of normal program output.
- in shell mode, `su.help` is a privileged command that prints available superuser shell controls and APIs.
- in shell mode, `su.verbosity.1` is minimal and `su.verbosity.5` is extremely verbose (source, lex/token, parse, and compile command tracing).
- in superuser mode, most LineScript guardrails are downgraded to warnings so backend compiler/toolchain validation can continue.
- syntax/parse errors and impossible-to-compile code are still errors.

Postfix increment/decrement:

```linescript
declare x = 10
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

This expands to assignment-style arithmetic updates.
Notes:
- `%=` is valid only for `i64`.
- `^=` / `**=` follow the same power semantics as `^` / `**`.

Collection handles and index-style operations through builtins:

```linescript
declare arr = array_new()
array_push(arr, "a")
array_set(arr, 1, "b")
println(array_get(arr, 1))

declare d = dict_new()
dict_set(d, "key", array_get(arr, 1))
println(dict_get(d, "key"))
```

Notes:
- arrays/maps are runtime handles stored as `i64`
- index/key read/write is done via builtin functions (`array_get/array_set`, `dict_get/dict_set`)
- this allows input values to be saved directly into array/dict entries

Option/Result (Rust-style enum-like handles, no null references):

```linescript
declare maybe_name = option_some("Lin")
declare none_name = option_none()
println(option_is_some(maybe_name))
println(option_unwrap_or(none_name, "guest"))
option_free(maybe_name)
option_free(none_name)

declare ok = result_ok("done")
declare err = result_err("IOError", "file missing")
println(result_is_ok(ok))
println(result_error_type(err))
println(result_error_message(err))
result_free(ok)
result_free(err)
```

Native graphics through builtins (no external package required):

```linescript
declare canvas = gfx_new(320, 200)
gfx_clear(canvas, 15, 20, 30)
gfx_line(canvas, 0, 0, 319, 199, 255, 0, 0)
gfx_rect(canvas, 20, 20, 80, 50, 0, 200, 255, false)
gfx_set(canvas, 10, 10, 255, 255, 0)
println(gfx_get(canvas, 10, 10))
println(gfx_save_ppm(canvas, "frame.ppm"))
gfx_free(canvas)
```

Graphics notes:
- graphics handle type is `i64`
- out-of-bounds draw calls are ignored safely
- `gfx_get` returns packed RGB `(r << 16) | (g << 8) | b`
- `gfx_save_ppm` returns `bool`

### Native Physics, Camera, and Input Polling

Physics/camera/input polling builtins:

```linescript
declare player = phys_new(0.0, 2.0, 0.0, 1.0, false)
camera_bind(player)
camera_set_offset(0.0, 1.8, -6.0)

if key_down_name("SPACE") do
  phys_apply_force(player, 0.0, 8.0, 0.0)
end

phys_step(0.016)
println(camera_target())
println(to_i64(round(camera_get_y())))
phys_free(player)
```

Physics/camera notes:
- physics object handles are `i64`
- camera defaults to the first active object
- freeing the active camera target rebinds to the next active object
- `soft = true` enables spring-like behavior
- `key_down`/`key_down_name` are native on Windows and return `false` on non-Windows targets

Native game runtime builtins:

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

Game runtime notes:
- `visible=false` enables deterministic headless mode (useful in tests)
- `game_set_fixed_dt(game, dt)` sets deterministic per-frame `game_delta(game)`
- `game_draw_gfx(game, canvas, x, y)` blits a `gfx_*` canvas into the game surface
- `game_checksum(game)` returns a deterministic frame checksum
- `game_mouse_x/y` map mouse position into simulation-space coordinates
- `game_mouse_norm_x/y` return normalized `[0.0, 1.0]` mouse coordinates

Pygame-like aliases:

```linescript
declare g = pg_init(320, 180, "Demo", true)
pg_begin(g)
pg_clear(g, 10, 12, 20)
pg_draw_rect(g, 20, 20, 80, 40, 255, 170, 60, true)
pg_end(g)
pg_quit(g)
```

Notes:
- `pg_*` mirrors `game_*`/`gfx_*` functionality with pygame-style naming.
- `pg_surface_*` functions operate on offscreen surfaces (`i64` handles).

NumPy-like vector helpers:

```linescript
declare a = np_from_range(0.0, 5.0, 1.0)
declare b = np_linspace(1.0, 5.0, 5)
declare c = np_add(a, b)
println(np_dot(a, b))
np_clip(c, 1.0, 4.0)
np_free(a)
np_free(b)
np_free(c)
```

Notes:
- `np_*` vectors are `i64` handles to `f64` buffers.
- invalid vector handles are safe (no crashes); invalid reads return `0`.

## 10. Statement Terminators

Either form works:
- newline
- semicolon

```linescript
declare x = 1
declare y = 2
```

```linescript
declare x = 1; declare y = 2;
```

## 11. Entry Point (Script-First)

```linescript
main() -> i64 do
  println(123)
  return 0
end
```

Rules:
- `main()` is optional
- if top-level statements exist, they run as the entry point
- otherwise `main()` is used if present
- otherwise, one zero-argument function can be used as entry automatically
- if multiple zero-argument functions exist and no top-level/main exists, build fails with an explicit entry error
- `main` must have zero parameters
- `main() -> i64` return value is process exit code
- `main() -> void` exits with code `0`

## 12. Multi-File Modular Build

Compile modules in one command:

```powershell
.\lsc.exe src\math.lsc src\main.lsc --run -O4 --cc clang
```

All top-level functions are merged into one program.

Speed recommendation:
- use `-O4` for release/performance runs (`--max-speed` is an alias).
- optional max-speed pipeline flags:
 - `--pgo-generate` build instrumentation profile
 - `--pgo-use <dir>` consume collected profile data
 - `--bolt-use <fdata>` apply BOLT profile data when `llvm-bolt` is available
- default backend mode is `--backend auto` (x86 asm first, then C++/C fallback).
- on Windows, eligible `-O4` programs can use an ultra-minimal backend path for lower runtime launch overhead.

## 13. Type Rules

- static typing with inference
- implicit widening allowed: `i64 -> f64`
- implicit narrowing disallowed: `f64 -> i64`
- `str` supports assignment, equality/inequality, and printing
- `%` requires `i64`
- constant `x / 0` is rejected at compile time
- constant `x % 0` is rejected at compile time
- `if`, `unless`, and `while` conditions require `bool`
- `for` start/stop/step require `i64`
- `spawn(fn())` requires a zero-arg `void` function call
- `spawn(...)` returns `i64` task handle
- `await(handle)` requires `i64` and returns `void`
- `formatOutput(<end>)` block argument must be `str`
- `.stateSpeed()` requires zero args and returns `void`
- `.format()` requires zero args and returns `void`
- `.freeConsole()` / `FreeConsole()` require zero args and return `void`
- `input()` / `input(str)` return `str`
- `input_i64()` / `input_i64(str)` return `i64`
- `input_f64()` / `input_f64(str)` return `f64`
- `array_*` and `dict_*` use `i64` handles
- `object_*` uses `i64` handles
- `mem_*` APIs use raw pointer addresses encoded as `i64`
- `gfx_*` uses `i64` canvas handles
- `game_*` uses `i64` game handles
- `game_mouse_x/y/norm_x/norm_y` return `f64`
- `phys_*` uses `i64` object handles
- `camera_bind` and `camera_target` use `i64` object handles
- `key_down` requires `i64`; `key_down_name` requires `str`
- `max/min/abs/clamp` require numeric arguments and return numeric results

## 14. Operator Precedence (high to low)

1. call / grouping / postfix update statements: `()`, `x++`, `x--`
2. unary: `-`, `!`
3. power: `**`, `^`
4. multiplicative: `*`, `/`, `%`
5. additive: `+`, `-`
6. comparison: `<`, `<=`, `>`, `>=`
7. equality: `==`, `!=`
8. logical AND: `&&` / `and`
9. logical OR: `||` / `or`

## 15. Error Format

```text
LineScript error (<file or stage>): line <n>, col <m>: <message>
```

## 16. Programming Paradigms

OOP-style:
- use `class` for C++-style fields + methods + constructor syntax
- use `object_*`/`dict_*` handles directly for low-level dynamic object storage

Functional style:
- prefer pure helper functions
- use `declare const` for immutable local values
- compose small functions for transform/reduce loops

Generic style:
- use ad-hoc generic numeric builtins: `max`, `min`, `abs`, `clamp`
- use type inference to keep call sites concise

## 17. Extra Notes

LineScript includes a few compatibility-style CLI flags for quick checks.

Custom CLI flags:

```linescript
flag hello-world() do
  println("flag triggered")
end
```

Usage:
- run with `--hello-world` to execute that flag function before normal script entry.
- flags do not replace script execution; your normal script still runs.
- undefined flags produce a warning and are ignored.
- malformed flags produce a warning and are ignored.
