typedef struct
{
	int size;
	u64 *random_data;
} RandomDataArray;

static void RandomDataArray_preinit (RandomDataArray *rda)
{
	if (!rda)
		return (void) ffsc (__func__);
	
	rda->size = 0;
	rda->random_data = NULL;
}

static void RandomDataArray_free (RandomDataArray *rda)
{
	if (!rda)
		return (void) ffsc (__func__);
	
	if (rda->random_data)
		free (rda->random_data);
	
	RandomDataArray_preinit (rda);
}

static int RandomDataArray_create (RandomDataArray *rda, int size)
{
	if (!rda)
		return ffsc (__func__);
	
	RandomDataArray_preinit (rda);
	
	if (size <= 0)
		return ffsc (__func__);
	
	rda->size = size;
	rda->random_data = malloc (sizeof (u64) * (u64) size);
	
	if (!rda->random_data)
	{
		fprintf (stderr, "Out of memory in %s\n", __func__);
		RandomDataArray_free (rda);
		return FALSE;
	}
	
	int data_ix;
	for (data_ix = 0; data_ix < size; data_ix++)
		rda->random_data [data_ix] = random_u64 ();
	
	return TRUE;
}

static int RandomDataArray_verify_compatibility (const RandomDataArray *rda, int needed_size)
{
	if (!rda || !rda->random_data)
		return ffsc (__func__);
	
	return rda->size >= needed_size;
}


typedef struct
{
	u64 key;
	u64 data;
} HashTable_u64_entry;

typedef struct
{
	u64 cur_capacity;
	u64 used_capacity;
	HashTable_u64_entry *table;
} HashTable_u64;

static void HashTable_u64_preinit (HashTable_u64 *ht)
{
	if (!ht)
		return (void) ffsc (__func__);
	
	ht->cur_capacity = 0;
	ht->used_capacity = 0;
	ht->table = NULL;
}

static void HashTable_u64_free (HashTable_u64 *ht)
{
	if (!ht)
		return (void) ffsc (__func__);
	
	if (ht->table)
		free (ht->table);
	
	HashTable_u64_preinit (ht);
}

static int HashTable_u64_allocate (HashTable_u64 *ht, u64 capacity)
{
	if (!ht || ht->table != NULL || bit_count_u64 (capacity) != 1)
		return ffsc (__func__);
	
	ht->table = malloc (capacity * sizeof (HashTable_u64_entry));
	if (!ht->table)
	{
		fprintf (stderr, "Out of memory in %s\n", __func__);
		HashTable_u64_free (ht);
		return FALSE;
	}
	
	ht->cur_capacity = capacity;
	ht->used_capacity = 0;
	
	u64 entry_ix;
	for (entry_ix = 0; entry_ix < capacity; entry_ix++)
	{
		ht->table [entry_ix].key = 0;
		ht->table [entry_ix].data = 0;
	}
	
	return TRUE;
}

static int HashTable_u64_create (HashTable_u64 *ht, u64 first_capacity)
{
	if (!ht)
		return ffsc (__func__);
	
	HashTable_u64_preinit (ht);
	
	if (!HashTable_u64_allocate (ht, first_capacity))
		return FALSE;
	
	return TRUE;
}

static int HashTable_u64_store (HashTable_u64 *ht, u64 key, u64 data, int replace_previous_data, int *was_present);
static int HashTable_u64_reallocate (HashTable_u64 *ht, u64 new_capacity)
{
	if (!ht || !ht->table || new_capacity <= ht->cur_capacity)
		return ffsc (__func__);
	
	HashTable_u64 temp_ht;
	
	if (!HashTable_u64_create (&temp_ht, new_capacity))
		return FALSE;
	
	u64 entry_ix;
	for (entry_ix = 0; entry_ix < ht->cur_capacity; entry_ix++)
	{
		u64 entry_key = ht->table [entry_ix].key;
		if (entry_key != 0)
			HashTable_u64_store (&temp_ht, entry_key, ht->table [entry_ix].data, TRUE, NULL);
	}
	
	HashTable_u64_free (ht);
	ht->cur_capacity = temp_ht.cur_capacity;
	ht->used_capacity = temp_ht.used_capacity;
	ht->table = temp_ht.table;
	
	return TRUE;
}

static int HashTable_u64_get_data (const HashTable_u64 *ht, u64 key, u64 *data)
{
	if (!ht || !ht->table || key == 0)
		return ffsc (__func__);
	
	u64 entry_ix = key & (ht->cur_capacity - 1);
	while (TRUE)
	{
		if (ht->table [entry_ix].key == key)
		{
			if (data != NULL)
				*data = ht->table [entry_ix].data;
			
			return TRUE;
		}
		else if (ht->table [entry_ix].key == 0)
		{
			if (data != NULL)
				*data = 0;
			
			return FALSE;
		}
		
		entry_ix = (entry_ix + 1) & (ht->cur_capacity - 1);
	}
}

static int HashTable_u64_store (HashTable_u64 *ht, u64 key, u64 data, int replace_previous_data, int *was_present)
{
	if (!ht || !ht->table || key == 0)
		return ffsc (__func__);
	
	if (8 * ht->used_capacity >= 7 * ht->cur_capacity)
		if (!HashTable_u64_reallocate (ht, 2 * ht->cur_capacity))
			return FALSE;
	
	u64 entry_ix = key & (ht->cur_capacity - 1);
	while (TRUE)
	{
		if (ht->table [entry_ix].key == key)
		{
			if (replace_previous_data)
				ht->table [entry_ix].data = data;
			
			if (was_present != NULL)
				*was_present = TRUE;
			
			return TRUE;
		}
		else if (ht->table [entry_ix].key == 0)
		{
			ht->table [entry_ix].key = key;
			ht->table [entry_ix].data = data;
			
			ht->used_capacity++;
			
			if (was_present != NULL)
				*was_present = FALSE;
			
			return TRUE;
		}
		
		entry_ix = (entry_ix + 1) & (ht->cur_capacity - 1);
	}
}
