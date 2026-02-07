# LineScript Built-in Library

These built-ins are available without imports.

## 1. Core I/O

```linescript
print(x) -> void
println(x) -> void
input() -> str
input(prompt: str) -> str
input_i64() -> i64
input_i64(prompt: str) -> i64
input_f64() -> f64
input_f64(prompt: str) -> f64
```

Printable argument types:
- `i64`
- `f64`
- `bool`
- `str`

Notes:
- `print(x)` prints values with no newline.
- `println(x)` prints values with a newline.
- `input()` reads one line from stdin and returns `str`.
- `input(prompt)` prints prompt text first, then reads one line.
- `input_i64` and `input_f64` read one line and parse numeric values.
- invalid numeric text parses as `0` (same behavior as `parse_i64` / `parse_f64` defaults).

Example:

```linescript
main() -> i64 do
  declare name = input("Name: ")
  println(name)
  return 0
end
```

## 2. Timing and Run Markers

```linescript
clock_ms() -> i64
clock_us() -> i64
.stateSpeed() -> void
.format() -> void
.freeConsole() -> void
FreeConsole() -> void
```

Notes:
- `.stateSpeed()` can appear anywhere in a function and prints `speed_us=<elapsed_microseconds_since_function_start>`.
- `.format()` takes no args and has no closing statement.
- `.freeConsole()` / `FreeConsole()` take no args and have no closing statement.
- on Windows they detach the console so only the game window remains.
- on non-Windows targets they are safe no-ops.
- on Windows, if `.format()` exists anywhere in a program, builds use the GUI subsystem (no extra console window is created).
- when launched from an existing terminal, stdio remains tied to that parent terminal.

## 3. String and Byte Helpers

```linescript
len(s: str) -> i64
bytes_len(s: str) -> i64
is_empty(s: str) -> bool
contains(s: str, part: str) -> bool
includes(s: str, part: str) -> bool
starts_with(s: str, prefix: str) -> bool
ends_with(s: str, suffix: str) -> bool
find(s: str, part: str) -> i64
replace(s: str, from: str, to: str) -> str
trim(s: str) -> str
lower(s: str) -> str
upper(s: str) -> str
substring(s: str, start: i64, count: i64) -> str
repeat(s: str, n: i64) -> str
reverse(s: str) -> str
byte_at(s: str, idx: i64) -> i64
ord(s: str) -> i64
chr(code: i64) -> str
```

Notes:
- `find` returns first 0-based index or `-1`.
- `byte_at` returns `-1` for out-of-range indexes.
- `ord("")` returns `-1`.
- `chr` returns empty string for invalid byte code.
- string results are safe to store in variables and pass between functions (stable value semantics).

## 4. Arrays (Dynamic String Collections)

Arrays are runtime handles (`i64`).

```linescript
array_new() -> i64
array_len(arr: i64) -> i64
array_free(arr: i64) -> void
array_push(arr: i64, value: str) -> void
array_get(arr: i64, idx: i64) -> str
array_set(arr: i64, idx: i64, value: str) -> void
array_pop(arr: i64) -> str
array_join(arr: i64, sep: str) -> str
array_includes(arr: i64, value: str) -> bool
```

Index behavior:
- `array_get` out of range returns `""`.
- `array_set` auto-grows array and fills missing slots with `""`.
- call `array_free` when done to release array storage immediately.

Example (input saved to array index):

```linescript
main() -> i64 do
  declare arr = array_new()
  array_set(arr, 0, input("first: "))
  println(array_get(arr, 0))
  return 0
end
```

## 5. Dictionaries and Maps (String Key/Value)

Dictionaries/maps are runtime handles (`i64`) with `str -> str` entries.

```linescript
dict_new() -> i64
dict_len(d: i64) -> i64
dict_free(d: i64) -> void
dict_set(d: i64, key: str, value: str) -> void
dict_get(d: i64, key: str) -> str
dict_has(d: i64, key: str) -> bool
dict_remove(d: i64, key: str) -> void
```

Map aliases:

```linescript
map_new() -> i64
map_len(m: i64) -> i64
map_free(m: i64) -> void
map_set(m: i64, key: str, value: str) -> void
map_get(m: i64, key: str) -> str
map_has(m: i64, key: str) -> bool
map_remove(m: i64, key: str) -> void
```

Object aliases (OOP-style dictionary model):

```linescript
object_new() -> i64
object_len(obj: i64) -> i64
object_free(obj: i64) -> void
object_set(obj: i64, key: str, value: str) -> void
object_get(obj: i64, key: str) -> str
object_has(obj: i64, key: str) -> bool
object_remove(obj: i64, key: str) -> void
```

Example (input saved to dictionary):

```linescript
main() -> i64 do
  declare d = dict_new()
  dict_set(d, "name", input("name: "))
  println(dict_get(d, "name"))
  dict_free(d)
  return 0
end
```

