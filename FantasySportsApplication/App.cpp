#include "App.h"

#include <iomanip>
#include <iostream>
#include <utility>

#include "BST/TeamTree.h"
#include "IO/DataIO.h"
#include "Nflverse/Franchises.h"
#include "Nflverse/NflverseData.h"
#include "Playoff/PlayoffLogic.h"
#include "UI/Console.h"

namespace
{
const char* const kRule = "******************************************************************************************";

struct SortOption
{
	const char* label;
	char key;
};

const SortOption kSortOptions[] = {
	{"Yards", 'y'},
	{"Wins", 'w'},
	{"Losses", 'l'},
	{"PPI", 'p'},
	{"Name", 'n'},
};
constexpr int kSortOptionCount = static_cast<int>(sizeof(kSortOptions) / sizeof(kSortOptions[0]));

void printBoxLine(const std::string& text)
{
	const int width = 86;
	std::string padded = text.size() > static_cast<std::size_t>(width) ? text.substr(0, width) : text;
	int left = (width - static_cast<int>(padded.size())) / 2;
	int right = width - static_cast<int>(padded.size()) - left;
	std::cout << "**" << std::string(left + 1, ' ') << padded << std::string(right + 1, ' ') << "**" << '\n';
}
} // namespace

App::App(League league, DownloadOptions downloads)
	: downloadOptions(std::move(downloads))
{
	setLeague(std::move(league));
}

void App::setLeague(League league)
{
	dataLabel = std::move(league.label);
	teams = std::move(league.teams);
	rebuildHashTable();
}

void App::rebuildHashTable()
{
	hashTable.clear();
	for (const Team& team : teams)
	{
		hashTable.add(team.keyOutput('y'), team);
	}
}

void App::run()
{
	while (true)
	{
		switch (mainMenu())
		{
		case EditTeams: editTeams(); break;
		case AddRemoveTeam: addRemoveTeam(); break;
		case DisplayTeams: displayTeams(); break;
		case ShowPlayoffs: showPlayoffs(); break;
		case ShowEfficiency: showEfficiency(); break;
		case DownloadData: downloadData(); break;
		case Quit:
			if (confirmQuit())
			{
				std::cout << "Goodbye" << std::endl;
				return;
			}
			break;
		}
	}
}

int App::mainMenu() const
{
	clearScreen();
	std::cout << std::setw(15) << "PPP Main Menu" << '\n'
		<< "1) Edit Current Teams" << '\n'
		<< "2) Add or Remove a Team" << '\n'
		<< "3) Display Teams" << '\n'
		<< "4) Who Will Make The Playoffs?" << '\n'
		<< "5) Efficiency" << '\n'
		<< "6) Download Latest Data (nflverse)" << '\n'
		<< "7) Quit" << '\n'
		<< "Current data: " << dataLabel << std::endl;
	return readInt(EditTeams, Quit);
}

void App::listTeams() const
{
	for (std::size_t i = 0; i < teams.size(); i++)
	{
		std::cout << i + 1 << ")  " << teams[i] << '\n';
	}
	std::cout << std::flush;
}

void App::editTeams()
{
	clearScreen();
	listTeams();
	std::cout << "Please enter the team you want to edit (0 to go back)" << std::endl;
	int choice = readInt(0, static_cast<int>(teams.size()));
	if (choice == 0)
	{
		return;
	}
	Team& team = teams[static_cast<std::size_t>(choice - 1)];

	bool again = true;
	while (again)
	{
		clearScreen();
		std::cout << std::setw(15) << "Edit Current Teams" << '\n'
			<< team << '\n'
			<< "Please select an operation to perform on the selected team" << '\n'
			<< "1) Add Yards" << '\n'
			<< "2) Sub Yards" << '\n'
			<< "3) Add Wins" << '\n'
			<< "4) Sub Wins" << '\n'
			<< "5) Add Losses" << '\n'
			<< "6) Sub Losses" << std::endl;
		auto edit = static_cast<TeamEdit>(readInt(1, 6));

		std::cout << "Please enter how much you want to change the value" << std::endl;
		int amount = readInt(0, 100000);
		team.changeVals(edit, amount);

		std::cout << "Updated: " << team << '\n'
			<< "Do you want to change another value? [y/N]" << std::endl;
		again = readYesNo(false);
	}
	rebuildHashTable();
}

void App::addRemoveTeam() const
{
	clearScreen();
	std::cout << std::setw(15) << "Add/Remove Team Menu" << '\n'
		<< "Adding and removing teams is not available yet." << '\n'
		<< "The playoff format needs exactly 4 teams in each of the 8 divisions, so a team" << '\n'
		<< "must be removed for every team added. To change the league today, edit the data file." << std::endl;
	pauseScreen();
}

