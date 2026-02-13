# LineScript API Reference (Complete)

This is the exhaustive API reference for LineScript v1.4.6.

Scope covered in this file:
- key detection and input polling APIs
- mouse position and mouse wheel APIs
- graphics/game/physics APIs
- concurrency, HTTP, collections, memory, math, and utility APIs
- language-level runtime forms (`formatOutput` block, custom `flag` functions, top-level execution)

Notes:
- all built-ins listed here are available without imports.
- handle-based APIs (`array_*`, `dict_*`, `gfx_*`, `game_*`, `np_*`, `phys_*`, etc.) use `i64` handle IDs.
- invalid handles are safe and return defaults (`0`, `false`, `""`, or no-op depending on function).

## Type System Notes

Core types:
- `i32`, `i64`
- `f32`, `f64`
- `bool`
- `str`
- `void`

Common rules:
- `%` is integer-only.
- `^` and `**` are power operators.
- typeless variables are inferred from initializer expression type.

## Input Detection API (Keyboard, Mouse, Scroll)

### Keyboard polling

```linescript
key_down(code: i64) -> bool
key_down_name(name: str) -> bool

pg_key_down(code: i64) -> bool
pg_key_down_name(name: str) -> bool
```

Behavior:
- `key_down(code)` expects virtual key code range `0..255`; out-of-range returns `false`.
- `key_down_name(name)` supports these exact names:
  - `W`, `A`, `S`, `D`
  - `UP`, `DOWN`, `LEFT`, `RIGHT`
  - `SPACE`, `ESC`, `SHIFT`, `CTRL`, `ENTER`
- unknown names return `false`.
- `pg_key_down*` are aliases.

Platform behavior:
- Windows: native polling via `GetAsyncKeyState`.
- non-Windows: returns `false` safely.

### Mouse position + normalized mouse coordinates

```linescript
game_mouse_x(game: i64) -> f64
game_mouse_y(game: i64) -> f64
game_mouse_norm_x(game: i64) -> f64
game_mouse_norm_y(game: i64) -> f64

pg_mouse_x(game: i64) -> f64
pg_mouse_y(game: i64) -> f64
pg_mouse_norm_x(game: i64) -> f64
pg_mouse_norm_y(game: i64) -> f64
```

Behavior:
- `game_mouse_x/y`: simulation-space mouse coordinates.
- `game_mouse_norm_x/y`: normalized `[0.0, 1.0]`.
- `pg_mouse_*` are aliases.

### Mouse wheel and mouse button polling

```linescript
game_scroll_x(game: i64) -> f64
game_scroll_y(game: i64) -> f64
game_mouse_down(game: i64, button: i64) -> bool
game_mouse_down_name(game: i64, name: str) -> bool

pg_scroll_x(game: i64) -> f64
pg_scroll_y(game: i64) -> f64
pg_mouse_down(game: i64, button: i64) -> bool
pg_mouse_down_name(game: i64, name: str) -> bool
```

Button mapping:
- `1` or `"LEFT"`
- `2` or `"RIGHT"`
- `3` or `"MIDDLE"`
- `4` or `"X1"`
- `5` or `"X2"`

Behavior:
- invalid button code/name returns `false`.
- `game_scroll_x/y` are per-poll deltas (reset each poll/frame and filled from native window events).
- in headless mode and on non-Windows targets, scroll deltas are `0`.

Example:

```linescript
main() -> i64 do
  declare g = game_new(320, 180, "input", true)
  while not game_should_close(g) do
    game_begin(g)

    if key_down_name("SPACE") do
      println("space down")
    end

    if game_mouse_down_name(g, "LEFT") do
      println("left click held")
    end

    println(to_i64(round(game_scroll_y(g))))
    game_end(g)
  end
  game_free(g)
  return 0
end
```

## Core I/O and Formatting

```linescript
print(x) -> void
println(x) -> void
print_i64(x: i64) -> void
print_f64(x: f64) -> void
print_bool(x: bool) -> void
print_str(x: str) -> void
println_i64(x: i64) -> void
println_f64(x: f64) -> void
println_bool(x: bool) -> void
println_str(x: str) -> void

input() -> str
input(prompt: str) -> str
input_i64() -> i64
input_i64(prompt: str) -> i64
input_f64() -> f64
input_f64(prompt: str) -> f64

formatOutput(x) -> str
FormatOutput(x) -> str
```

Printable types:
- `i32`, `i64`, `f32`, `f64`, `bool`, `str`

Formatting block form:

```linescript
formatOutput(end_suffix: str) do
  // statements
end
```

Notes:
- `formatOutput/FormatOutput` expression form returns `str`.
- `formatOutput` block end suffix must be `str`.

## Execution Markers and Developer Controls

```linescript
clock_ms() -> i64
clock_us() -> i64
stateSpeed() -> void
.stateSpeed() -> void
.format() -> void
.freeConsole() -> void
FreeConsole() -> void
```