## 6. Manual Memory (C-Style, No GC)

Pointers are represented as `i64` addresses.

```linescript
mem_alloc(bytes: i64) -> i64
mem_realloc(ptr: i64, bytes: i64) -> i64
mem_free(ptr: i64) -> void
mem_set(ptr: i64, byte: i64, bytes: i64) -> void
mem_copy(dst: i64, src: i64, bytes: i64) -> void
mem_read_i64(ptr: i64) -> i64
mem_write_i64(ptr: i64, value: i64) -> void
mem_read_f64(ptr: i64) -> f64
mem_write_f64(ptr: i64, value: f64) -> void
```

Notes:
- no garbage collector is used.
- ownership is explicit: you allocate and free manually.
- `mem_alloc`/`mem_realloc` return `0` on failure.

Example:

```linescript
main() -> i64 do
  declare p: i64 = mem_alloc(16)
  mem_write_i64(p, 123)
  mem_write_f64(p + 8, 2.5)
  println(mem_read_i64(p))
  println(mem_read_f64(p + 8))
  mem_free(p)
  return 0
end
```

## 7. Native Graphics (2D Raster)

Graphics are built in and require no external package.

```linescript
gfx_new(width: i64, height: i64) -> i64
gfx_free(canvas: i64) -> void
gfx_width(canvas: i64) -> i64
gfx_height(canvas: i64) -> i64
gfx_clear(canvas: i64, r: i64, g: i64, b: i64) -> void
gfx_set(canvas: i64, x: i64, y: i64, r: i64, g: i64, b: i64) -> void
gfx_get(canvas: i64, x: i64, y: i64) -> i64
gfx_line(canvas: i64, x0: i64, y0: i64, x1: i64, y1: i64, r: i64, g: i64, b: i64) -> void
gfx_rect(canvas: i64, x: i64, y: i64, w: i64, h: i64, r: i64, g: i64, b: i64, fill: bool) -> void
gfx_save_ppm(canvas: i64, path: str) -> bool
```

Notes:
- `gfx_new` returns `-1` on invalid size or allocation failure.
- canvas size is bounded for safety.
- out-of-range pixel writes are ignored (no crash).
- colors are clamped to `0..255`.
- `gfx_get` returns packed RGB: `(r << 16) | (g << 8) | b`, or `-1` when invalid/out of range.
- `gfx_save_ppm` writes a binary PPM (`P6`) image.

Example:

```linescript
main() -> i64 do
  declare g = gfx_new(256, 256)
  gfx_clear(g, 20, 20, 30)
  gfx_line(g, 0, 0, 255, 255, 255, 60, 60)
  gfx_rect(g, 40, 40, 120, 80, 40, 180, 255, false)
  println(gfx_save_ppm(g, "frame.ppm"))
  gfx_free(g)
  return 0
end
```

## 8. Native 2D Game Runtime

```linescript
game_new(width: i64, height: i64, title: str, visible: bool) -> i64
game_free(game: i64) -> void
game_width(game: i64) -> i64
game_height(game: i64) -> i64
game_set_target_fps(game: i64, fps: i64) -> void
game_set_fixed_dt(game: i64, dt_seconds: f64) -> void
game_should_close(game: i64) -> bool
game_begin(game: i64) -> void
game_poll(game: i64) -> void
game_present(game: i64) -> void
game_end(game: i64) -> void
game_delta(game: i64) -> f64
game_frame(game: i64) -> i64
game_mouse_x(game: i64) -> f64
game_mouse_y(game: i64) -> f64
game_mouse_norm_x(game: i64) -> f64
game_mouse_norm_y(game: i64) -> f64
game_clear(game: i64, r: i64, g: i64, b: i64) -> void
game_set(game: i64, x: i64, y: i64, r: i64, g: i64, b: i64) -> void
game_get(game: i64, x: i64, y: i64) -> i64
game_line(game: i64, x0: i64, y0: i64, x1: i64, y1: i64, r: i64, g: i64, b: i64) -> void
game_rect(game: i64, x: i64, y: i64, w: i64, h: i64, r: i64, g: i64, b: i64, fill: bool) -> void
game_draw_gfx(game: i64, canvas: i64, dst_x: i64, dst_y: i64) -> void
game_save_ppm(game: i64, path: str) -> bool
game_checksum(game: i64) -> i64
```

Notes:
- `game_new(..., visible=false)` is deterministic headless mode for tests/CI.
- `game_new(..., visible=true)` opens a native window on Windows.
- use `game_begin`/`game_end` once per frame.
- `game_set_fixed_dt` forces deterministic frame delta.
- `game_set_target_fps(0)` disables frame sleep throttling.
- mouse coordinates are normalized from window client size into internal simulation resolution.
- `game_mouse_x/y` return simulation-space coordinates.
- `game_mouse_norm_x/y` return normalized `[0.0, 1.0]` coordinates.

