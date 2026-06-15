# Preset Libraries

LineScript preset libraries are regular `.lsc` files that define helper functions. There is no special package manager yet; pass the library file before your program when compiling.

```powershell
.\linescript.cmd .\libraries\math\math_extras.lsc .\main.lsc -O4 --cc clang
```

Multiple packs can be combined:

```powershell
.\linescript.cmd .\libraries\math\math_extras.lsc .\libraries\text\text_extras.lsc .\main.lsc -O4 --cc clang
```

## Math Pack

File: `libraries/math/math_extras.lsc`

Functions:

- `ls_math_square_i64(x: i64) -> i64`
- `ls_math_cube_i64(x: i64) -> i64`
- `ls_math_sign_i64(x: i64) -> i64`
- `ls_math_is_even(x: i64) -> bool`
- `ls_math_wrap_i64(x: i64, modv: i64) -> i64`
- `ls_math_lerp_f64(a: f64, b: f64, t: f64) -> f64`

## Text Pack

File: `libraries/text/text_extras.lsc`

Functions:

- `ls_text_clean_lower(s: str) -> str`
- `ls_text_clean_upper(s: str) -> str`
- `ls_text_safe_substring(s: str, start: i64, count: i64) -> str`
- `ls_text_has_word(s: str, word: str) -> bool`
- `ls_text_repeat3(s: str) -> str`
- `ls_text_reverse(s: str) -> str`

## Collections Pack

File: `libraries/collections/collections_extras.lsc`

Functions:

- `ls_array3(a: str, b: str, c: str) -> i64`
- `ls_array_push_if_missing(arr: i64, value: str) -> bool`
- `ls_dict_get_or(d: i64, key: str, fallback: str) -> str`
- `ls_dict_put_if_missing(d: i64, key: str, value: str) -> bool`

Free arrays/dictionaries when you own them.

## Game Pack

File: `libraries/game/game_helpers.lsc`

Functions:

- `ls_game_headless(width: i64, height: i64, title: str) -> i64`
- `ls_game_toggle_fullscreen(game: i64) -> bool`
- `ls_game_clear_dark(game: i64) -> void`
- `ls_game_center_x(game: i64) -> i64`
- `ls_game_center_y(game: i64) -> i64`
- `ls_game_use_backend(name: str) -> bool`
- `ls_game_backend() -> str`
- `ls_game_backend_ready(name: str) -> bool`

`ls_game_toggle_fullscreen` uses the native fullscreen API on Windows visible windows and safely no-ops in headless mode.
Renderer backend helpers accept `software`, `opengl`, `vulkan`, `directx11`, and `directx12`; hardware acceleration depends on whether a native backend has been linked for that platform.

## Window Pack

File: `libraries/window/window_helpers.lsc`

Functions:

- `ls_window_create(width: i64, height: i64, title: str, fps: i64) -> i64`
- `ls_window_create_headless(width: i64, height: i64, title: str) -> i64`
- `ls_window_begin_frame(game: i64, r: i64, g: i64, b: i64) -> void`
- `ls_window_end_frame(game: i64) -> void`
- `ls_window_toggle_fullscreen(game: i64) -> bool`
- `ls_window_center_x(game: i64) -> i64`
- `ls_window_center_y(game: i64) -> i64`

Use this pack when you want cleaner frame lifecycle calls without writing `game_begin`, `game_clear`, and `game_end` manually.

## UI Pack

File: `libraries/ui/ui_tools.lsc`

Functions:

- `ls_ui_mouse_in(game: i64, x: i64, y: i64, w: i64, h: i64) -> bool`
- `ls_ui_panel(game: i64, x: i64, y: i64, w: i64, h: i64) -> void`
- `ls_ui_button(game: i64, x: i64, y: i64, w: i64, h: i64, hot: bool) -> void`
- `ls_ui_progress(game: i64, x: i64, y: i64, w: i64, h: i64, current: i64, maxv: i64) -> void`
- `ls_ui_label(game: i64, x: i64, y: i64, text: str) -> void`
- `ls_ui_label_color(game: i64, x: i64, y: i64, text: str, r: i64, g: i64, b: i64) -> void`
- `ls_ui_image(game: i64, bitmap: i64, x: i64, y: i64) -> void`

The UI pack is immediate-mode and intentionally small. It is meant for debug overlays, tools, menus, and early game prototypes.

## 2D Graphics Pack

File: `libraries/graphics2d/graphics2d_helpers.lsc`

Functions:

- `ls_2d_clear_night(game: i64) -> void`
- `ls_2d_draw_crosshair(game: i64, x: i64, y: i64, radius: i64, r: i64, g: i64, b: i64) -> void`
- `ls_2d_draw_border(game: i64, r: i64, g: i64, b: i64) -> void`
- `ls_2d_draw_grid(game: i64, cell: i64, r: i64, g: i64, b: i64) -> void`
- `ls_2d_fill_canvas(canvas: i64, r: i64, g: i64, b: i64) -> void`
- `ls_2d_draw_bitmap(game: i64, bitmap: i64, x: i64, y: i64) -> void`
- `ls_2d_draw_text(game: i64, x: i64, y: i64, text: str, r: i64, g: i64, b: i64) -> void`
- `ls_2d_canvas_text(canvas: i64, x: i64, y: i64, text: str, r: i64, g: i64, b: i64) -> void`

The grid helper ignores non-positive cell sizes so a bad value does not hang a game loop.

## 3D Helper Pack

File: `libraries/engine3d/engine3d_helpers.lsc`

Functions:

- `ls_3d_depth_safe(z: f64) -> f64`
- `ls_3d_project_x(x: f64, z: f64, screen_w: i64, focal: f64) -> i64`
- `ls_3d_project_y(y: f64, z: f64, screen_h: i64, focal: f64) -> i64`
- `ls_3d_draw_point(game: i64, x: f64, y: f64, z: f64, focal: f64, size: i64, r: i64, g: i64, b: i64) -> void`
- `ls_3d_draw_segment(game: i64, ax: f64, ay: f64, az: f64, bx: f64, by: f64, bz: f64, focal: f64, r: i64, g: i64, b: i64) -> void`

This is a lightweight projected-3D helper layer, not a full mesh/material renderer. It is useful for simple 3D debug views and prototypes.

## Physics Pack

File: `libraries/physics/physics_helpers.lsc`

Functions:

- `ls_phys_body(x: f64, y: f64, z: f64, mass: f64) -> i64`
- `ls_phys_soft_body(x: f64, y: f64, z: f64, mass: f64) -> i64`
- `ls_phys_step_60() -> void`
- `ls_phys_apply_gravity_scale(obj: i64, scale: f64) -> void`
- `ls_phys_bind_camera_follow(obj: i64, off_x: f64, off_y: f64, off_z: f64) -> void`
- `ls_phys_speed_xz(obj: i64) -> f64`

Free physics bodies with `phys_free(obj)` when you own them.

## HTTP Pack

File: `libraries/http/http_helpers.lsc`

Functions:

- `ls_http_listen_local(port: i64) -> i64`
- `ls_http_respond_ok(client: i64, body: str) -> void`
- `ls_http_get_root_local(port: i64) -> str`

## Installer Selection

`LineScriptSetup.exe` lets users choose which preset library packs to install. The packs are copied into an installed `libraries` folder and can be compiled alongside user programs.

The setup wizard groups packs into:

- Core libraries: math, text, collections, HTTP.
- Game engine libraries: game/window, window lifecycle, UI, 2D graphics, 3D projection, physics.
- Tooling: VSCode extension files and optional test suites.
