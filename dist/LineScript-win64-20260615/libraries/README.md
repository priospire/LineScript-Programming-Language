# LineScript Preset Libraries

Preset libraries are ordinary LineScript modules. Compile them together with your program:

```powershell
.\linescript.cmd .\libraries\math\math_extras.lsc .\main.lsc -O4 --cc clang
```

Installed packs:

- `math`: small numeric helpers.
- `text`: string helpers built on LineScript's native string functions.
- `collections`: array/dictionary convenience helpers.
- `game`: native game/window helper functions.
- `window`: cleaner window lifecycle helpers around `game_new`, `game_begin`, and `game_end`.
- `ui`: simple immediate-mode panels, buttons, progress bars, and mouse hit-testing.
- `graphics2d`: reusable 2D drawing helpers such as grids, borders, and crosshairs.
- `engine3d`: lightweight 3D projection helpers for drawing simple projected points/segments.
- `physics`: physics body, soft-body, gravity, camera-follow, and speed helpers.
- `http`: HTTP server/client convenience helpers.

These files do not run by themselves. They only define functions for your program to call.

Game-oriented packs can be combined:

```powershell
.\linescript.cmd .\libraries\window\window_helpers.lsc .\libraries\ui\ui_tools.lsc .\libraries\graphics2d\graphics2d_helpers.lsc .\libraries\physics\physics_helpers.lsc .\main.lsc -O4 --cc clang
```
