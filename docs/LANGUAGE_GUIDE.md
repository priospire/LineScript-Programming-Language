# LineScript Language Guide

This guide explains how to write and run LineScript programs quickly.

For the full API catalog (including key names, mouse buttons, and scroll wheel APIs), see
`docs/API_REFERENCE.md`.

## 1. Running Code

Fastest workflow on Windows:

```powershell
.\linescript.cmd your_file.lsc -O4 --cc clang
```

Linux/macOS equivalent:

```bash
./lsc your_file.lsc --run -O4 --cc clang
```

Performance note:
- treat `-O4` as the standard release mode.
- `--max-speed` is still supported as a compatibility alias.
- use non-`-O4` builds mainly for debugging.
- backend defaults to `--backend auto` (x86 asm-first, then C++/C fallback).
- on Windows, `-O4` can use an ultra-minimal backend path for eligible programs to reduce startup overhead.
- on Linux, threading-enabled builds (`spawn`/`await`) automatically link with `-pthread`.

Behavior:
- auto-builds `lsc.exe` when needed
- defaults to `--build --run` when not explicitly provided

Manual usage:

```powershell
.\lsc.exe your_file.lsc --run -O4 --cc clang
```

Fast syntax/type validation without building:

```powershell
.\lsc.exe your_file.lsc --check
```

## 2. Your First Program

```linescript
main() -> i64 do
  println(12345)
  return 0
end
```

You can also run direct top-level statements:

```linescript
print("hello")
```

You can define and call functions without `main()` too:

```linescript
sudo() {
  print("hi")
}

sudo()
```

Entry selection in build/run mode:
1. if top-level statements exist, LineScript runs them as the entry script
2. else if `main()` exists, `main()` is the entry
3. else if there is exactly one zero-argument function (any name), that function is used
4. else LineScript reports an explicit entry-point error

## 2.1 No-Boilerplate Script Mode

You do not need `main()` for most work. This is valid and runnable:

```linescript
declare total = 0
for i in 0..5 do
  total += i
end
println(total)
```

Top-level is fine for normal coding:
- declarations, loops, conditionals
- class definitions and object usage
- function definitions and calls
- built-ins (`array_*`, `dict_*`, `math`, `input`, `gfx_*`, `game_*`, etc.)

## 3. Function Start/End

Use `do` to start and `end` to close:

```linescript
add(a: i64, b: i64) -> i64 do
  return a + b
end
```

Braces are also supported:

```linescript
add(a: i64, b: i64) -> i64 {
  return a + b;
}
```

Explicit throw contracts are supported:

```linescript
read_config() -> i64 throws IOError do
  return 1
end

main() -> i64 throws IOError do
  return read_config()
end
```

## 4. Variables (`declare`)

```linescript
vars_demo() -> i64 do
  declare x
  declare y: i64
  declare z = 10
  declare const limit: i64 = 1000
  declare owned cache = dict_new()

  x = 5
  y = z + x
  return y
end
```

## 5. Keyword and Type Glossary

