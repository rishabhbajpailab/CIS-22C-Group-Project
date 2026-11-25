#include "TeamTree.h"

#include <ostream>

TeamTree::~TeamTree()
{
	destroy(root);
}

void TeamTree::destroy(Node* node)
{
	if (node == nullptr)
	{
		return;
	}
	destroy(node->lchild);
	destroy(node->rchild);
	delete node;
}

void TeamTree::insertNode(int key, const Team& dataIn)
{
	Node* p = root;
	Node* q = nullptr;
	while (p != nullptr)
	{
		q = p;
		p = (key < p->key) ? p->lchild : p->rchild;
	}

	Node* node = new Node(dataIn, key);
	if (q == nullptr)
	{
		root = node;
	}
	else if (key < q->key)
	{
		q->lchild = node;
	}
	else
	{
		q->rchild = node;
	}
	count++;
}

TeamTree::Node* TeamTree::search(int key) const
{
	Node* p = root;
	while (p != nullptr && p->key != key)
	{
		p = (key < p->key) ? p->lchild : p->rchild;
	}
	return p;
}

void TeamTree::printDescending(std::ostream& os) const
{
	printDescending(root, os);
}

void TeamTree::printDescending(const Node* node, std::ostream& os)
{
	if (node != nullptr)
	{
		printDescending(node->rchild, os);
		os << node->data << " " << '\n';
		printDescending(node->lchild, os);
	}
}

TeamTree::Node* TeamTree::parent(const Node* child) const
{
	// Walk the same path insertNode used, comparing node identity so duplicate keys resolve correctly.
	Node* p = root;
	Node* q = nullptr;
	while (p != nullptr && p != child)
	{
		q = p;
		p = (child->key < p->key) ? p->lchild : p->rchild;
	}
	return q;
}

TeamTree::Node* TeamTree::minValue(Node* node)
{
	Node* p = node;
	while (p->lchild != nullptr)
	{
		p = p->lchild;
	}
	return p;
}

TeamTree::Node* TeamTree::postNode(Node* node) const
{
	if (node->rchild != nullptr)
	{
		return minValue(node->rchild);
	}
	Node* p = node;
	Node* par = parent(p);
	while (par != nullptr && par->rchild == p)
	{
		p = par;
		par = parent(p);
	}
	return par;
}

bool TeamTree::deleteValue(int key)
{
	Node* p = search(key);
	if (p == nullptr)
	{
		return false;
	}

	// q is the node that is physically unlinked: p itself, or its in-order successor.
	Node* q = (p->lchild == nullptr || p->rchild == nullptr) ? p : postNode(p);
	Node* child = (q->lchild != nullptr) ? q->lchild : q->rchild;
	Node* s = parent(q);

	if (s == nullptr)
	{
		root = child;
	}
	else if (s->lchild == q)
	{
		s->lchild = child;
	}
	else
	{
		s->rchild = child;
	}

	if (q != p)
	{
		p->key = q->key;
		p->data = q->data;
	}
	delete q;
	count--;
	return true;
}
