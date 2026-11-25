// Perfect Playoff Predictions: NFL playoff bracket predictor with a what-if team editor.

#include <cstring>
#include <iostream>
#include <string>

#include "App.h"
#include "IO/DataIO.h"
#include "UI/Console.h"

namespace
{
void printUsage(const char* program)
{
	std::cout << "Usage: " << program << " [--no-clear] [data-file.csv]\n"
		<< "  data-file.csv  Team data to load (default: " << kDefaultDataFile << ").\n"
		<< "                 If omitted, you are asked whether to use the default file.\n"
		<< "  --no-clear     Do not clear the screen between menus.\n";
}

std::string promptForDataFile()
{
	std::cout << "The default input file is " << kDefaultDataFile
		<< ". Would you like to enter a different data file? [y/N]" << std::endl;
	if (!readYesNo(false))
	{
		return kDefaultDataFile;
	}
	std::cout << "Please enter the data file you would like to use. It must be a .csv file in the same format"
		<< " as the default input (see docs/data-format.md)." << std::endl;
	std::string path = readLine();
	return path.empty() ? kDefaultDataFile : path;
}
} // namespace

int main(int argc, char* argv[])
{
	std::string dataFile;
	for (int i = 1; i < argc; i++)
	{
		if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0)
		{
			printUsage(argv[0]);
			return 0;
		}
		if (std::strcmp(argv[i], "--no-clear") == 0)
		{
			setScreenClearing(false);
		}
		else if (dataFile.empty())
		{
			dataFile = argv[i];
		}
		else
		{
			printUsage(argv[0]);
			return 2;
		}
	}

	try
	{
		clearScreen();
		std::cout << "Welcome to the Perfect Playoff Predictions" << "\n\n"
			<< "Brought to you by Team 2 (also known as The Best Team)" << '\n'
			<< "Playoff brackets are our best approximations, we can't account for heart...yet" << "\n\n";

		if (dataFile.empty())
		{
			dataFile = promptForDataFile();
		}

		App app(dataFile);
		app.run();
	}
	catch (const DataError& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}
	catch (const InputClosed&)
	{
		std::cout << std::endl;
	}
	return 0;
}
