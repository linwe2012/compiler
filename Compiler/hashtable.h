#pragma once




typedef int (*HashEqual)(const void* a, const void* b);
typedef int (*Hasher)(const void* key);
typedef unsigned int HashedKey;

typedef struct HashItem
{
	struct HashItem* next;
	const void* key;
	HashedKey key_hash;
	char payload[1];
} HashItem;

typedef struct HashTable
{
	HashItem** items;
	int* collisions;
	int count;
	int capacity;

	int max_collisions; // if collisions exceeds, a rehash is done
	int payload_size;
	int item_size; // item_size = payload_size + sizeof(struct HashItem*)
	HashEqual equal;
	Hasher hasher;
} HashTable;

// Hash 表操作
// =================================

// HashTable<int>:
// payload_size = sizeof(int)
// capacity = any prime number, can be zero
// equal = (a, b)->{ return *(int*)a == *(int*)b; }
HashTable* hash_new(int payload_size, int capacity, HashEqual equal, Hasher* hasher);

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