Example:

```linescript
main() -> i64 do
  declare game = game_new(320, 180, "LineScript Game", true)
  game_set_target_fps(game, 60)

  while not game_should_close(game) do
    game_begin(game)
    game_clear(game, 18, 22, 30)
    declare mx = to_i64(round(game_mouse_x(game)))
    declare my = to_i64(round(game_mouse_y(game)))
    game_rect(game, mx - 3, my - 3, 6, 6, 120, 255, 120, true)
    game_rect(game, 30, 30, 80, 50, 255, 170, 60, true)
    game_line(game, 0, 0, 319, 179, 90, 220, 255)
    game_end(game)
  end

  game_free(game)
  return 0
end
```

## 9. Native Physics, Camera, and Input Polling

```linescript
phys_new(x: f64, y: f64, z: f64, mass: f64, soft: bool) -> i64
phys_free(obj: i64) -> void
phys_set_position(obj: i64, x: f64, y: f64, z: f64) -> void
phys_set_velocity(obj: i64, vx: f64, vy: f64, vz: f64) -> void
phys_move(obj: i64, dx: f64, dy: f64, dz: f64) -> void
phys_apply_force(obj: i64, fx: f64, fy: f64, fz: f64) -> void
phys_step(dt: f64) -> void
phys_get_x(obj: i64) -> f64
phys_get_y(obj: i64) -> f64
phys_get_z(obj: i64) -> f64
phys_get_vx(obj: i64) -> f64
phys_get_vy(obj: i64) -> f64
phys_get_vz(obj: i64) -> f64
phys_is_soft(obj: i64) -> bool
camera_bind(obj: i64) -> void
camera_target() -> i64
camera_set_offset(x: f64, y: f64, z: f64) -> void
camera_get_x() -> f64
camera_get_y() -> f64
camera_get_z() -> f64
key_down(code: i64) -> bool
key_down_name(name: str) -> bool
```

Notes:
- `phys_new` returns `-1` on allocation/capacity failure.
- camera defaults to the first active object and auto-rebinds if target is freed.
- `soft = true` enables spring-like damping behavior; `soft = false` is rigid-body style.
- `phys_step(dt)` ignores `dt <= 0`.
- key polling is native on Windows; other platforms safely return `false`.

Example:

```linescript
main() -> i64 do
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
  return 0
end
```

## 10. Numeric Conversion and Utilities

```linescript
parse_i64(s: str) -> i64
parse_f64(s: str) -> f64
to_i64(x: f64) -> i64
to_f64(x: i64) -> f64
max_i64(a: i64, b: i64) -> i64
min_i64(a: i64, b: i64) -> i64
abs_i64(x: i64) -> i64
clamp_i64(x: i64, lo: i64, hi: i64) -> i64
gcd(a: i64, b: i64) -> i64
lcm(a: i64, b: i64) -> i64
max_f64(a: f64, b: f64) -> f64
min_f64(a: f64, b: f64) -> f64
abs_f64(x: f64) -> f64
clamp_f64(x: f64, lo: f64, hi: f64) -> f64
max(a: i64|f64, b: i64|f64) -> i64|f64
min(a: i64|f64, b: i64|f64) -> i64|f64
abs(x: i64|f64) -> i64|f64
clamp(x: i64|f64, lo: i64|f64, hi: i64|f64) -> i64|f64
```

Generic numeric helper notes:
- `max/min/abs/clamp` are ad-hoc generic builtins for numeric types.
- when all args are `i64`, result is `i64`; otherwise result is `f64`.
- non-numeric arguments are compile-time errors.

## 11. Math

```linescript
pi() -> f64
tau() -> f64
deg_to_rad(d: f64) -> f64
rad_to_deg(r: f64) -> f64
sqrt(x: f64) -> f64
sin(x: f64) -> f64
cos(x: f64) -> f64
tan(x: f64) -> f64
asin(x: f64) -> f64
acos(x: f64) -> f64
atan(x: f64) -> f64
atan2(y: f64, x: f64) -> f64
exp(x: f64) -> f64
log(x: f64) -> f64
log10(x: f64) -> f64
floor(x: f64) -> f64
ceil(x: f64) -> f64
round(x: f64) -> f64
pow(a: f64, b: f64) -> f64
```

Also available in expressions:
- power operators `**` and `^`

## 12. Formatting Helpers

```linescript
formatOutput(x) -> str
FormatOutput(x) -> str
```

Supported input types:
- `i64`
- `f64`
- `bool`
- `str`

Block formatting:

```linescript
formatOutput("\n") do
  print("value=")
  print(42)
end
```

Behavior:
- captures all `print` and `println` inside the block
- appends optional `str` suffix argument
- emits final text once after block