Behavior:
- `.stateSpeed()` prints elapsed microseconds since current function start.
- `.format()` suppresses compiler/build chatter in `--run` mode, without removing program output.
- `.freeConsole()` / `FreeConsole()` detach console on Windows; no-op on non-Windows.

## Superuser API

```linescript
superuser() -> void
su.trace.on() -> void
su.trace.off() -> void
su.capabilities() -> str
su.memory.inspect() -> str
su.limit.set(step_limit: i64, mem_limit: i64) -> void
su.compiler.inspect() -> str
su.ir.dump() -> void
su.debug.hook(tag: str) -> void
```

Behavior:
- `su.*` requires `superuser()`.
- otherwise compile-time error: `Not privileged: call superuser() to enable developer superuser mode`.
- enabling `superuser()` prints a warning because safety guard rails are intentionally relaxed.

## Concurrency and Async

```linescript
spawn(fn()) -> i64
await(task_id: i64) -> void
await_all() -> void
parallel for i in start..end [step s] do ... end
```

Notes:
- `spawn` target must take zero args and return `void`.
- `parallel for` rejects `break/continue` and outer-variable writes.

## Native HTTP APIs

```linescript
http_server_listen(port: i64) -> i64
http_server_accept(server: i64) -> i64
http_server_read(client: i64) -> str
http_server_respond_text(client: i64, status: i64, body: str) -> void
http_server_close(server: i64) -> void

http_client_connect(host: str, port: i64) -> i64
http_client_send(client: i64, data: str) -> void
http_client_read(client: i64) -> str
http_client_close(client: i64) -> void
```

Notes:
- returns `-1` on listen/connect failure.
- reads are chunked and deterministic for fixed workloads.

## String and Byte APIs

```linescript
includes(s: str, part: str) -> bool
len(s: str) -> i64
bytes_len(s: str) -> i64
is_empty(s: str) -> bool
contains(s: str, part: str) -> bool
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

## Manual Memory APIs (No GC)

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

## Collections APIs

### Arrays

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

### Dictionaries

```linescript
dict_new() -> i64
dict_len(d: i64) -> i64
dict_free(d: i64) -> void
dict_set(d: i64, key: str, value: str) -> void
dict_get(d: i64, key: str) -> str
dict_has(d: i64, key: str) -> bool
dict_remove(d: i64, key: str) -> void
```

### Map aliases

```linescript
map_new() -> i64
map_len(m: i64) -> i64
map_free(m: i64) -> void
map_set(m: i64, key: str, value: str) -> void
map_get(m: i64, key: str) -> str
map_has(m: i64, key: str) -> bool
map_remove(m: i64, key: str) -> void
```

### Object aliases

```linescript
object_new() -> i64
object_len(obj: i64) -> i64
object_free(obj: i64) -> void
object_set(obj: i64, key: str, value: str) -> void
object_get(obj: i64, key: str) -> str
object_has(obj: i64, key: str) -> bool
object_remove(obj: i64, key: str) -> void
```

## Option and Result APIs

```linescript
option_some(value: str) -> i64
option_none() -> i64
option_is_some(opt: i64) -> bool
option_is_none(opt: i64) -> bool
option_unwrap(opt: i64) -> str
option_unwrap_or(opt: i64, fallback: str) -> str
option_free(opt: i64) -> void

result_ok(value: str) -> i64
result_err(error_type: str, message: str) -> i64
result_is_ok(res: i64) -> bool
result_is_err(res: i64) -> bool
result_value(res: i64) -> str
result_error_type(res: i64) -> str
result_error_message(res: i64) -> str
result_unwrap_or(res: i64, fallback: str) -> str
result_free(res: i64) -> void
```

## Graphics APIs

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

## Game Runtime APIs

```linescript
game_new(width: i64, height: i64, title: str, visible: bool) -> i64
game_free(game: i64) -> void
game_width(game: i64) -> i64
game_height(game: i64) -> i64
game_set_target_fps(game: i64, fps: i64) -> void
game_set_fixed_dt(game: i64, dt_seconds: f64) -> void
game_should_close(game: i64) -> bool
game_begin(game: i64) -> void
game_end(game: i64) -> void
game_poll(game: i64) -> void
game_present(game: i64) -> void
game_delta(game: i64) -> f64
game_frame(game: i64) -> i64

game_mouse_x(game: i64) -> f64
game_mouse_y(game: i64) -> f64
game_mouse_norm_x(game: i64) -> f64
game_mouse_norm_y(game: i64) -> f64
game_scroll_x(game: i64) -> f64
game_scroll_y(game: i64) -> f64
game_mouse_down(game: i64, button: i64) -> bool
game_mouse_down_name(game: i64, name: str) -> bool

