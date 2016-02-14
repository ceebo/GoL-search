#define BYTE_SEQ_STORE_MIN_NODE_DATA_SIZE 4
#define BYTE_SEQ_STORE_MAX_BYTE_SEQ_SIZE 0x0000ffff

typedef struct _ByteSeqStoreNode ByteSeqStoreNode;
typedef struct _ByteSeqStoreNode
{
	ByteSeqStoreNode *next_node;
	int used_size;
	u8 data [];
} ByteSeqStoreNode;

typedef struct
{
	ByteSeqStoreNode *first_node;
	ByteSeqStoreNode *last_node;
	int node_data_size;
	s64 seq_count;
} ByteSeqStore;

static void ByteSeqStore_preinit (ByteSeqStore *bss)
{
	if (!bss)
		return (void) ffsc (__func__);
	
	bss->first_node = NULL;
	bss->last_node = NULL;
	bss->node_data_size = 0;
	bss->seq_count = 0;
}

static void ByteSeqStore_free (ByteSeqStore *bss)
{
	if (!bss)
		return (void) ffsc (__func__);
	
	while (bss->first_node)
	{
		ByteSeqStoreNode *del_node = bss->first_node;
		bss->first_node = del_node->next_node;
		free (del_node);
	}
	
	ByteSeqStore_preinit (bss);
}

static ByteSeqStoreNode *ByteSeqStore_alloc_node (int node_size)
{
	ByteSeqStoreNode *new_node = malloc (node_size);
	if (!new_node)
	{
		fprintf (stderr, "Out of memory in %s\n", __func__);
		return NULL;
	}
	
	new_node->next_node = NULL;
	new_node->used_size = 0;
	
	return new_node;
}

static int ByteSeqStore_create (ByteSeqStore *bss, int node_size)
{
	if (!bss || node_size < (int) sizeof (ByteSeqStoreNode) + BYTE_SEQ_STORE_MIN_NODE_DATA_SIZE)
		return ffsc (__func__);
	
	ByteSeqStore_preinit (bss);
	
	bss->first_node = ByteSeqStore_alloc_node (node_size);
	bss->last_node = bss->first_node;
	bss->node_data_size = node_size - sizeof (ByteSeqStoreNode);
	bss->seq_count = 0;
	
	if (!bss->first_node)
	{
		ByteSeqStore_free (bss);
		return FALSE;
	}
	
	return TRUE;
}

static void ByteSeqStore_clear (ByteSeqStore *bss)
{
	if (!bss || !bss->last_node)
		return (void) ffsc (__func__);
	
	while (bss->first_node->next_node)
	{
		ByteSeqStoreNode *del_node = bss->first_node->next_node;
		bss->first_node->next_node = del_node->next_node;
		free (del_node);
	}
	
	bss->last_node = bss->first_node;
	bss->seq_count = 0;
	
	bss->first_node->used_size = 0;
}

static int ByteSeqStore_store (ByteSeqStore *bss, const u8 *byte_seq, int seq_size)
{
	if (!bss || !bss->last_node || !byte_seq || seq_size > BYTE_SEQ_STORE_MAX_BYTE_SEQ_SIZE)
		return ffsc (__func__);
	
	int needed_space = 2 + seq_size;
	if (needed_space > bss->node_data_size)
		return ffsc (__func__);
	
	if (bss->last_node->used_size + needed_space > bss->node_data_size)
	{
		ByteSeqStoreNode *new_node = ByteSeqStore_alloc_node ((int) sizeof (ByteSeqStoreNode) + bss->node_data_size);
		if (!new_node)
			return FALSE;
		
		bss->last_node->next_node = new_node;
		bss->last_node = new_node;
	}
	
	bss->last_node->data [bss->last_node->used_size++] = seq_size >> 8;
	bss->last_node->data [bss->last_node->used_size++] = seq_size & 0x00ff;
	
	int seq_ix;
	for (seq_ix = 0; seq_ix < seq_size; seq_ix++)
		bss->last_node->data [bss->last_node->used_size++] = byte_seq [seq_ix];
	
	bss->seq_count++;
	
	return TRUE;
}

static void ByteSeqStore_start_get_iteration (const ByteSeqStore *bss, ByteSeqStoreNode **bss_node, int *node_data_offset)
{
	if (bss_node)
		*bss_node = NULL;
	if (node_data_offset)
		*node_data_offset = 0;
	
	if (!bss || !bss->last_node || !bss_node || !node_data_offset)
		return (void) ffsc (__func__);
	
	*bss_node = bss->first_node;
	*node_data_offset = 0;
}

static int ByteSeqStore_get_next (const ByteSeqStore *bss, ByteSeqStoreNode **bss_node, int *node_data_offset, u8 *byte_seq, int max_seq_size, int *seq_size)
{
	if (!bss || !bss->last_node || !bss_node || !node_data_offset || !byte_seq || !seq_size)
		return ffsc (__func__);
	
	if (*bss_node == NULL)
		return FALSE;
	
	int size = ((int) ((*bss_node)->data [*node_data_offset] << 8)) | (int) (*bss_node)->data [(*node_data_offset) + 1];
	int success = TRUE;
	
	if (size > max_seq_size)
	{
		*seq_size = 0;
		success = FALSE;
	}
	else
	{
		*seq_size = size;
		int seq_ix;
		for (seq_ix = 0; seq_ix < *seq_size; seq_ix++)
			byte_seq [seq_ix] = (*bss_node)->data [(*node_data_offset) + (2 + seq_ix)];
	}
	
	*node_data_offset += (2 + size);
	
	if (*node_data_offset >= (*bss_node)->used_size)
	{
		*bss_node = (*bss_node)->next_node;
		*node_data_offset = 0;
	}
	
	return success;
}