- `declare`: declares a variable.
- `owned`: after `declare`, enables deterministic scope-exit free for supported handle constructors.
- `const`: marks a declared variable as immutable (cannot be reassigned).
- `class`: defines a class with typed fields, constructor, and methods.
- `this`: current instance inside class members.
- `i64`: 64-bit signed integer type.
- `i32`: 32-bit signed integer type.
- `f64`: 64-bit floating-point type.
- `f32`: 32-bit floating-point type.
- `bool`: boolean type (`true`/`false`).
- `str`: string type for text like `"hello"`.
- `->`: return type annotation in a function signature.
- `throws`: explicit error contract annotation in a function signature.
- `do` / `end`: start and end a block.
- `inline`: hint for aggressive inlining of small hot functions.
- `extern`: declares a function implemented outside LineScript.
- `parallel`: marks a `for` loop as data-parallel.
- `spawn`: launches a zero-arg `void` function asynchronously and returns a task handle.
- `await`: waits for one task handle.
- `await_all`: waits for all spawned tasks.
- `input`: reads one line and returns `str` (`input()` or `input("prompt")`).
- `input_i64` / `input_f64`: read one line and parse numeric values.
- `formatOutput` / `FormatOutput`: convert values to `str`.
- `.stateSpeed()`: prints elapsed microseconds since the current function started.
- `.format()`: inline run-format marker (no `end`); it suppresses compiler/build status chatter.
- with `.format()`, `lsc --run` prints only program output (no compiler status jargon).
- `.freeConsole()` / `FreeConsole()`: inline console detach marker (no `end`).
- `array_*`, `dict_*`, `map_*`: built-in collection helpers using `i64` handles.
- `object_*`: OOP-style aliases over dictionary handles (`i64`).
- `option_*` / `result_*`: enum-style nullable/error handles (`i64`) for explicit non-null flows.
- `gfx_*`: built-in native 2D graphics helpers using `i64` canvas handles.
- `game_*`: built-in 2D game runtime (frame loop + rendering + deterministic headless mode).
- `pg_*`: pygame-like aliases over `game_*` and `gfx_*`.
- `np_*`: numpy-like vector helpers over contiguous `f64` buffers.
- `phys_*`: built-in physics object helpers using `i64` handles.
- `camera_*`: built-in camera helpers bound to physics objects.
- `key_down` / `key_down_name`: key polling helpers (native on Windows).

## 6. Conditionals

Standard:

```linescript
classify(x: i64) -> i64 do
  if x < 0 do
    return -1
  elif x == 0 do
    return 0
  else do
    return 1
  end
end
```

Unique syntax with `unless`:

```linescript
nonzero(x: i64) -> i64 do
  unless x != 0 do
    return 1
  end
  return x
end
```

## 7. Loops

`while`:

```linescript
countdown(n: i64) -> i64 do
  declare i = n
  while i > 0 do
    i = i - 1
  end
  return 0
end
```

`for` range:

```linescript
sum_to(n: i64) -> i64 do
  declare total: i64 = 0
  for i in 0..(n + 1) do
    total = total + i
  end
  return total
end
```

Loop control:

```linescript
scan(n: i64) -> i64 do
  declare count: i64 = 0
  for i in 0..n do
    if i % 2 == 0 do
      continue
    end
    if count > 1000 do
      break
    end
    count = count + 1
  end
  return count
end
```

Postfix increment/decrement:

```linescript
declare i = 0
i++
i--
i += 5
i -= 2
i *= 3
i /= 2
i %= 7
i ^= 2
i **= 2
```

Notes:
- `%=` requires `i64`.
- `^=` / `**=` use power semantics.

## 8. Parallel and Async

Data-parallel loop:

```linescript
parallel for i in 0..1000000 do
  declare x = i * i
  if x < 0 do
    println(x)
  end
end
```

LineScript also includes a few compatibility-style CLI flags for quick checks.

Custom CLI flags:

```linescript
flag hello() do
  println("hello flag")
end
```

- invoke with `--hello`.
- custom flags run before normal script entry and do not suppress normal script execution.
- undefined flags produce warnings and are ignored.
- malformed flags produce warnings and are ignored.

Async task primitives:

```linescript
worker() -> void do
  println("worker complete")
end

main() -> i64 do
  declare t = spawn(worker())
  await(t)
  await_all()
  return 0
end
```

Rules:
- `spawn` must be `spawn(fn())`
- `fn` must take zero args and return `void`
- `parallel for` does not allow `break`/`continue`

Formatting helper:

```linescript
println(formatOutput(42))
println(FormatOutput(3.5))
println(formatOutput("x"))
```

Block formatter:

```linescript
formatOutput("\n") do
  print("value=")
  print(42)
end
```

Inline speed probe:

```linescript
main() -> i64 do
  declare total: i64 = 0
  for i in 0..500000 do
    total = total + i
  end
  .stateSpeed()
  return 0
end
```

Inline run-format marker (no closing statement):

```linescript
main() -> i64 do
  .format()
  declare name = input("Name: ")
  println(name)
  return 0
end
```

Superuser mode (advanced diagnostics):

```linescript
superuser()
su.trace.on()
println(su.capabilities())
su.trace.off()
```

