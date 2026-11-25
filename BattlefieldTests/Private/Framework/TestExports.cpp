#include "BattlefieldTests.h"
#include "Framework/AutomationShim.h"
#include "Platform/BattlefieldLog.h"

#include <algorithm>
#include <cstdio>
#include <exception>
#include <string>

namespace
{
void StderrSink(ELogVerbosity verbosity, const char* message)
{
	const char* label = verbosity == ELogVerbosity::Error ? "Error" : verbosity == ELogVerbosity::Warning ? "Warning" : "Log";
	std::fprintf(stderr, "[Battlefield:%s] %s\n", label, message);
}

// Registration order depends on static-init order across translation units; sort for stable output.
std::vector<const FTestRegistry::FEntry*> SortedEntries()
{
	std::vector<const FTestRegistry::FEntry*> sorted;
	for (const FTestRegistry::FEntry& entry : FTestRegistry::Entries())
	{
		sorted.push_back(&entry);
	}
	std::sort(sorted.begin(), sorted.end(),
	          [](const FTestRegistry::FEntry* a, const FTestRegistry::FEntry* b) { return a->name < b->name; });
	return sorted;
}
}

extern "C"
{
BATTLEFIELDTESTS_API int BFT_GetTestCount(void)
{
	return static_cast<int>(FTestRegistry::Entries().size());
}

BATTLEFIELDTESTS_API const char* BFT_GetTestName(int index)
{
	const auto sorted = SortedEntries();
	if (index < 0 || index >= static_cast<int>(sorted.size()))
	{
		return nullptr;
	}
	return sorted[static_cast<size_t>(index)]->name.c_str();
}

BATTLEFIELDTESTS_API int BFT_RunTests(const char* filter, BFT_ResultCallback callback, void* userData)
{
	const std::string needle = filter ? filter : "";
	int matched = 0;
	int failed = 0;

	for (const FTestRegistry::FEntry* entry : SortedEntries())
	{
		if (!needle.empty() && entry->name.find(needle) == std::string::npos)
		{
			continue;
		}
		++matched;

		std::unique_ptr<FAutomationTestBase> test = entry->factory();
		bool passed = false;
		try
		{
			passed = test->RunTest(FString());
		}
		catch (const std::exception& e)
		{
			test->AddError(std::string("unhandled exception: ") + e.what());
		}
		catch (...)
		{
			test->AddError("unhandled unknown exception");
		}

		if (test->GetErrors().empty() && !passed)
		{
			test->AddError("RunTest returned false");
		}
		passed = passed && test->GetErrors().empty();
		if (!passed)
		{
			++failed;
		}

		if (callback)
		{
			std::string errors;
			for (const std::string& error : test->GetErrors())
			{
				errors += error;
				errors += '\n';
			}
			callback(entry->name.c_str(), passed ? 1 : 0, test->GetCheckCount(), errors.c_str(), userData);
		}
	}

	return matched == 0 ? -1 : failed;
}

BATTLEFIELDTESTS_API void BFT_SetVerboseLog(int enabled)
{
	FBattlefieldLog::SetSink(enabled ? &StderrSink : nullptr);
}
}
