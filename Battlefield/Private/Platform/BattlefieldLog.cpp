#include "Platform/BattlefieldLog.h"
#include <atomic>
#include <cstdarg>
#include <cstdio>

namespace
{
std::atomic<FLogSink> gSink{nullptr};
}

void FBattlefieldLog::SetSink(FLogSink sink)
{
	gSink.store(sink);
}

FLogSink FBattlefieldLog::GetSink()
{
	return gSink.load();
}

void FBattlefieldLog::Write(ELogVerbosity verbosity, const char* format, ...)
{
	const FLogSink sink = gSink.load();
	if (!sink)
	{
		return;
	}

	char buffer[512];
	va_list args;
	va_start(args, format);
	std::vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	sink(verbosity, buffer);
}