## 13. Concurrency and Parallelism

```linescript
spawn(worker()) -> i64
await(task_id: i64) -> void
await_all() -> void
```

Parallel loop primitive:

```linescript
parallel for i in 0..1000000 do
  declare x = i * i
end
```

Rules:
- `spawn` target must be zero-arg and return `void`.
- `parallel for` forbids `break` and `continue`.
- `parallel for` forbids assignments to outer-scope variables.

## 14. Pygame-like API (`pg_*`)

These are convenience aliases over native `game_*` and `gfx_*` primitives.

```linescript
pg_init(width: i64, height: i64, title: str, visible: bool) -> i64
pg_quit(game: i64) -> void
pg_should_quit(game: i64) -> bool
pg_begin(game: i64) -> void
pg_end(game: i64) -> void
pg_set_target_fps(game: i64, fps: i64) -> void
pg_set_fixed_dt(game: i64, dt_seconds: f64) -> void
pg_clear(game: i64, r: i64, g: i64, b: i64) -> void
pg_draw_pixel(game: i64, x: i64, y: i64, r: i64, g: i64, b: i64) -> void
pg_draw_line(game: i64, x0: i64, y0: i64, x1: i64, y1: i64, r: i64, g: i64, b: i64) -> void
pg_draw_rect(game: i64, x: i64, y: i64, w: i64, h: i64, r: i64, g: i64, b: i64, fill: bool) -> void
pg_blit(game: i64, surface: i64, dst_x: i64, dst_y: i64) -> void
pg_get_pixel(game: i64, x: i64, y: i64) -> i64
pg_save_ppm(game: i64, path: str) -> bool
pg_checksum(game: i64) -> i64
pg_mouse_x(game: i64) -> f64
pg_mouse_y(game: i64) -> f64
pg_mouse_norm_x(game: i64) -> f64
pg_mouse_norm_y(game: i64) -> f64
pg_delta(game: i64) -> f64
pg_frame(game: i64) -> i64
pg_key_down(code: i64) -> bool
pg_key_down_name(name: str) -> bool
pg_surface_new(width: i64, height: i64) -> i64
pg_surface_free(surface: i64) -> void
pg_surface_clear(surface: i64, r: i64, g: i64, b: i64) -> void
pg_surface_set(surface: i64, x: i64, y: i64, r: i64, g: i64, b: i64) -> void
pg_surface_get(surface: i64, x: i64, y: i64) -> i64
pg_surface_line(surface: i64, x0: i64, y0: i64, x1: i64, y1: i64, r: i64, g: i64, b: i64) -> void
pg_surface_rect(surface: i64, x: i64, y: i64, w: i64, h: i64, r: i64, g: i64, b: i64, fill: bool) -> void
pg_surface_save_ppm(surface: i64, path: str) -> bool
```

Example:

```linescript
main() -> i64 do
  declare g = pg_init(320, 180, "Demo", true)
  pg_set_target_fps(g, 60)
  while not pg_should_quit(g) do
    pg_begin(g)
    pg_clear(g, 18, 22, 30)
    pg_draw_rect(g, 40, 30, 80, 50, 255, 170, 60, true)
    pg_end(g)
  end
  pg_quit(g)
  return 0
end
```

## 15. NumPy-like Vector API (`np_*`)

Numeric vectors are runtime handles (`i64`) over contiguous `f64` storage.

```linescript
np_new(n: i64) -> i64
np_free(v: i64) -> void
np_len(v: i64) -> i64
np_get(v: i64, idx: i64) -> f64
np_set(v: i64, idx: i64, value: f64) -> void
np_copy(v: i64) -> i64
np_fill(v: i64, value: f64) -> void
np_from_range(start: f64, stop: f64, step: f64) -> i64
np_linspace(start: f64, stop: f64, count: i64) -> i64
np_sum(v: i64) -> f64
np_mean(v: i64) -> f64
np_min(v: i64) -> f64
np_max(v: i64) -> f64
np_dot(a: i64, b: i64) -> f64
np_add(a: i64, b: i64) -> i64
np_sub(a: i64, b: i64) -> i64
np_mul(a: i64, b: i64) -> i64
np_div(a: i64, b: i64) -> i64
np_add_scalar(v: i64, k: f64) -> void
np_mul_scalar(v: i64, k: f64) -> void
np_clip(v: i64, lo: f64, hi: f64) -> void
np_abs(v: i64) -> void
```

Notes:
- vector math loops are emitted with SIMD-friendly loop hints in generated C.
- invalid handles are safe: read aggregations return `0` and mutating ops become no-ops.
- `np_new` and builders return `-1` on invalid sizes or allocation/capacity failure.
- `np_div` writes `0.0` for divisions by zero.

Example:

```linescript
main() -> i64 do
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
  return 0
end
```
