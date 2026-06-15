# LineScript Deterministic Tests

Run all tests:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_tests.ps1
```

Linux/macOS:

```bash
bash ./tests/run_tests.sh
```

Optional flags:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_tests.ps1 -FrontendCompiler clang++ -BackendCompiler clang -MaxSpeed
```

Linux/macOS equivalent:

```bash
bash ./tests/run_tests.sh --frontend-compiler clang++ --backend-compiler clang --max-speed
```

Keep compiled test artifacts:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_tests.ps1 -KeepArtifacts
```

Linux/macOS equivalent:

```bash
bash ./tests/run_tests.sh --keep-artifacts
```

What is validated:
- runtime outputs from stable `.lsc`/`.ls` programs
- expected compile-failure diagnostics
- CLI hardening (`--cc` sanitization and extension validation)