void App::displayTeams() const
{
	clearScreen();
	std::cout << "Do you want to order the output? [y/N]" << std::endl;
	if (!readYesNo(false))
	{
		for (const Team& team : teams)
		{
			std::cout << team << '\n';
		}
		pauseScreen();
		return;
	}

	std::cout << "Please select which value you would like to sort by (0 to go back)" << '\n';
	for (int i = 0; i < kSortOptionCount; i++)
	{
		std::cout << i + 1 << ")" << kSortOptions[i].label << '\n';
	}
	std::cout << std::flush;
	int choice = readInt(0, kSortOptionCount);
	if (choice == 0)
	{
		return;
	}

	char key = kSortOptions[choice - 1].key;
	TeamTree tree;
	for (const Team& team : teams)
	{
		tree.insertNode(team.keyOutput(key), team);
	}
	clearScreen();
	std::cout << "Teams by " << kSortOptions[choice - 1].label << " (highest first)" << "\n\n";
	tree.printDescending(std::cout);
	pauseScreen();
}

void App::showPlayoffs() const
{
	std::vector<Team> bracket = playoffBracket(teams);

	clearScreen();
	std::cout << kRule << '\n';
	printBoxLine("NFL PLAYOFFS");
	printBoxLine(dataLabel);
	std::cout << kRule << '\n';
	printBoxLine("NFC");
	std::cout << kRule << '\n';
	for (int i = 0; i < kSeedsPerConference; i++)
	{
		std::cout << "**" << bracket[static_cast<std::size_t>(i)].playoffOutput(i + 1) << '\n';
	}
	std::cout << kRule << '\n';
	printBoxLine("AFC");
	std::cout << kRule << '\n';
	for (int i = 0; i < kSeedsPerConference; i++)
	{
		std::cout << "**" << bracket[static_cast<std::size_t>(kSeedsPerConference + i)].playoffOutput(i + 1) << '\n';
	}
	std::cout << kRule << std::endl;
	pauseScreen();
}

void App::showEfficiency() const
{
	clearScreen();
	std::cout << std::fixed << std::setprecision(2)
		<< "The hash table (keyed by total yards) holds " << hashTable.size() << " teams in "
		<< hashTable.bucketCount() << " buckets (" << hashTable.occupiedBuckets() << " in use)." << '\n'
		<< "Load factor: " << hashTable.loadFactor() << '\n';
	std::cout.unsetf(std::ios::floatfield);
	if (hashTable.size() < static_cast<int>(teams.size()))
	{
		std::cout << "Note: " << teams.size() - static_cast<std::size_t>(hashTable.size())
			<< " team(s) share a yards total with another team and replaced it in the table." << '\n';
	}
	std::cout << "With a fixed 32-team league and 32 buckets, the load factor is 1.00 unless teams share a yards total."
		<< std::endl;
	pauseScreen();
}

void App::downloadData()
{
	clearScreen();
	const int season = currentSeason();
	std::cout << "Download team records and yards from nflverse." << '\n'
		<< "Enter a season from " << kFirstSupportedSeason << " to " << season
		<< ", or press Enter for the latest season (0 to go back)." << std::endl;
	int choice = readIntOrDefault(0, season, -1);
	if (choice == 0)
	{
		return;
	}

	DownloadOptions options = downloadOptions;
	options.season = choice < 0 ? 0 : choice;
	std::cout << "Downloading..." << std::endl;
	try
	{
		setLeague(downloadLeague(options));
		std::cout << "Loaded " << dataLabel << ". Any edits you made to the previous data were replaced." << std::endl;
	}
	catch (const std::exception& error)
	{
		std::cerr << "Download failed: " << error.what() << '\n'
			<< "Keeping the current data (" << dataLabel << ")." << std::endl;
	}
	pauseScreen();
}

bool App::confirmQuit() const
{
	std::cout << "Are you sure you want to exit? [Y/n]" << std::endl;
	if (!readYesNo(true))
	{
		return false;
	}
	std::cout << "Do you want to save the current teams to " << kDefaultReportFile << "? [y/N]" << std::endl;
	if (readYesNo(false))
	{
		try
		{
			writeReport(teams, kDefaultReportFile);
			std::cout << "Team data saved to " << kDefaultReportFile << std::endl;
		}
		catch (const DataError& error)
		{
			std::cerr << "Error: " << error.what() << std::endl;
		}
	}
	return true;
}
