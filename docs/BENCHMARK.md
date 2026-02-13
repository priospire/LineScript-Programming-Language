# LineScript Benchmark Notes

Collected on `2026-02-06` in this workspace.

Environment:
- frontend compiler build: `clang++ -std=c++20 -O3 -Wall -Wextra -pedantic src/lsc.cpp -o lsc.exe`
- backend C compiler: `clang`
- LineScript mode: `-O4` (`--max-speed` alias still supported)
- selected native flags: speed-first `-O3` profile with aggressive math, native tuning, loop unrolling, and aliasing/frame-pointer optimizations (OpenMP attempted first, with fallback)

## Bench Programs

- `examples/fib.lsc`
- `examples/benchmark.lsc` (50,000,000 loop iterations)

## Build Times

Measured with repeated `.\lsc.exe <file> --build --cc clang -O4`.

`fib.lsc` over 5 runs:
- average: `188.8115 ms`
- min: `184.6763 ms`
- max: `193.1799 ms`

`benchmark.lsc` over 5 runs:
- average: `205.4011 ms`
- min: `201.8718 ms`
- max: `211.7106 ms`

## Runtime Times

Measured from PowerShell using `Measure-Command` on compiled binaries.

`fib.exe`:
- warm-up: 3 runs
- measured: 12 runs
- average: `7.3609 ms`
- min: `6.2258 ms`
- max: `12.2390 ms`

`benchmark.exe`:
- warm-up: 2 runs
- measured: 8 runs
- average: `83.1532 ms`
- min: `80.3172 ms`
- max: `86.5077 ms`

Raw `benchmark.exe` run samples (ms):
- `86.5077`
- `81.2639`
- `80.4871`
- `80.3172`
- `86.4524`
- `82.0398`
- `85.8531`
- `82.3042`

## Reproduce

```powershell
clang++ -std=c++20 -O3 -Wall -Wextra -pedantic src/lsc.cpp -o lsc.exe

.\lsc.exe examples\benchmark.lsc --build --cc clang -O4 -o examples\benchmark.exe
Measure-Command { .\examples\benchmark.exe > $null }
```

## Deterministic Stress Suite

Run fixed-workload stress programs with exact expected outputs:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_stress_tests.ps1
```

Stress cases:
- `tests/stress/stress_integer_reduction.lsc`
- `tests/stress/stress_mod_pipeline.lsc`
- `tests/stress/stress_parallel_independent.lsc`
- `tests/stress/stress_memory_reuse.lsc`
- `tests/stress/stress_task_spawn_reuse.lsc`
- `tests/stress/stress_http_burst_roundtrip.lsc`
- `tests/stress/stress_edge_guarded_ranges.lsc`
