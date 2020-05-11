#include "alloc.h"
#include "hashtable.h"
#include <string.h>

HashTable* hash_new(int payload_size, int capacity, HashEqual equal, Hasher* hasher)
{
	int item_size = payload_size + sizeof(void*);
	HashTable* tbl = allocate(sizeof(HashTable));

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
	memset(tbl->collisions, 0, sizeof(int) * capacity);
	tbl->equal = equal;
	tbl->max_collisions = 5;
	tbl->hasher = hasher;
	return tbl;
}

HashTable* hash_destroy(HashTable* table)
{
	for (int i = 0; i < table->capacity; ++i)
	{
		HashItem* item = table->items[i];
		HashItem* next;
		while (item)
		{
			next = item->next;
			free(item);
			item = next;
		}
	}
	deallocate(table->items);
	deallocate(table->collisions);
	deallocate(table);
	return NULL;
}

int* prime_buffer = NULL;
int buffer_used = 0;
int buffer_capacity = 0;

/*
int next_prime(int x)
{
	if (prime_buffer == NULL)
	{
		buffer_capacity = 128;
		prime_buffer = malloc(buffer_capacity);
		int initial[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
			31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
			73, 79, 83, 89, 97,
			101
		};

		memcpy(prime_buffer, initial, sizeof(initial));

		buffer_used = sizeof(initial) / sizeof(int);
	}
	int left = 0;
	int right = buffer_used;
	int mid = (left + right) / 2;

	while (x < prime_buffer[mid-1]  )
	{

	}


}*/

void hash_enlarge(HashTable* table) {
	int capacity = table->capacity * 2 + 1;
	HashTable *tbl = hash_new(table->payload_size, capacity, table->equal, table->hasher);
	
	for (int i = 0; i < table->capacity; ++i)
	{
		HashItem* item = table->items[i];

		while (item)
		{
			hash_add(tbl, item->key, item->payload);
		}
	}
	
	deallocate(table->collisions);
	deallocate(table->items);

	memcpy(table, tbl, sizeof(HashTable));
	deallocate(tbl);
}

HashItem* hash_find_item(HashTable* table, const void* key, HashedKey hashed_key, int should_compute_hash)
{
	if (should_compute_hash)
	{
		hashed_key = table->hasher(key);
	}
	HashItem* target = table->items[hashed_key % table->capacity];

	while (target)
	{
		if (target->key_hash == hashed_key)
		{
			if (table->equal(target->key, key))
			{
				return target;
			}
		}
		target = target->next;
	}

	return NULL;
}

void* hash_add(HashTable* table, const void* key, void* payload)
{
	
	HashItem* item = allocate(table->item_size);
	// memcpy(item->payload, payload, table->payload_size);
	item->payload = payload;
	item->key = key;
	item->key_hash = table->hasher(key);

	int pos = item->key_hash % table->capacity;
	item->next = table->items[pos];
	table->items[pos] = item;

	++table->count;
	++table->collisions[pos];
	if (table->collisions[pos] > table->max_collisions)
	{
		hash_enlarge(table);
		item = hash_find_item(table, key, item->key_hash, 1);
	}
	return item->payload;
}

void* hash_find(HashTable* table, const void* key)
{
	HashItem* item = hash_find_item(table, key, -1, 0);
	if (item == NULL)
	{
		return NULL;
	}
	return item->payload;
}

int hash_impl_strcmp(const void* a, const void* b)
{
	return (strcmp(a, b) == 0);
}

int hash_impl_strhash(const void* a)
{
	const char* str = a;

	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

HashTable* hash_new_strkey(int payload_size, int capacity)
{
	hash_new(payload_size, capacity, hash_impl_strcmp, hash_impl_strhash);
}

int hash_remove(HashTable* table, const void* key)
{
	int hashed = table->hasher(key);
	int hashed_key = hashed % table->capacity;
	HashItem* item = table->items[hashed_key];

	if (table->equal(item->key, key))
	{
		table->items[hashed_key] = item->next;
		free(item);
		return 0;
	}

	HashItem* prev = item;
	item = item->next;
	while (item)
	{
		if (table->equal(item->key, key))
		{
			prev->next = item->next;
			free(item);
			return 0;
		}
	}

	return -1;
}




