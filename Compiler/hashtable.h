#pragma once




typedef int (*HashEqual)(const void* a, const void* b);
typedef int (*Hasher)(const void* key);
typedef unsigned int HashedKey;

typedef struct HashItem
{
	struct HashItem* next; // next hash item
	const void* key;       // key of this item
	HashedKey key_hash;    // hashed key of this item, for quick look up
	void* payload;         // user's data, i.e. value of the entry
} HashItem;

typedef struct HashTable
{
	HashItem** items;   // hash bins of key-value pairs
	int* collisions;    // collison count for each bin
	int count;          // number of key-value pairs
	int capacity;       // number of bins

	int max_collisions; // if collisions exceeds, a rehash is done
	int payload_size;   // size of the value
	int item_size;      // item_size = payload_size + sizeof(struct HashItem*)
	HashEqual equal;    // function determines 2 keys are equal
	Hasher hasher;      // function that hashes a key
} HashTable;

// Hash 表操作
// =================================

// HashTable<int>:
// payload_size = sizeof(int)
// capacity = any prime number, can be zero
// equal = (a, b)->{ return *(int*)a == *(int*)b; }
HashTable* hash_new(int payload_size, int capacity, HashEqual equal, Hasher hasher);

// returns nullptr
HashTable* hash_destroy(HashTable* table);

// return 0 if success, return -1 if there is no such key in table
int hash_remove(HashTable*table, const void* key);

// 返回 payload
void* hash_add(HashTable* table, const void* key, void* payload);
void* hash_find(HashTable* table, const void* key);


// 预定义的 Hash Table
// ==========================================

// 返回字符串作为 Key 的哈希表
HashTable* hash_new_strkey(int payload_size, int capacity);