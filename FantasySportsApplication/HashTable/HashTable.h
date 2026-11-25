//
//  HashTable.h
//  HashTable
//
//  Created by Dennis Berko on 10/20/14.
//  Copyright (c) 2014 Dennis. All rights reserved.
//

#pragma once

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

#include "HashTableEntry.h"

// Hash table using separate chaining. Bucket index is std::hash<KeyType>(key) % bucket count.
template <class KeyType, class ItemType>
class HashTable
{
public:
	class NotFoundException {};

	static constexpr int DEFAULT_SIZE = 32;

	HashTable() : HashTable(DEFAULT_SIZE) {}
	explicit HashTable(int tableSize);
	HashTable(const HashTable& other);
	HashTable& operator=(HashTable other);
	~HashTable() { clear(); }

	// Inserts a new entry, or replaces the item if the key is already present.
	bool add(const KeyType& key, const ItemType& item);
	// Removes the entry with this key. Returns false if it was not present.
	bool remove(const KeyType& key);
	// Returns the item for this key, or throws NotFoundException.
	ItemType get(const KeyType& key) const;

	bool isEmpty() const { return itemCount == 0; }
	int size() const { return itemCount; }
	int bucketCount() const { return static_cast<int>(buckets.size()); }
	// Number of buckets holding at least one entry.
	int occupiedBuckets() const;
	// Entries per bucket (itemCount / bucketCount).
	double loadFactor() const;

	void clear();
	bool contains(const KeyType& key) const;

	// Calls visit(item) for every entry.
	template <class Visitor>
	void traverse(Visitor visit) const;

	friend void swap(HashTable& a, HashTable& b) noexcept
	{
		using std::swap;
		swap(a.buckets, b.buckets);
		swap(a.itemCount, b.itemCount);
	}

private:
	using Entry = HashTableEntry<KeyType, ItemType>;

	int getHashIndex(const KeyType& key) const;
	Entry* find(const KeyType& key) const;

	std::vector<Entry*> buckets;
	int itemCount = 0;
};

template <class KeyType, class ItemType>
HashTable<KeyType, ItemType>::HashTable(int tableSize)
	: buckets(static_cast<std::size_t>(tableSize > 0 ? tableSize : DEFAULT_SIZE), nullptr)
{
}

template <class KeyType, class ItemType>
HashTable<KeyType, ItemType>::HashTable(const HashTable& other)
	: buckets(other.buckets.size(), nullptr), itemCount(other.itemCount)
{
	// Copy each chain, preserving order.
	for (std::size_t i = 0; i < other.buckets.size(); i++)
	{
		Entry* tail = nullptr;
		for (Entry* src = other.buckets[i]; src != nullptr; src = src->getNext())
		{
			Entry* entry = new Entry(src->getKey(), src->getItem());
			if (tail == nullptr)
			{
				buckets[i] = entry;
			}
			else
			{
				tail->setNext(entry);
			}
			tail = entry;
		}
	}
}

template <class KeyType, class ItemType>
HashTable<KeyType, ItemType>& HashTable<KeyType, ItemType>::operator=(HashTable other)
{
	swap(*this, other);
	return *this;
}

template <class KeyType, class ItemType>
bool HashTable<KeyType, ItemType>::add(const KeyType& key, const ItemType& item)
{
	if (Entry* existing = find(key))
	{
		existing->setItem(item);
		return true;
	}

	// New keys are pushed onto the front of their bucket's chain.
	int index = getHashIndex(key);
	Entry* entry = new Entry(key, item);
	entry->setNext(buckets[index]);
	buckets[index] = entry;
	itemCount++;
	return true;
}

template <class KeyType, class ItemType>
bool HashTable<KeyType, ItemType>::remove(const KeyType& key)
{
	int index = getHashIndex(key);
	Entry* prev = nullptr;
	for (Entry* cur = buckets[index]; cur != nullptr; prev = cur, cur = cur->getNext())
	{
		if (cur->getKey() == key)
		{
			if (prev == nullptr)
			{
				buckets[index] = cur->getNext();
			}
			else
			{
				prev->setNext(cur->getNext());
			}
			delete cur;
			itemCount--;
			return true;
		}
	}
	return false;
}

template <class KeyType, class ItemType>
ItemType HashTable<KeyType, ItemType>::get(const KeyType& key) const
{
	if (Entry* entry = find(key))
	{
		return entry->getItem();
	}
	throw NotFoundException();
}

template <class KeyType, class ItemType>
int HashTable<KeyType, ItemType>::occupiedBuckets() const
{
	int occupied = 0;
	for (Entry* head : buckets)
	{
		if (head != nullptr)
		{
			occupied++;
		}
	}
	return occupied;
}

template <class KeyType, class ItemType>
double HashTable<KeyType, ItemType>::loadFactor() const
{
	return buckets.empty() ? 0.0 : static_cast<double>(itemCount) / static_cast<double>(buckets.size());
}

template <class KeyType, class ItemType>
void HashTable<KeyType, ItemType>::clear()
{
	for (Entry*& head : buckets)
	{
		while (head != nullptr)
		{
			Entry* next = head->getNext();
			delete head;
			head = next;
		}
	}
	itemCount = 0;
}

template <class KeyType, class ItemType>
bool HashTable<KeyType, ItemType>::contains(const KeyType& key) const
{
	return find(key) != nullptr;
}

template <class KeyType, class ItemType>
template <class Visitor>
void HashTable<KeyType, ItemType>::traverse(Visitor visit) const
{
	for (Entry* head : buckets)
	{
		for (Entry* cur = head; cur != nullptr; cur = cur->getNext())
		{
			visit(cur->getItem());
		}
	}
}

template <class KeyType, class ItemType>
int HashTable<KeyType, ItemType>::getHashIndex(const KeyType& key) const
{
	std::hash<KeyType> keyHash;
	return static_cast<int>(keyHash(key) % buckets.size());
}

template <class KeyType, class ItemType>
typename HashTable<KeyType, ItemType>::Entry* HashTable<KeyType, ItemType>::find(const KeyType& key) const
{
	for (Entry* cur = buckets[getHashIndex(key)]; cur != nullptr; cur = cur->getNext())
	{
		if (cur->getKey() == key)
		{
			return cur;
		}
	}
	return nullptr;
}
