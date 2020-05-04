#pragma once
typedef struct HashItem
{
	struct HashItem* next;
	char payload[1];
} HashItem;

typedef int (*HashEqual)(const void* a, const void* b);
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
} HashTable;

// HashTable<int>:
// payload_size = sizeof(int)
// capacity = any prime number, can be zero
// equal = (a, b)->{ return *(int*)a == *(int*)b; }
HashTable* hash_new(int payload_size, int capacity, HashEqual equal);

// returns nullptr
HashTable* hash_destroy(HashTable* table);

// return 0 if success, return -1 if there is no such key in table
//int hash_remove(HashTable*table, int key);

void* hash_add(HashTable* table, int key, void* payload);
void* hash_find(HashTable* table, int key);

//int hash_str(const char*);

