#pragma once

#include <iosfwd>
#include <utility>

#include "../Team/Team.h"

// Binary search tree of teams keyed by an integer (yards, wins, PPI, ...).
// Duplicate keys are allowed and are placed in the right subtree.
class TeamTree
{
public:
	struct Node
	{
		Node(Team dataIn, int keyIn) : data(std::move(dataIn)), key(keyIn) {}

		Team data;
		int key;
		Node* lchild = nullptr;
		Node* rchild = nullptr;
	};

	TeamTree() = default;
	~TeamTree();

	TeamTree(const TeamTree&) = delete;
	TeamTree& operator=(const TeamTree&) = delete;

	bool treeEmpty() const { return root == nullptr; }
	int size() const { return count; }

	void insertNode(int key, const Team& dataIn);
	Node* search(int key) const;
	bool deleteValue(int key);

	// Prints teams from the highest key to the lowest (reverse in-order traversal).
	void printDescending(std::ostream& os) const;

private:
	static void printDescending(const Node* node, std::ostream& os);
	static void destroy(Node* node);
	static Node* minValue(Node* node);
	Node* parent(const Node* child) const;
	Node* postNode(Node* node) const;

	Node* root = nullptr;
	int count = 0;
};
