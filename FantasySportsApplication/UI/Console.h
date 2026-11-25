#pragma once

#include <stdexcept>
#include <string>

// Thrown when standard input is closed (Ctrl+D / Ctrl+Z or end of a piped script).
class InputClosed : public std::runtime_error
{
public:
	InputClosed() : std::runtime_error("input closed") {}
};

// When false, clearScreen() does nothing (useful for scripted runs and logs).
void setScreenClearing(bool enabled);
void clearScreen();

// Waits for the user to press Enter.
void pauseScreen();

// Reads one line of input. Throws InputClosed at end of input.
std::string readLine();

// Prompts until the user enters a whole number in [minValue, maxValue].
int readInt(int minValue, int maxValue);

// Prompts until the user enters y/yes or n/no (case-insensitive).
// An empty answer returns defaultAnswer.
bool readYesNo(bool defaultAnswer);
