// Minimal console host for BattlefieldTests.dll.
//   BattlefieldTestRunner [--filter <text>] [--list] [--verbose] [--quiet]
// Exit code = number of failed tests (0 = success), 2 = nothing matched the filter.

#include "BattlefieldTests.h"

#include <cstdio>
#include <cstring>

namespace
{
struct FState
{
	int passed = 0;
	int failed = 0;
	long checks = 0;
	bool quiet = false;
};

void OnResult(const char* name, int passed, int checks, const char* errors, void* userData)
{
	FState* state = static_cast<FState*>(userData);
	state->checks += checks;
	if (passed)
	{
		++state->passed;
		if (!state->quiet)
		{
			std::printf("[ PASS ] %s\n", name);
		}
		return;
	}

	++state->failed;
	std::printf("[ FAIL ] %s\n", name);
	for (const char* line = errors; *line;)
	{
		const char* end = std::strchr(line, '\n');
		const int length = end ? static_cast<int>(end - line) : static_cast<int>(std::strlen(line));
		std::printf("         %.*s\n", length, line);
		line += length + (end ? 1 : 0);
	}
}
}

int main(int argc, char** argv)
{
	const char* filter = nullptr;
	bool list = false;
	FState state;

	for (int i = 1; i < argc; ++i)
	{
		if (std::strcmp(argv[i], "--filter") == 0 && i + 1 < argc)
		{
			filter = argv[++i];
		}
		else if (std::strcmp(argv[i], "--list") == 0)
		{
			list = true;
		}
		else if (std::strcmp(argv[i], "--verbose") == 0)
		{
			BFT_SetVerboseLog(1);
		}
		else if (std::strcmp(argv[i], "--quiet") == 0)
		{
			state.quiet = true;
		}
		else
		{
			std::printf("usage: %s [--filter <text>] [--list] [--verbose] [--quiet]\n", argv[0]);
			return 1;
		}
	}

	if (list)
	{
		for (int i = 0, n = BFT_GetTestCount(); i < n; ++i)
		{
			std::printf("%s\n", BFT_GetTestName(i));
		}
		return 0;
	}

	const int result = BFT_RunTests(filter, &OnResult, &state);
	if (result < 0)
	{
		std::printf("No test matches filter '%s'\n", filter ? filter : "");
		return 2;
	}

	std::printf("\n%d tests, %d passed, %d failed (%ld checks)\n", state.passed + state.failed, state.passed,
	            state.failed, state.checks);
	return state.failed;
}
