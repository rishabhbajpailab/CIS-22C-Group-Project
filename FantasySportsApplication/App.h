#pragma once

#include <string>
#include <vector>

#include "HashTable/HashTable.h"
#include "Team/Team.h"

// Interactive command-line front end for the playoff predictor.
class App
{
public:
	// Loads the league from dataFile. Throws DataError if the file is missing or invalid.
	explicit App(const std::string& dataFile);

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
		Quit,
	};

	int mainMenu() const;
	void editTeams();
	void addRemoveTeam() const;
	void displayTeams() const;
	void showPlayoffs() const;
	void showEfficiency() const;
	bool confirmQuit() const;

	void listTeams() const;
	void rebuildHashTable();

	std::string dataLabel;
	std::vector<Team> teams;
	HashTable<int, Team> hashTable; // keyed by total yards
};
