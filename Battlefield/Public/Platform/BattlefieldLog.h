#pragma once
#include "Platform/CoreTypes.h"

// Replacement for UE_LOG(LogBattlefield, ...). The library is silent by default; a host
// application (or the test runner) can install a sink to receive the messages.

enum class ELogVerbosity : uint8
{
	Log = 0,
	Warning,
	Error
};

using FLogSink = void (*)(ELogVerbosity verbosity, const char* message);

class BATTLEFIELD_API FBattlefieldLog
{
public:
	// Pass nullptr to go back to silent mode.
	static void SetSink(FLogSink sink);
	static FLogSink GetSink();

	// printf style formatting; no-op if no sink is installed.
	static void Write(ELogVerbosity verbosity, const char* format, ...);
};

#define BF_LOG(Verbosity, ...) FBattlefieldLog::Write(ELogVerbosity::Verbosity, __VA_ARGS__)