Notes:
- without `superuser()`, any `su.*` call fails with `Not privileged`.
- with `superuser()`, LineScript prints a terminal warning because checks are intentionally relaxed.
- `.format()` keeps normal program output clean and routes superuser diagnostics to debug/error output.
- useful superuser calls:
- `su.memory.inspect()`
- `su.limit.set(step_limit, mem_limit)`
- `su.compiler.inspect()`
- `su.ir.dump()`
- `su.debug.hook("tag")`

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

Notes:
- on Windows, `.freeConsole()` / `FreeConsole()` detach the console window.
- on non-Windows targets they are safe no-ops.

## 9. Input and Collections

Input to variable:

```linescript
declare name = input("name: ")
declare age = input_i64("age: ")
declare score = input_f64("score: ")
```

Input to array index:

```linescript
declare arr = array_new()
array_set(arr, 0, input())
println(array_get(arr, 0))
```

Input to dictionary key:

```linescript
declare d = dict_new()
dict_set(d, "name", input("name: "))
println(dict_get(d, "name"))
```

Map aliases:

```linescript
declare m = map_new()
map_set(m, "x", "10")
println(map_get(m, "x"))
```

Native graphics (2D raster):

```linescript
declare canvas = gfx_new(320, 200)
gfx_clear(canvas, 20, 20, 30)
gfx_line(canvas, 0, 0, 319, 199, 255, 0, 0)
gfx_rect(canvas, 20, 20, 80, 50, 0, 200, 255, false)
println(gfx_get(canvas, 10, 10))
println(gfx_save_ppm(canvas, "frame.ppm"))
gfx_free(canvas)
```

Notes:
- out-of-range pixel writes are ignored safely
- `gfx_get` returns packed RGB `(r << 16) | (g << 8) | b`
- `gfx_save_ppm` writes a `.ppm` image and returns `bool`

Physics + camera + input polling:

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

Notes:
- camera defaults to the first active object and auto-rebinds if target is freed.
- `key_down*` is native on Windows; non-Windows targets safely return `false`.

Native game loop (smooth window mode + deterministic test mode):

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

Deterministic headless mode for tests:
- create with `game_new(..., false)`
- `game_set_fixed_dt(game, dt)` for fixed frame step
- `game_set_target_fps(game, 0)` to disable sleeping
- verify output with `game_checksum(game)`
- `game_mouse_x/y` map mouse coordinates to simulation resolution
- `game_mouse_norm_x/y` return normalized `[0.0, 1.0]`

Pygame-like aliases:

```linescript
declare g = pg_init(320, 180, "Demo", true)
pg_set_target_fps(g, 60)
while not pg_should_quit(g) do
  pg_begin(g)
  pg_clear(g, 18, 22, 30)
  pg_draw_rect(g, 30, 30, 80, 50, 255, 170, 60, true)
  pg_end(g)
end
pg_quit(g)
```

NumPy-like vectors:

```linescript
declare a = np_from_range(0.0, 5.0, 1.0)
declare b = np_linspace(1.0, 5.0, 5)
declare c = np_add(a, b)
println(to_i64(round(np_dot(a, b))))
np_add_scalar(c, 1.0)
np_clip(c, 2.0, 4.0)
println(to_i64(round(np_mean(c))))
np_free(a)
np_free(b)
np_free(c)
```
## 10. Operators

Arithmetic:
- `+`, `-`, `*`, `/`, `%`
- power: `**` and `^`
- compound assignment: `+=`, `-=`, `*=`, `/=`, `%=`, `^=`, `**=`

Comparison:
- `==`, `!=`, `<`, `<=`, `>`, `>=`

Logical:
- `&&`, `||`
- `and`, `or`, `not`

## 11. Printing

Recommended generic API:

```linescript
print(10)
println(10)
println(3.14159)
println(true)
println("x")
```

Behavior:
- `print(x)` prints the value of variable `x`.
- `print("x")` prints the literal text `x`.

## 12. String and Byte Helpers

Common text/byte helpers:
- `len`, `bytes_len`, `contains`, `includes`, `starts_with`, `ends_with`, `find`
- `trim`, `lower`, `upper`, `replace`, `substring`, `repeat`, `reverse`
- `byte_at`, `ord`, `chr`

## 13. Math Library

Examples:

