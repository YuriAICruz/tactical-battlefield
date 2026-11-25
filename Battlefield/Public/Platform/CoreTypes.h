#pragma once

// Platform layer: replaces the Unreal "CoreMinimal.h" dependencies of the original module
// (fixed-width aliases, DLL export macro and the standard headers the module relies on).

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
// DLL export / import
//   BATTLEFIELD_EXPORTS : defined while building the Battlefield DLL itself.
//   BATTLEFIELD_STATIC  : defined when the sources are compiled into a static lib / directly.
// ---------------------------------------------------------------------------
#if defined(BATTLEFIELD_STATIC)
#define BATTLEFIELD_API
#elif defined(_WIN32)
#if defined(BATTLEFIELD_EXPORTS)
#define BATTLEFIELD_API __declspec(dllexport)
#else
#define BATTLEFIELD_API __declspec(dllimport)
#endif
#else
#define BATTLEFIELD_API __attribute__((visibility("default")))
#endif

#if defined(_MSC_VER)
// Exported classes expose std:: members (vectors, std::function...). Client and DLL are always built
// with the same toolchain/runtime, so the C4251 warning is noise here.
#pragma warning(disable : 4251)
#endif

// ---------------------------------------------------------------------------
// Fixed-width aliases (same names Unreal uses, so the transpiled code stays readable)
// ---------------------------------------------------------------------------
using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

// Name of the unit in FBattleController::FUnitData (FName in Unreal).
using FName = std::string;
