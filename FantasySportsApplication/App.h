#pragma once

#include <string>
#include <vector>

#include "HashTable/HashTable.h"
#include "Nflverse/DataSource.h"
#include "Team/Team.h"

// Interactive command-line front end for the playoff predictor.
class App
{
public:
	// downloads configures the "Download Latest Data" menu option.
	App(League league, DownloadOptions downloads);

	// Runs the main menu until the user quits.
	void run();

private:
	enum MainMenuChoice
	{
		EditTeams = 1,
		AddRemoveTeam,
		DisplayTeams,
		ShowPlayoffs,
		ShowEfficiency,
		DownloadData,
		Quit,
	};

	int mainMenu() const;
	void editTeams();
	void addRemoveTeam() const;
	void displayTeams() const;
	void showPlayoffs() const;
	void showEfficiency() const;
	void downloadData();
	bool confirmQuit() const;

	void listTeams() const;
	void rebuildHashTable();
	void setLeague(League league);

	DownloadOptions downloadOptions;
	std::string dataLabel;
	std::vector<Team> teams;
	HashTable<int, Team> hashTable; // keyed by total yards
};
