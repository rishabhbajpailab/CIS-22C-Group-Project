#include <sstream>
#include <string>

#include "BST/TeamTree.h"
#include "TestSupport.h"

namespace
{
Team team(const std::string& name, int yards)
{
	return Team(name, 8, 8, yards, 0.5, 1);
}

std::string names(const TeamTree& tree)
{
	std::ostringstream out;
	tree.printDescending(out);
	std::string text = out.str();
	std::string result;
	std::istringstream lines(text);
	std::string line;
	while (std::getline(lines, line))
	{
		if (line.rfind("In the", 0) == 0 || line.empty() || line == " ")
		{
			continue;
		}
		result += line.substr(0, line.find(' ')) + ",";
	}
	return result;
}
} // namespace

int main()
{
	TeamTree tree;
	CHECK(tree.treeEmpty());
	const int keys[] = {333, 111, 222, 444, 555, 222};
	const char* labels[] = {"c", "a", "b", "d", "e", "b2"};
	for (int i = 0; i < 6; i++)
	{
		tree.insertNode(keys[i], team(labels[i], keys[i]));
	}
	CHECK_EQ(tree.size(), 6);
	CHECK_EQ(names(tree), std::string("e,d,c,b2,b,a,"));

	CHECK(tree.search(444) != nullptr);
	CHECK(tree.search(999) == nullptr);

	CHECK(tree.deleteValue(555)); // leaf
	CHECK(tree.deleteValue(333)); // root with two children
	CHECK(!tree.deleteValue(999));
	CHECK_EQ(tree.size(), 4);
	CHECK_EQ(names(tree), std::string("d,b2,b,a,"));
	CHECK(tree.search(333) == nullptr);

	CHECK(tree.deleteValue(222));
	CHECK(tree.deleteValue(222)); // duplicates delete one at a time
	CHECK(tree.deleteValue(444));
	CHECK(tree.deleteValue(111));
	CHECK(tree.treeEmpty());
	return finishTests("TeamTreeTests");
}
