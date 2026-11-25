//
//  HashTableEntry.h
//  HashTable
//
//  Created by Dennis Berko on 10/25/14.
//  Copyright (c) 2014 Dennis. All rights reserved.
//

#pragma once

#include <utility>

// One key/item pair in a HashTable bucket's singly linked chain.
template <class KeyType, class ItemType>
class HashTableEntry
{
public:
	HashTableEntry(KeyType keyIn, ItemType itemIn)
		: key(std::move(keyIn)), item(std::move(itemIn)) {}

	const ItemType& getItem() const { return item; }
	const KeyType& getKey() const { return key; }
	HashTableEntry* getNext() const { return next; }

	void setItem(const ItemType& itemIn) { item = itemIn; }
	void setNext(HashTableEntry* nextIn) { next = nextIn; }

private:
	KeyType key;
	ItemType item;
	HashTableEntry* next = nullptr;
};
