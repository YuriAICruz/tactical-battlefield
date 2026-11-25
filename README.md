# BattlefieldCpp

Pure C++20 port of the Hellmarch `Battlefield` module (turn-based grid tactics core) as two CMake projects
that build to DLLs: **`Battlefield`** (the game core) and **`BattlefieldTests`** (the transpiled tests).

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
build\bin\Release\BattlefieldTestRunner.exe
```

Full documentation is in [`docs/`](docs/README.md).
