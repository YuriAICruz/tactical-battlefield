# Testing

## What is tested

`BattlefieldTests.dll` contains the 166 automation tests of the Unreal module, transpiled from
`Source/Battlefield/Private/Tests` (the bodies are the originals; only the framework underneath changed), plus the
12 tests added for `features/01-bugfixes.md`, 7 for `features/02-unit-attributes.md` and 6 for `features/03-tile-tags.md` (191 in total), spread over the suites below.

| Suite (name prefix) | Source file | Tests | Covers |
|---|---|---|---|
| `Hellmarch.Battlefield.Actor.*` | `ActorTests.cpp` | 43 | units on the grid, movement/AP budgets, damage/heal/death, status effects and their events, attribute bag, tag and name |
| `Hellmarch.Battlefield.Team.*` | `TeamTests.cpp` | 14 | turn rotation, who may act, refresh on turn start, defeat and winner |
| `Hellmarch.Battlefield.Grid.*` | `GridTests.cpp` | 62 | bounds, tiles, tile tags, grid↔world maths, neighbours, BFS and weighted paths, weighted distance/areas |
| `Hellmarch.Battlefield.Controller.*` | `BattlefieldControllerTests.cpp` | 31 | spawn (incl. name/attribute copy), move, attack, end turn, death removal, victory |
| `Hellmarch.Battlefield.Query.*` | `BattlefieldQueryTests.cpp` | 24 | movement area, targets, visibility, line of sight, valid actions, winner |
| `Hellmarch.Battlefield.Flow.*` | `BattlefieldFlowTests.cpp` | 17 | start battle, full turn cycles, an AI-vs-AI simulation, every controller event |

## Running

```powershell
build\bin\Debug\BattlefieldTestRunner.exe                       # everything
build\bin\Debug\BattlefieldTestRunner.exe --filter Grid.        # only tests whose name contains "Grid."
build\bin\Debug\BattlefieldTestRunner.exe --list                # print test names
build\bin\Debug\BattlefieldTestRunner.exe --quiet               # only failures + summary
build\bin\Debug\BattlefieldTestRunner.exe --verbose             # also print Battlefield's internal error log
ctest --test-dir build -C Debug --output-on-failure             # same suite through CTest
```

The exit code is the number of failed tests (`0` = success, `2` = the filter matched nothing).

## The test DLL's C API

`BattlefieldTests/Public/BattlefieldTests.h` — a flat C interface so any host can drive the suite:

```c
int         BFT_GetTestCount(void);
const char* BFT_GetTestName(int index);
int         BFT_RunTests(const char* filter, BFT_ResultCallback cb, void* userData); // failed count, -1 = no match
void        BFT_SetVerboseLog(int enabled);
// void cb(const char* testName, int passed, int checks, const char* errors, void* userData)
```

Example from Python:

```python
import ctypes
dll = ctypes.CDLL(r"build\bin\Release\BattlefieldTests.dll")
CB = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_char_p, ctypes.c_void_p)
failed = dll.BFT_RunTests(None, CB(lambda n, ok, c, e, u: print("PASS" if ok else "FAIL", n.decode())), None)
```

## The automation shim

`Private/Framework/AutomationShim.h` re-creates the small part of Unreal's automation API the tests use, so test code
reads exactly as it did in the engine:

| Unreal API | Shim |
|---|---|
| `IMPLEMENT_SIMPLE_AUTOMATION_TEST(Class, "Name", Flags)` | declares `Class : FAutomationTestBase` and self-registers it in `FTestRegistry` |
| `bool Class::RunTest(const FString& Parameters)` | unchanged |
| `TestTrue/False/Equal/NotEqual/GreaterThan/Null/NotNull` | same signatures; failures are collected, the test continues |
| `FString`, `TEXT("…")` | `std::string`, identity macro |
| `EAutomationTestFlags::…` | accepted and ignored |

A test fails if any check failed, if `RunTest` returned `false`, or if it threw. Tests are executed in alphabetical
order of their full name.

## Adding a test

1. Open (or create) a `*.cpp` in `BattlefieldTests/Private/Tests/` and include `Framework/AutomationShim.h`
   and `Tests/TestsHelper.h`.
2. Declare and implement:

   ```cpp
   IMPLEMENT_SIMPLE_AUTOMATION_TEST(MyNewTest, "Hellmarch.Battlefield.Grid.MyNewTest",
       EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

   bool MyNewTest::RunTest(const FString& Parameters)
   {
       FGrid grid = TestsHelper::CreateTestGrid();
       TestTrue(TEXT("Origin is valid"), grid.IsValid(0, 0));
       return true;
   }
   ```
3. If it is a new file, add it to `BattlefieldTests/CMakeLists.txt`.

`TestsHelper` provides the base stats/resources/weapon constants, `CreateTestGrid(w, h)` (default 10 × 10),
`CreateController(&grid)` and `ContainsTile(list, coord)`.
