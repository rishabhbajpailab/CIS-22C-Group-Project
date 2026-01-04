#pragma once

#include <vector>

#include "../Team/Team.h"

std::vector<Team> getDivTeams(std::vector<Team>& teams, int inDiv, int& divScalar);
std::vector<Team> playoffBracket(std::vector<Team> teams);
std::vector<Team> buildPlayoffBracket(std::vector<Team> currConf);
std::vector<Team> buildPlayoffBracket(std::vector<Team> currConf, int AFC);
void getWildCards(std::vector<Team> source, std::vector<Team>& destination);
void reorderVec(std::vector<Team>& teams);
