#include "Console.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

namespace
{
bool screenClearing = true;

void enableAnsiOnce()
{
#ifdef _WIN32
	static bool done = false;
	if (!done)
	{
		done = true;
		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mode = 0;
		if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode))
		{
			SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
	}
#endif
}

std::string lowerTrimmed(std::string text)
{
	auto notSpace = [](unsigned char c) { return !std::isspace(c); };
	text.erase(text.begin(), std::find_if(text.begin(), text.end(), notSpace));
	text.erase(std::find_if(text.rbegin(), text.rend(), notSpace).base(), text.end());
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return text;
}
} // namespace

void setScreenClearing(bool enabled)
{
	screenClearing = enabled;
}

void clearScreen()
{
	if (!screenClearing)
	{
		return;
	}
	enableAnsiOnce();
	// ANSI clear + home. Supported by modern terminals, including Windows Terminal and Windows 10+ consoles.
	std::cout << "\033[2J\033[H" << std::flush;
}

void pauseScreen()
{
	std::cout << "Press Enter to continue..." << std::endl;
	readLine();
}

std::string readLine()
{
	std::string line;
	if (!std::getline(std::cin, line))
	{
		throw InputClosed();
	}
	if (!line.empty() && line.back() == '\r')
	{
		line.pop_back();
	}
	return line;
}

namespace
{
int readIntImpl(int minValue, int maxValue, const int* defaultValue)
{
	while (true)
	{
		std::string line = readLine();
		if (defaultValue != nullptr && lowerTrimmed(line).empty())
		{
			return *defaultValue;
		}
		std::istringstream stream(line);
		int value = 0;
		char extra = 0;
		if (stream >> value && !(stream >> extra) && value >= minValue && value <= maxValue)
		{
			return value;
		}
		std::cout << "Please enter a number between " << minValue << " and " << maxValue << "." << std::endl;
	}
}
} // namespace

int readInt(int minValue, int maxValue)
{
	return readIntImpl(minValue, maxValue, nullptr);
}

int readIntOrDefault(int minValue, int maxValue, int defaultValue)
{
	return readIntImpl(minValue, maxValue, &defaultValue);
}

bool readYesNo(bool defaultAnswer)
{
	while (true)
	{
		std::string answer = lowerTrimmed(readLine());
		if (answer.empty())
		{
			return defaultAnswer;
		}
		if (answer == "y" || answer == "yes")
		{
			return true;
		}
		if (answer == "n" || answer == "no")
		{
			return false;
		}
		std::cout << "Please answer y or n." << std::endl;
	}
}
