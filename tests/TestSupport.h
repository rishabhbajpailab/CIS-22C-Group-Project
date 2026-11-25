#pragma once

// Minimal test helpers: CHECK works in every build type (unlike assert, which NDEBUG disables).

#include <cstdlib>
#include <iostream>

inline int& testFailures()
{
	static int failures = 0;
	return failures;
}

#define CHECK(condition)                                                                    \
	do                                                                                      \
	{                                                                                       \
		if (!(condition))                                                                   \
		{                                                                                   \
			std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #condition "\n"; \
			testFailures()++;                                                               \
		}                                                                                   \
	} while (false)

#define CHECK_EQ(actual, expected)                                                       \
	do                                                                                   \
	{                                                                                    \
		const auto& checkActual = (actual);                                              \
		const auto& checkExpected = (expected);                                          \
		if (!(checkActual == checkExpected))                                             \
		{                                                                                \
			std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK_EQ failed: " #actual    \
					  << " == " << checkActual << ", expected " << checkExpected << "\n"; \
			testFailures()++;                                                            \
		}                                                                                \
	} while (false)

#define CHECK_THROWS(statement, ExceptionType)                                                       \
	do                                                                                               \
	{                                                                                                \
		bool checkThrew = false;                                                                     \
		try                                                                                          \
		{                                                                                            \
			statement;                                                                               \
		}                                                                                            \
		catch (const ExceptionType&)                                                                 \
		{                                                                                            \
			checkThrew = true;                                                                       \
		}                                                                                            \
		if (!checkThrew)                                                                             \
		{                                                                                            \
			std::cerr << __FILE__ << ":" << __LINE__ << ": expected " #ExceptionType " from " #statement "\n"; \
			testFailures()++;                                                                        \
		}                                                                                            \
	} while (false)

inline int finishTests(const char* suite)
{
	if (testFailures() == 0)
	{
		std::cout << suite << " passed." << std::endl;
		return EXIT_SUCCESS;
	}
	std::cerr << suite << ": " << testFailures() << " check(s) failed." << std::endl;
	return EXIT_FAILURE;
}