```linescript
math_demo(x: f64) -> f64 do
  declare base = sqrt(x)
  declare trig = sin(x) + cos(x) + tan(x)
  declare p = x ** 2.0
  declare unit = deg_to_rad(180.0)
  return base + trig + p + unit + pi()
end
```

Common utility helpers:
- text: `len`, `is_empty`, `contains`, `includes`, `starts_with`, `ends_with`, `find`
- conversion: `parse_i64`, `parse_f64`, `to_i32`, `to_i64`, `to_f32`, `to_f64`
- integer helpers: `gcd`, `lcm`, `min_i64`, `max_i64`, `abs_i64`, `clamp_i64`
- ad-hoc generic numeric helpers: `max`, `min`, `abs`, `clamp`

## 14. Programming Styles

Object-Oriented style (C++-style classes):

```linescript
class Player do
  declare name: str = "player"
  declare hp: i64 = 100

  constructor(name: str, hp: i64) do
    this.name = name
    this.hp = hp
  end

  hit(dmg: i64) -> void do
    this.hp = max(0, this.hp - dmg)
  end

  alive() -> bool do
    return this.hp > 0
  end
end

main() -> i64 do
  declare p = Player("Ava", 120)
  p.hit(30)
  println(p.hp)
  println(p.alive())
  return 0
end
```

Notes:
- class fields must be declared before methods.
- class names can be used in type annotations and are stored as `i64` handles internally.
- constructor is optional; a default constructor is generated when omitted.
- classes can use `i32`/`f32` fields when you want narrower numeric storage, but `i64`/`f64` remain fully supported.

Low-level object model is still available:

```linescript
declare o = object_new()
object_set(o, "k", "v")
println(object_get(o, "k"))
```

Functional style (pure functions, immutable values, composition):

```linescript
sqr(x: i64) -> i64 do
  return x * x
end

sum_of_squares(n: i64) -> i64 do
  declare const limit = n
  declare acc: i64 = 0
  for i in 0..limit do
    acc = acc + sqr(i)
  end
  return acc
end
```

Generic style (ad-hoc numeric generics):

```linescript
println(max(2, 7))        // i64
println(max(2.5, 7.0))    // f64
println(clamp(1.5, 0.0, 1.0))
```

## 15. Modular Programs

Compile multiple files as one program:

```powershell
.\lsc.exe examples\module_math.lsc examples\module_main.lsc --run -O4 --cc clang
```

## 16. Benchmarking

Inside LineScript:

```linescript
main() -> i64 do
  declare t0 = clock_us()
  declare s: i64 = 0
  for i in 0..5000000 do
    s = s + i
  end
  declare t1 = clock_us()
  println(s)
  println(t1 - t0)
  return 0
end
```

External timing from PowerShell:

```powershell
.\lsc.exe examples\benchmark.lsc --build --cc clang -O4 -o examples\benchmark.exe
Measure-Command { .\examples\benchmark.exe > $null }
```

## 17. Performance Checklist

1. Build with `-O4` (or `--max-speed` alias).
2. Keep hot loops numeric and simple.
3. Use `inline` for very small hot functions.
4. Use `i64` unless `f64` is required.
5. Use `parallel for` only for independent loop bodies.
6. Avoid loop-carried dependencies to help vectorization.
7. Benchmark with warm-up runs before collecting timing samples.

## 18. Deterministic Testing

Run the full regression suite:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_tests.ps1
```

Linux/macOS:

```bash
bash ./tests/run_tests.sh
```

It validates:
- deterministic runtime outputs
- expected compile failures and diagnostics
- CLI hardening rules

Run deterministic stress workloads:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_stress_tests.ps1
```

Linux/macOS:

```bash
bash ./tests/run_stress_tests.sh
```

## 19. Fresh Project in VS Code / Other IDEs

Complete step-by-step setup instructions are in:

- `docs/IDE_SETUP.md`

That guide includes:
- fresh project creation
- VS Code `tasks.json`
- commands for other IDE run configurations

## 20. Native Web Server

LineScript includes built-in HTTP server/client primitives, so you can write local APIs with no external package:

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

For full API details, see:
- `docs/STDLIB.md`