game_clear(game: i64, r: i64, g: i64, b: i64) -> void
game_set(game: i64, x: i64, y: i64, r: i64, g: i64, b: i64) -> void
game_get(game: i64, x: i64, y: i64) -> i64
game_line(game: i64, x0: i64, y0: i64, x1: i64, y1: i64, r: i64, g: i64, b: i64) -> void
game_rect(game: i64, x: i64, y: i64, w: i64, h: i64, r: i64, g: i64, b: i64, fill: bool) -> void
game_draw_gfx(game: i64, canvas: i64, dst_x: i64, dst_y: i64) -> void
game_save_ppm(game: i64, path: str) -> bool
game_checksum(game: i64) -> i64
```

## Pygame-style Alias APIs

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
pg_blit(game: i64, surface: i64, x: i64, y: i64) -> void
pg_get_pixel(game: i64, x: i64, y: i64) -> i64
pg_save_ppm(game: i64, path: str) -> bool
pg_checksum(game: i64) -> i64

pg_mouse_x(game: i64) -> f64
pg_mouse_y(game: i64) -> f64
pg_mouse_norm_x(game: i64) -> f64
pg_mouse_norm_y(game: i64) -> f64
pg_scroll_x(game: i64) -> f64
pg_scroll_y(game: i64) -> f64
pg_delta(game: i64) -> f64
pg_frame(game: i64) -> i64
pg_key_down(code: i64) -> bool
pg_key_down_name(name: str) -> bool
pg_mouse_down(game: i64, button: i64) -> bool
pg_mouse_down_name(game: i64, name: str) -> bool

pg_surface_new(width: i64, height: i64) -> i64
pg_surface_free(surface: i64) -> void
pg_surface_clear(surface: i64, r: i64, g: i64, b: i64) -> void
pg_surface_set(surface: i64, x: i64, y: i64, r: i64, g: i64, b: i64) -> void
pg_surface_get(surface: i64, x: i64, y: i64) -> i64
pg_surface_line(surface: i64, x0: i64, y0: i64, x1: i64, y1: i64, r: i64, g: i64, b: i64) -> void
pg_surface_rect(surface: i64, x: i64, y: i64, w: i64, h: i64, r: i64, g: i64, b: i64, fill: bool) -> void
pg_surface_save_ppm(surface: i64, path: str) -> bool
```

## NumPy-style Vector APIs

```linescript
np_new(len: i64) -> i64
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
np_add_scalar(v: i64, s: f64) -> void
np_mul_scalar(v: i64, s: f64) -> void
np_clip(v: i64, lo: f64, hi: f64) -> void
np_abs(v: i64) -> void
```

## Physics and Camera APIs

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
```

## Conversion and Utility APIs

```linescript
parse_i64(s: str) -> i64
parse_f64(s: str) -> f64
bool_to_i64(v: bool) -> i64
i64_to_bool(v: i64) -> bool
to_i32(v: i64) -> i32
to_f32(v: f64) -> f32
to_i64(v: f64) -> i64
to_f64(v: i64) -> f64
```

Integer utilities:

```linescript
gcd(a: i64, b: i64) -> i64
lcm(a: i64, b: i64) -> i64
max_i64(a: i64, b: i64) -> i64
min_i64(a: i64, b: i64) -> i64
abs_i64(x: i64) -> i64
clamp_i64(x: i64, lo: i64, hi: i64) -> i64
```

Floating utilities:

```linescript
max_f64(a: f64, b: f64) -> f64
min_f64(a: f64, b: f64) -> f64
abs_f64(x: f64) -> f64
clamp_f64(x: f64, lo: f64, hi: f64) -> f64
```

Generic numeric helpers (language-level):

```linescript
max(a: numeric, b: numeric) -> numeric
min(a: numeric, b: numeric) -> numeric
abs(x: numeric) -> numeric
clamp(x: numeric, lo: numeric, hi: numeric) -> numeric
```

## Math APIs

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

Also available as operators:
- `**`
- `^`

## Language-Level API Forms (Not Plain Function Calls)

### Top-level execution

Entry order:
1. top-level statements if present
2. otherwise `main()`
3. otherwise exactly one zero-arg function

### Custom CLI flags defined in source

```linescript
flag hello-world() do
  println("flag ran")
end
```

Run with:
- `--hello-world`

Behavior:
- custom flags run before normal script entry.
- undefined flags are warned and ignored.
- malformed flags (for example `---bad`) are warned and ignored.

### Conditional logic and operators

Supported:
- `if`, `elif`, `else`, `unless`
- `not`, `and`, `or`
- `==`, `!=`, `<`, `<=`, `>`, `>=`

### Variable declarations

Supported forms:
- `declare x`
- `declare x: i64 = 10`
- `declare const x: i64 = 10`
- `declare owned h = array_new()`

## Platform Notes

- keyboard polling is native on Windows, safe `false` fallback on non-Windows.
- game runtime supports headless deterministic mode across platforms.
- scroll and mouse button polling are event-driven in windowed mode; headless returns neutral defaults.
- no mandatory VM and no garbage collector in runtime.
