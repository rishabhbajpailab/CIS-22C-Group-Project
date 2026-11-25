// Perfect Playoff Predictions: NFL playoff bracket predictor with a what-if team editor.

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "App.h"
#include "IO/DataIO.h"
#include "Nflverse/DataSource.h"
#include "Nflverse/Franchises.h"
#include "Nflverse/NflverseData.h"
#include "UI/Console.h"

namespace
{
enum class Source
{
	Ask,      // no data arguments: ask at startup
	Download, // --download or --season
	File,     // a data file path was given
};

struct Options
{
	Source source = Source::Ask;
	std::string dataFile;
	int season = 0;
	bool downloadOnly = false;
};

void printUsage(const char* program)
{
	std::cout << "Usage: " << program << " [options] [data-file.csv]\n"
		<< "\n"
		<< "With no data options you are asked where to load team data from.\n"
		<< "\n"
		<< "  data-file.csv    Load team data from a file (format: docs/data-format.md).\n"
		<< "  --download       Download the latest season from nflverse. If the download fails,\n"
		<< "                   the last downloaded copy (data/nflverse-<season>.csv) or\n"
		<< "                   " << kDefaultDataFile << " is used.\n"
		<< "  --season YEAR    Download a specific season (" << kFirstSupportedSeason << " or later).\n"
		<< "  --download-only  Download (latest, or --season) to data/nflverse-<season>.csv and exit.\n"
		<< "  --no-clear       Do not clear the screen between menus.\n"
		<< "  -h, --help       Show this help.\n";
}

bool parseSeason(const char* text, int& season)
{
	char* end = nullptr;
	long value = std::strtol(text, &end, 10);
	if (end == text || *end != '\0' || value < kFirstSupportedSeason || value > currentSeason())
	{
		return false;
	}
	season = static_cast<int>(value);
	return true;
}

// Returns false (after printing usage) if the arguments are invalid.
bool parseArguments(int argc, char* argv[], Options& options, int& exitCode)
{
	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0)
		{
			printUsage(argv[0]);
			exitCode = 0;
			return false;
		}
		if (std::strcmp(arg, "--no-clear") == 0)
		{
			setScreenClearing(false);
		}
		else if (std::strcmp(arg, "--download") == 0)
		{
			options.source = Source::Download;
		}
		else if (std::strcmp(arg, "--download-only") == 0)
		{
			options.source = Source::Download;
			options.downloadOnly = true;
		}
		else if (std::strcmp(arg, "--season") == 0)
		{
			if (i + 1 >= argc || !parseSeason(argv[i + 1], options.season))
			{
				std::cerr << "--season needs a year from " << kFirstSupportedSeason << " to " << currentSeason() << ".\n";
				exitCode = 2;
				return false;
			}
			options.source = Source::Download;
			i++;
		}
		else if (arg[0] != '-' && options.dataFile.empty())
		{
			options.dataFile = arg;
		}
		else
		{
			std::cerr << "Unrecognized argument: " << arg << "\n\n";
			printUsage(argv[0]);
			exitCode = 2;
			return false;
		}
	}
	if (!options.dataFile.empty())
	{
		if (options.source == Source::Download)
		{
			std::cerr << "Give either a data file or a download option, not both.\n";
			exitCode = 2;
			return false;
		}
		options.source = Source::File;
	}
	return true;
}

// Asks where to load data from and fills in options.
void promptForSource(Options& options)
{
	std::cout << "Where should the team data come from?" << '\n'
		<< "1) Download the latest season from nflverse (default)" << '\n'
		<< "2) Use the bundled " << kDefaultDataFile << " (2014 season)" << '\n'
		<< "3) Enter another data file" << std::endl;
	switch (readIntOrDefault(1, 3, 1))
	{
	case 1:
		options.source = Source::Download;
		break;
	case 2:
		options.source = Source::File;
		options.dataFile = kDefaultDataFile;
		break;
	default:
		std::cout << "Please enter the data file you would like to use. It must be a .csv file in the format"
			<< " described in docs/data-format.md." << std::endl;
		options.source = Source::File;
		options.dataFile = readLine();
		if (options.dataFile.empty())
		{
			options.dataFile = kDefaultDataFile;
		}
		break;
	}
}

int downloadOnly(const DownloadOptions& downloads)
{
	try
	{
		League league = downloadLeague(downloads);
		std::cout << "Saved " << league.label << " to " << cachePathForSeason(league.season) << std::endl;
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}
}
} // namespace

int main(int argc, char* argv[])
{
	Options options;
	int exitCode = 0;
	if (!parseArguments(argc, argv, options, exitCode))
	{
		return exitCode;
	}

	DownloadOptions downloads;
	downloads.season = options.season;
	if (options.downloadOnly)
	{
		return downloadOnly(downloads);
	}

	try
	{
		clearScreen();
		std::cout << "Welcome to the Perfect Playoff Predictions" << "\n\n"
			<< "Brought to you by Team 2 (also known as The Best Team)" << '\n'
			<< "Playoff brackets are our best approximations, we can't account for heart...yet" << "\n\n";

		if (options.source == Source::Ask)
		{
			promptForSource(options);
		}

		League league;
		if (options.source == Source::Download)
		{
			std::cout << "Downloading team data from nflverse..." << std::endl;
			league = downloadLeagueOrFallback(downloads, kDefaultDataFile);
		}
		else
		{
			league = loadLeagueFile(options.dataFile);
		}

		App app(std::move(league), downloads);
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
