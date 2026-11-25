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

Run `GenerateSolution.bat` (or `cmake -S . -B vs2022 -G "Visual Studio 17 2022" -A x64`) to produce
`vs2022\BattlefieldCpp.sln`, then open it in Visual Studio 2022 or Rider (File → Open → the `.sln`).
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
cmake --install build --config Release --prefix C:\sdk\battlefield
```

installs `bin\Battlefield.dll`, `lib\Battlefield.lib` and the `Public/` headers under `include\`. From another CMake
project in the same repository tree you can simply do:

```cmake
add_subdirectory(BattlefieldCpp/Battlefield)
target_link_libraries(MyGame PRIVATE Battlefield)   # include path + dllimport come with it
```

Remember to copy `Battlefield.dll` next to your executable.

## ABI notes

The exported API uses C++ types in its signatures (`std::vector`, `std::function`, `std::unique_ptr` members).
Build the DLL and every client with the **same compiler version and the same dynamic C runtime** (`/MD` or `/MDd`;
CMake's default). Mixing Debug and Release binaries is not supported.
