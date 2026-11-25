// Pins the PPI formulas and sort keys to the values the original implementation produced.

#include "Team/Team.h"
#include "TestSupport.h"

int main()
{
	Team colts("Indianapolis Colts", 11, 5, 6506, 0.667, 7);
	CHECK_EQ(colts.getPPI(), 4339);
	CHECK_EQ(colts.nameToInt(), static_cast<int>('C'));
	CHECK_EQ(colts.getDiv(), std::string("AFC South"));
	colts.scaledPPI(4);
	CHECK_EQ(colts.getPPI(), 50230254);

	Team browns;
	browns.input("Cleveland Browns", 3, 13, 5000, 0.2, 5);
	browns.scaledPPI(3);
	CHECK_EQ(browns.getPPI(), 599998);
	CHECK_EQ(browns.nameToInt(), static_cast<int>('B'));

	Team edited("Test Team", 8, 8, 5000, 0.5, 1);
	CHECK_EQ(edited.getPPI(), 2500);
	edited.changeVals(TeamEdit::AddYards, 1000);
	CHECK_EQ(edited.getYards(), 6000);
	CHECK_EQ(edited.getPPI(), 3000); // edits recompute the PPI
	edited.changeVals(TeamEdit::SubtractWins, 20);
	CHECK_EQ(edited.getWins(), 0); // stats never go negative
	edited.changeVals(TeamEdit::AddLosses, 2);
	CHECK_EQ(edited.keyOutput('l'), 10);
	CHECK_EQ(edited.keyOutput('y'), 6000);
	CHECK_EQ(edited.keyOutput('?'), 6000); // unknown keys fall back to yards

	Team a("A", 1, 1, 100, 1.0, 1);
	Team b("B", 1, 1, 200, 1.0, 1);
	CHECK(a < b);
	CHECK(b > a);
	CHECK(a != b);
	CHECK(a == Team("A", 1, 1, 100, 1.0, 2)); // division is not part of equality
	return finishTests("TeamTests");
}
