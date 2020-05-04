#include "alloc.h"
#include "hashtable.h"
#include <string.h>

HashTable* hash_new(int payload_size, int capacity, HashEqual equal)
{
	int item_size = payload_size + sizeof(void*);
	HashTable* tbl = allocate(item_size);

	if (capacity <= 0)
	{
		capacity = 37;
	}

	tbl->capacity = capacity;
	tbl->item_size = item_size;
	tbl->payload_size = payload_size;
	tbl->count = 0;
	tbl->items = allocate(sizeof(HashItem*) * capacity);
	tbl->collisions = allocate(sizeof(int) * capacity);
	tbl->equal = equal;
	tbl->max_collisions = 5;
	return tbl;
}

HashTable* hash_destroy(HashTable* table)
{
	deallocate(table->items);
	return NULL;
}


void hash_enlarge(HashTable* table) {}
HashItem* hash_find_item(HashTable* table, int key) { return NULL; }

void* hash_add(HashTable* table, int key, void* payload)
{
	
	HashItem* item = allocate(table->item_size);
	memcpy(item->payload, payload, table->payload_size);

	int pos = key % table->capacity;
	item->next = table->items[pos];
	table->items[pos] = item;

	++table->count;
	++table->collisions[pos];
	if (table->collisions[pos] > table->max_collisions)
	{
		hash_enlarge(table);
		item = hash_find_item(table, key);
	}
	return item->payload;
}

void* hash_find(HashTable* table, int key)
{
	HashItem* item = hash_find_item(table, key);
	if (item == NULL)
	{
		return NULL;
	}
	return item->payload;
}


