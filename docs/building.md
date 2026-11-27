# Building

## Requirements

* CMake ≥ 3.20
* A C++20 compiler: MSVC 2022 (v143, tested with 14.44), or a recent GCC/Clang for the Linux/macOS shared-library build.
* Nothing else — no Unreal Engine, no third-party libraries.

## Configure and build (Windows, Visual Studio 2022 / Build Tools)

```powershell
cd BattlefieldCpp
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
cmake --build build --config Release
```

Outputs land in `build\bin\<Config>\`:

| File | Project |
|---|---|
| `Battlefield.dll` (+ `.pdb`, import lib in `build\lib`) | Battlefield |
| `BattlefieldTests.dll` | BattlefieldTests |
| `BattlefieldTestRunner.exe` | BattlefieldTests (console host) |

All three are written to the same folder so Windows resolves the DLLs without any `PATH` changes.

Other generators work too, for example Ninja from a *Developer PowerShell for VS*:

```powershell
cmake -S . -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ninja
```

## Visual Studio / Rider solution

Run `Generate.bat` (or `cmake -S . -B vs2022 -G "Visual Studio 17 2022" -A x64`) to produce
`vs2022\BattlefieldCpp.sln` (`Generate.bat -h` / `./Generate.sh -h` print the usage), then open it in Visual Studio 2022 or Rider (File → Open → the `.sln`).
It contains `Battlefield`, `BattlefieldTests` and `BattlefieldTestRunner`; the runner is the startup project, so
**F5 debugs the whole suite** and breakpoints in `Battlefield/Private` are hit. To debug a single test, set
*Debugging → Command Arguments* of `BattlefieldTestRunner` to `--filter Grid.NeighbourTest`.

The solution is generated, not hand-maintained: after adding or removing source files edit the `CMakeLists.txt`
and the IDE reloads the solution (or re-run the script). `vs2022/` is git-ignored.
Rider can also open the root `CMakeLists.txt` directly as a CMake project.

## Options

| Option | Default | Effect |
|---|---|---|
| `BATTLEFIELD_BUILD_TESTS` | `ON` | Build `BattlefieldTests.dll`, the runner and the CTest entry. Turn off to ship only the core DLL. |

Compile definitions:

| Definition | Where | Meaning |
|---|---|---|
| `BATTLEFIELD_EXPORTS` | set by CMake while building `Battlefield.dll` | `BATTLEFIELD_API` → `dllexport` |
| `BATTLEFIELD_STATIC` | set by you | `BATTLEFIELD_API` → empty; compile the `Battlefield/Private` sources directly into your own binary |

## Install / consume from another project

```powershell
Generate.bat install Debug        # builds and installs into BattlefieldCpp\dist
Generate.bat install Release      # installing both configurations into the same prefix is supported
```

`dist\` then contains `bin\Battlefield.dll`, `lib\Battlefield.lib`, the public headers in `include\` and a CMake
package in `lib\cmake\Battlefield`. A consumer lives in its **own** project and solution (see
`../BattlefieldGame`, documented in its own `docs/`):

```cmake
find_package(Battlefield CONFIG REQUIRED)          # with -DBattlefield_DIR=<dist>/lib/cmake/Battlefield
target_link_libraries(MyGame PRIVATE Battlefield::Battlefield)   # include path + dllimport come along
```

Copy the DLL of the configuration you build (`Battlefield.dll`, or `Battlefieldd.dll` for Debug) next to your executable; the sample does this in a post-build step with `$<TARGET_FILE:Battlefield::Battlefield>`. Debug carries a `d` postfix (`DEBUG_POSTFIX`) precisely so both configurations can live in one `dist`: without it the second install overwrote the first.

## ABI notes

The exported API uses C++ types in its signatures (`std::vector`, `std::function`, `std::unique_ptr` members).
Build the DLL and every client with the **same compiler version and the same dynamic C runtime** (`/MD` or `/MDd`;
CMake's default). Mixing Debug and Release binaries is not supported.
