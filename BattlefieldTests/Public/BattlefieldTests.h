#pragma once

// C API exported by BattlefieldTests.dll. Any host (the bundled runner, a CI script via
// ctypes/P-Invoke, another C++ application...) can drive the whole test suite through it.

#if defined(_WIN32)
#if defined(BATTLEFIELDTESTS_EXPORTS)
#define BATTLEFIELDTESTS_API __declspec(dllexport)
#else
#define BATTLEFIELDTESTS_API __declspec(dllimport)
#endif
#else
#define BATTLEFIELDTESTS_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Called once per executed test. `errors` holds one failure per line (empty when the test passed).
typedef void (*BFT_ResultCallback)(const char* testName, int passed, int checks, const char* errors, void* userData);

// Number of registered tests.
BATTLEFIELDTESTS_API int BFT_GetTestCount(void);

// Full name of a test ("Hellmarch.Battlefield.Grid.NeighbourTest") or NULL if index is out of range.
BATTLEFIELDTESTS_API const char* BFT_GetTestName(int index);

// Runs every test whose name contains `filter` (NULL or "" runs all of them).
// Returns the number of FAILED tests; -1 if the filter matched nothing.
BATTLEFIELDTESTS_API int BFT_RunTests(const char* filter, BFT_ResultCallback callback, void* userData);

// Routes the Battlefield library's internal error log to stderr (0 = silent, the default).
BATTLEFIELDTESTS_API void BFT_SetVerboseLog(int enabled);

#ifdef __cplusplus
}
#endif
