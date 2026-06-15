# LineScript Project Context

This file is the source-of-truth handoff context for this workspace.

## Workspace

- Local path: `c:\Users\user\Downloads\LineScript`
- Local workspace status: this folder is not a Git repository.
- GitHub repository: `https://github.com/priospire/LineScript-Programming-Language`
- Packaging entrypoint: `scripts/package.ps1`
- Current release family: LineScript 1.5.1c "Velocity Update"
- Local RTK setup: `rtk 0.42.4` installed to `%USERPROFILE%\.local\bin`; local developer tooling initialized.

## Core Files

- Compiler/frontend/runtime generator: `src/lsc.cpp`
- Root compiler source mirror: `lsc.cpp`
- Runtime and compiler tests: `tests/`
- Examples: `examples/`
- Main documentation: `README.md`, `docs/SYNTAX.md`, `docs/STDLIB.md`, `docs/API_REFERENCE.md`, `docs/LANGUAGE_GUIDE.md`, `docs/OOP.md`
- Changelog: `docs/CHANGELOG.md`
- VSCode extension: `vscode-extension/linescript-vscode/`
- Preset libraries: `libraries/`
- Dist output: `dist/LineScript-win64-YYYYMMDD`

## Build And Test Commands

```powershell
clang++ -std=c++20 -O3 -Wall -Wextra -pedantic src\lsc.cpp -o lsc.exe
powershell -ExecutionPolicy Bypass -File tests\run_tests.ps1 -OutputAssuranceRounds 1
Push-Location vscode-extension\linescript-vscode; npm test; Pop-Location
powershell -ExecutionPolicy Bypass -File scripts\package.ps1
```

## Latest QA Snapshot

- Compiler build: `clang++ -std=c++20 -O3 -Wall -Wextra -pedantic src\lsc.cpp -o lsc.exe` passed.
- Targeted renderer backend test: `tests\cases\runtime\renderer_backend_targets.lsc` passed through the local compiler.
- Version flag check: `lsc.exe --LineScript` reports `LineScript version 1.5.1c (Velocity update)`.
- Deterministic suite: `tests\run_tests.ps1 -FrontendCompiler clang++ -BackendCompiler clang -OutputAssuranceRounds 1` passed `167 / 167`.
- VSCode extension tests: `npm test` passed.
- Dist smoke test: packaged `dist\LineScript-win64-20260616\lsc.exe` compiled and ran `tests\cases\runtime\renderer_backend_targets.lsc`; generated smoke executable was removed after verification.

## Latest Dist

- Package directory: `dist\LineScript-win64-20260616`
- Package zip: `dist\LineScript-win64-20260616.zip`

## Current Architecture Notes

- LineScript compiles to generated C/C++ style code with aggressive native optimization flags.
- `-O4` is the primary maximum-speed flag; `--max-speed` remains an alias.
- `parallel for` lowers to OpenMP parallel/SIMD pragmas when OpenMP is available and falls back to serial behavior otherwise.
- `spawn(...)`, `await(task)`, and `await_all()` use native OS threads.
- Multicore worker controls are exposed through `task_hardware_threads`, `task_set_worker_count`, `task_worker_count`, `task_set_hyperthreading`, and `task_hyperthreading_enabled`.
- Game/window rendering currently has a built-in software raster path, Win32 visible windows, and headless deterministic rendering.
- Renderer selectors accept `software`, `opengl`/`gl`, `vulkan`/`vk`, DirectX 11 aliases, and DirectX 12 aliases; compatibility and hardware-acceleration preferences are exposed in the API even when the built-in deterministic path remains software.
- Game windows expose `windowed`, `fullscreen`, and `windowed_fullscreen` mode selectors plus optional frame-time interpolation.

## Release And GitHub Workflow

Because the workspace is not a Git repository, publishing uses a temporary clone of the GitHub repository, copies the relevant project files into that clone, commits, pushes, and opens or updates a PR.

## Safety Notes

- Do not delete user-created workspace files unless they are confirmed generated or redundant.
- Do not use destructive Git commands in the user workspace.
- Dist packaging intentionally removes old `dist/LineScript-*` folders before generating the latest package.
