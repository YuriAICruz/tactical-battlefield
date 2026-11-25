#pragma once

// A tiny, dependency-free replacement for Unreal's automation test framework.
// It provides just enough of the API surface used by the original Battlefield tests
// (IMPLEMENT_SIMPLE_AUTOMATION_TEST, TestEqual/TestTrue/..., TEXT(), FString) so the
// test bodies could be transpiled almost verbatim.

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "Platform/CoreTypes.h"

using FString = std::string;

#ifndef TEXT
#define TEXT(x) x
#endif

namespace EAutomationTestFlags
{
enum Type : unsigned
{
	EditorContext = 1u << 0,
	EngineFilter = 1u << 1,
};
}

namespace BFT
{
template <typename T>
std::string ToDisplayString(const T& value)
{
	using V = std::remove_cv_t<std::remove_reference_t<T>>;
	if constexpr (std::is_same_v<V, bool>)
	{
		return value ? "true" : "false";
	}
	else if constexpr (std::is_enum_v<V>)
	{
		return std::to_string(static_cast<long long>(static_cast<std::underlying_type_t<V>>(value)));
	}
	else if constexpr (std::is_integral_v<V>)
	{
		if constexpr (std::is_signed_v<V>)
		{
			return std::to_string(static_cast<long long>(value));
		}
		else
		{
			return std::to_string(static_cast<unsigned long long>(value));
		}
	}
	else if constexpr (std::is_floating_point_v<V>)
	{
		std::ostringstream stream;
		stream << value;
		return stream.str();
	}
	else if constexpr (std::is_pointer_v<V>)
	{
		std::ostringstream stream;
		stream << static_cast<const void*>(value);
		return stream.str();
	}
	else if constexpr (std::is_convertible_v<V, std::string>)
	{
		return "\"" + std::string(value) + "\"";
	}
	else
	{
		return "<object>";
	}
}
}

class FAutomationTestBase
{
public:
	explicit FAutomationTestBase(std::string name) : mName(std::move(name))
	{
	}

	virtual ~FAutomationTestBase() = default;

	virtual bool RunTest(const FString& Parameters) = 0;

	const std::string& GetTestName() const { return mName; }
	const std::vector<std::string>& GetErrors() const { return mErrors; }
	int GetCheckCount() const { return mChecks; }

	void AddError(const std::string& message)
	{
		mErrors.push_back(message);
	}

	bool TestTrue(const char* what, bool value)
	{
		++mChecks;
		if (!value)
		{
			AddError(std::string(what) + ": expected true, was false");
		}
		return value;
	}

	bool TestFalse(const char* what, bool value)
	{
		++mChecks;
		if (value)
		{
			AddError(std::string(what) + ": expected false, was true");
		}
		return !value;
	}

	template <typename TActual, typename TExpected>
	bool TestEqual(const char* what, const TActual& actual, const TExpected& expected)
	{
		++mChecks;
		const bool equal = (actual == expected);
		if (!equal)
		{
			AddError(std::string(what) + ": expected " + BFT::ToDisplayString(expected) + ", was " +
				BFT::ToDisplayString(actual));
		}
		return equal;
	}

	template <typename TActual, typename TExpected>
	bool TestNotEqual(const char* what, const TActual& actual, const TExpected& expected)
	{
		++mChecks;
		const bool different = !(actual == expected);
		if (!different)
		{
			AddError(std::string(what) + ": expected a value different from " + BFT::ToDisplayString(expected));
		}
		return different;
	}

	template <typename TActual, typename TExpected>
	bool TestGreaterThan(const char* what, const TActual& actual, const TExpected& expected)
	{
		++mChecks;
		const bool greater = actual > expected;
		if (!greater)
		{
			AddError(std::string(what) + ": expected value greater than " + BFT::ToDisplayString(expected) +
				", was " + BFT::ToDisplayString(actual));
		}
		return greater;
	}

	template <typename TPointer>
	bool TestNull(const char* what, const TPointer* pointer)
	{
		++mChecks;
		if (pointer != nullptr)
		{
			AddError(std::string(what) + ": expected null pointer");
		}
		return pointer == nullptr;
	}

	template <typename TPointer>
	bool TestNotNull(const char* what, const TPointer* pointer)
	{
		++mChecks;
		if (pointer == nullptr)
		{
			AddError(std::string(what) + ": expected non-null pointer");
		}
		return pointer != nullptr;
	}

private:
	std::string mName;
	std::vector<std::string> mErrors;
	int mChecks = 0;
};

class FTestRegistry
{
public:
	using FFactory = std::function<std::unique_ptr<FAutomationTestBase>()>;

	struct FEntry
	{
		std::string name;
		FFactory factory;
	};

	static std::vector<FEntry>& Entries()
	{
		static std::vector<FEntry> entries;
		return entries;
	}

	static bool Register(const char* name, FFactory factory)
	{
		Entries().push_back({name, std::move(factory)});
		return true;
	}
};

#define IMPLEMENT_SIMPLE_AUTOMATION_TEST(TClass, PrettyName, TFlags) \
	class TClass : public FAutomationTestBase \
	{ \
	public: \
		TClass() : FAutomationTestBase(PrettyName) {} \
		bool RunTest(const FString& Parameters) override; \
	}; \
	namespace \
	{ \
		const bool TClass##_Registered = FTestRegistry::Register( \
			PrettyName, []() -> std::unique_ptr<FAutomationTestBase> { return std::make_unique<TClass>(); }); \
	}
