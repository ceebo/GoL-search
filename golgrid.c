typedef struct
{
	Rect grid_rect;
	int pop_x_on;
	int pop_x_off;
	int pop_y_on;
	int pop_y_off;
	void *grid_alloc;
	u64 *grid;
	u64 row_offset;
	s64 generation;
} GoLGrid;

static void GoLGrid_preinit (GoLGrid *gg)
{
	if (!gg)
		return (void) ffsc (__func__);
	
	Rect_make (&gg->grid_rect, 0, 0, 0, 0);
	gg->pop_x_on = 0;
	gg->pop_x_off = 0;
	gg->pop_y_on = 0;
	gg->pop_y_off = 0;
	gg->grid_alloc = NULL;
	gg->grid = NULL;
	gg->row_offset = 0;
	gg->generation = 0;
}

static void GoLGrid_free (GoLGrid *gg)
{
	if (!gg)
		return (void) ffsc (__func__);
	
	if (gg->grid_alloc)
		free (gg->grid_alloc);
	
	GoLGrid_preinit (gg);
}

static void GoLGrid_set_empty_population_rect (GoLGrid *gg)
{
	if (!gg)
		return (void) ffsc (__func__);
	
	gg->pop_x_on = gg->grid_rect.width >> 1;
	gg->pop_x_off = gg->grid_rect.width >> 1;
	gg->pop_y_on = gg->grid_rect.height >> 1;
	gg->pop_y_off = gg->grid_rect.height >> 1;
}

static int GoLGrid_create (GoLGrid *gg, const Rect *grid_rect, u64 grid_row_byte_offset, u64 grid_alloc_alignment_base, u64 grid_alloc_alignment_offset)
{
	if (!gg)
		return ffsc (__func__);
	
	GoLGrid_preinit (gg);
	
	if (!grid_rect || (grid_rect->left_x & 0x0000001f) != 0 || (grid_rect->top_y & 0x0000001f) != 0 || (grid_rect->width & 0x0000001f) != 0 || grid_rect->width < 64 ||
			(grid_rect->height & 0x0000001f) != 0 || grid_rect->height < 32 || (grid_row_byte_offset % 8) != 0 || grid_row_byte_offset < (u64) (grid_rect->width >> 3) ||
			grid_row_byte_offset >= 2 * (u64) (grid_rect->width >> 3) || bit_count_u64 (grid_alloc_alignment_base) > 1 || grid_alloc_alignment_base < 8 ||
			(grid_alloc_alignment_offset % 8) != 0 || grid_alloc_alignment_offset >= grid_alloc_alignment_base)
		return ffsc (__func__);
	
	u64 test_endianness = 0x0807060504030201ULL;
	if (*(u8 *) &test_endianness != 0x01)
	{
		fprintf (stderr, "The GoLGrid datatype only works with little-endian hardware\n");
		return FALSE;
	}

	Rect_copy (grid_rect, &gg->grid_rect);
	GoLGrid_set_empty_population_rect (gg);
	
	gg->grid_alloc = malloc (grid_alloc_alignment_base + (grid_row_byte_offset * grid_rect->height));
	if (!gg->grid_alloc)
	{
		fprintf (stderr, "Out of memory in %s\n", __func__);
		GoLGrid_free (gg);
		return FALSE;
	}
	
	memset (gg->grid_alloc, 0, grid_alloc_alignment_base + (grid_row_byte_offset * grid_rect->height));
	
	uintptr_t grid_start = (((uintptr_t) gg->grid_alloc) & ~(grid_alloc_alignment_base - 1)) + grid_alloc_alignment_offset;
	if (grid_start < (uintptr_t) gg->grid_alloc)
		grid_start += grid_alloc_alignment_base;
	
	gg->grid = (u64 *) grid_start;
	gg->row_offset = grid_row_byte_offset / sizeof (u64);
	gg->generation = 0;
	
	return TRUE;
}

static int GoLGrid_get_cell (const GoLGrid *gg, int x, int y)
{
	if (!gg || !gg->grid)
		return ffsc (__func__);
	
	if (!Rect_within (&gg->grid_rect, x, y))
		return 0;
	
	x -= gg->grid_rect.left_x;
	y -= gg->grid_rect.top_y;
	
	return (gg->grid [gg->row_offset * y + (x >> 6)] >> (x & 0x0000003f)) & 0x0000000000000001ULL;
}

static int GoLGrid_set_cell_on (GoLGrid *gg, int x, int y)
{
	if (!gg || !gg->grid)
		return ffsc (__func__);
	
	if (!Rect_within (&gg->grid_rect, x, y))
		return FALSE;
	
	x -= gg->grid_rect.left_x;
	y -= gg->grid_rect.top_y;
	
	gg->grid [gg->row_offset * y + (x >> 6)] |= (0x0000000000000001ULL << (x & 0x0000003f));
	
	if (gg->pop_x_off <= gg->pop_x_on)
	{
		gg->pop_x_on = x;
		gg->pop_x_off = x + 1;
		gg->pop_y_on = y;
		gg->pop_y_off = y + 1;
	}
	else
	{
		if (gg->pop_x_on > x)
			gg->pop_x_on = x;
		else if (gg->pop_x_off < x + 1)
			gg->pop_x_off = x + 1;
		
		if (gg->pop_y_on > y)
			gg->pop_y_on = y;
		else if (gg->pop_y_off < y + 1)
			gg->pop_y_off = y + 1;
	}
	
	return TRUE;
}

static int GoLGrid_increase_pop_x_on (GoLGrid *gg)
{
	// Function sanity check handled by caller
	
	u64 or_of_col = 0;
	int col_ix;
	int row_ix;
	
	for (col_ix = gg->pop_x_on >> 6; col_ix <= (gg->pop_x_off - 1) >> 6; col_ix++)
	{
		for (row_ix = gg->pop_y_on; row_ix < gg->pop_y_off; row_ix++)
			or_of_col |= gg->grid [(gg->row_offset * row_ix) + col_ix];
		
		if (or_of_col != 0)
			break;
	}
	
	if (or_of_col == 0)
	{
		GoLGrid_set_empty_population_rect (gg);
		return FALSE;
	}
	
	gg->pop_x_on = (64 * col_ix) + least_significant_bit_u64 (or_of_col);
	
	return TRUE;
}

static void GoLGrid_decrease_pop_x_off (GoLGrid *gg)
{
	// Function sanity check handled by caller
	
	u64 or_of_col = 0;
	int col_ix;
	int row_ix;
	
	for (col_ix = (gg->pop_x_off - 1) >> 6; col_ix >= gg->pop_x_on >> 6; col_ix--)
	{
		for (row_ix = gg->pop_y_on; row_ix < gg->pop_y_off; row_ix++)
			or_of_col |= gg->grid [(gg->row_offset * row_ix) + col_ix];
		
		if (or_of_col != 0)
			break;
	}
	
	gg->pop_x_off = (64 * col_ix) + (most_significant_bit_u64 (or_of_col) + 1);
}

static void GoLGrid_increase_pop_y_on (GoLGrid *gg)
{
	// Function sanity check handled by caller
	
	int row_ix;
	int col_ix;
	for (row_ix = gg->pop_y_on; row_ix < gg->pop_y_off; row_ix++)
		for (col_ix = gg->pop_x_on >> 6; col_ix <= (gg->pop_x_off - 1) >> 6; col_ix++)
			if (gg->grid [(gg->row_offset * row_ix) + col_ix] != 0)
			{
				gg->pop_y_on = row_ix;
				return;
			}
}

static void GoLGrid_decrease_pop_y_off (GoLGrid *gg)
{
	// Function sanity check handled by caller
	
	int row_ix;
	int col_ix;
	for (row_ix = gg->pop_y_off - 1; row_ix >= gg->pop_y_on; row_ix--)
		for (col_ix = gg->pop_x_on >> 6; col_ix <= (gg->pop_x_off - 1) >> 6; col_ix++)
			if (gg->grid [(gg->row_offset * row_ix) + col_ix] != 0)
			{
				gg->pop_y_off = row_ix + 1;
				return;
			}
}

static int GoLGrid_set_cell_off (GoLGrid *gg, int x, int y)
{
	if (!gg || !gg->grid)
		return ffsc (__func__);
	
	if (!Rect_within (&gg->grid_rect, x, y))
		return FALSE;
	
	if (gg->pop_x_off <= gg->pop_x_on)
		return TRUE;
	
	x -= gg->grid_rect.left_x;
	y -= gg->grid_rect.top_y;
	
	gg->grid [gg->row_offset * y + (x >> 6)] &= ~(0x0000000000000001ULL << (x & 0x0000003f));
	
	int not_empty = TRUE;
	if (x == gg->pop_x_on)
		not_empty = GoLGrid_increase_pop_x_on (gg);
	
	if (not_empty)
	{
		if (x == gg->pop_x_off - 1)
			GoLGrid_decrease_pop_x_off (gg);
		if (y == gg->pop_y_on)
			GoLGrid_increase_pop_y_on (gg);
		if (y == gg->pop_y_off - 1)
			GoLGrid_decrease_pop_y_off (gg);
	}
	
	return TRUE;
}

static void GoLGrid_shrink_population_y_limits_single_col (GoLGrid *gg, int min_new_y_on, int max_new_y_off)
{
	// Function sanity check handled by caller
	
	int new_y_on = min_new_y_on;
	while (gg->grid [new_y_on] == 0)
		new_y_on++;
	
	gg->pop_y_on = new_y_on;
	
	int new_y_off = max_new_y_off;
	while (gg->grid [new_y_off - 1] == 0)
		new_y_off--;
	
	gg->pop_y_off = new_y_off;
}

static void GoLGrid_set_generation (GoLGrid *gg, s64 generation)
{
	if (!gg)
		return (void) ffsc (__func__);
	
	gg->generation = generation;
}

static void GoLGrid_clear_one_col (u64 *col_entry, u64 row_offset, int row_cnt)
{
	// Function sanity check handled by caller
	
	int row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		*col_entry = 0;
		col_entry += row_offset;
	}
}

static void GoLGrid_clear_two_cols (u64 *col_entry_1, u64 *col_entry_2, u64 row_offset, int row_cnt)
{
	// Function sanity check handled by caller
	
	int row_ix;
	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		*col_entry_1 = 0;
		*col_entry_2 = 0;
		col_entry_1 += row_offset;
		col_entry_2 += row_offset;
	}
}

static void GoLGrid_clear_col_range (u64 *col_entry, int col_cnt, u64 row_offset, int row_cnt)
{
	// Function sanity check handled by caller
	
	int row_ix;
	int col_ix;

	for (row_ix = 0; row_ix < row_cnt; row_ix++)
	{
		u64 *row_entry = col_entry;
		
		// This does the same for col_cnt <= 6 and col_cnt > 6, but makes the compiler realize small values are likely, so it (at least GCC) will generate one version with in-line code, and one with a call to memset
		if (col_cnt <= 6)
			for (col_ix = 0; col_ix < col_cnt; col_ix++)
				*row_entry++ = 0;
		else
			for (col_ix = 0; col_ix < col_cnt; col_ix++)
				*row_entry++ = 0;
		
		col_entry += row_offset;
	}
}

static void GoLGrid_clear_strip (GoLGrid *gg, int row_on, int row_off, int byte_col_first, int byte_col_last)
{
	// Function sanity check handled by caller
	
	int word_cols = ((byte_col_last - byte_col_first) >> 3) + 1;
	
	if ((byte_col_last >> 3) - (byte_col_first >> 3) < word_cols)
		byte_col_first &= 0xfffffff8;
	else
		byte_col_first = (byte_col_first & 0xfffffff8) | ((byte_col_last + 1) & 0x00000007);
	
	u64 *row_entry = gg->grid + (gg->row_offset * row_on);
	
	if (word_cols == 1)
		GoLGrid_clear_one_col ((u64 *) (((u8 *) row_entry) + byte_col_first), gg->row_offset, row_off - row_on);
	else if (word_cols == 2)
		GoLGrid_clear_two_cols ((u64 *) (((u8 *) row_entry) + byte_col_first), (u64 *) (((u8 *) row_entry) + (byte_col_first + 8)), gg->row_offset, row_off - row_on);
	else
		GoLGrid_clear_col_range ((u64 *) (((u8 *) row_entry) + byte_col_first), word_cols, gg->row_offset, row_off - row_on);
}

static void GoLGrid_clear_left_right_strips (GoLGrid *gg, int row_on, int row_off, int left_byte_col_first, int left_byte_col_last, int right_byte_col_first, int right_byte_col_last)
{
	// Function sanity check handled by caller
	// This should not be called if any of the two strips have byte_col_last < byte_col_first
	
	int left_word_cols = ((left_byte_col_last - left_byte_col_first) >> 3) + 1;
	
	if ((left_byte_col_last >> 3) - (left_byte_col_first >> 3) < left_word_cols)
		left_byte_col_first &= 0xfffffff8;
	else
		left_byte_col_first = (left_byte_col_first & 0xfffffff8) | ((left_byte_col_last + 1) & 0x00000007);
	
	int right_word_cols = ((right_byte_col_last - right_byte_col_first) >> 3) + 1;
	
	if ((right_byte_col_last >> 3) - (right_byte_col_first >> 3) < right_word_cols)
		right_byte_col_first &= 0xfffffff8;
	else
		right_byte_col_first = (right_byte_col_first & 0xfffffff8) | ((right_byte_col_last + 1) & 0x00000007);
	
	if (left_word_cols == 1 && right_word_cols == 1)
		GoLGrid_clear_two_cols ((u64 *) (((u8 *) (gg->grid + (gg->row_offset * row_on))) + left_byte_col_first), (u64 *) (((u8 *) (gg->grid + (gg->row_offset * row_on))) + right_byte_col_first),
				gg->row_offset, row_off - row_on);
	else
	{
		GoLGrid_clear_strip (gg, row_on, row_off, left_byte_col_first, left_byte_col_last);
		GoLGrid_clear_strip (gg, row_on, row_off, right_byte_col_first, right_byte_col_last);
	}
}

static void GoLGrid_clear (GoLGrid *gg)
{
	if (!gg || !gg->grid)
		return (void) ffsc (__func__);
	
	gg->generation = 0;
	
	if (gg->pop_x_off <= gg->pop_x_on)
		return;
	
	if (gg->row_offset == 1)
		GoLGrid_clear_one_col (gg->grid + gg->pop_y_on, 1, gg->pop_y_off - gg->pop_y_on);
	else
		GoLGrid_clear_strip (gg, gg->pop_y_on, gg->pop_y_off, gg->pop_x_on >> 3, (gg->pop_x_off - 1) >> 3);
	
	GoLGrid_set_empty_population_rect (gg);
}

static int GoLGrid_is_same_grid_specs (const GoLGrid *gg_1, const GoLGrid *gg_2)
{
	// Function sanity check handled by caller
	
	return (Rect_is_equal_to (&gg_1->grid_rect, &gg_2->grid_rect) && gg_1->row_offset == gg_2->row_offset);
}

static int GoLGrid_is_equal_to (const GoLGrid *obj_gg, const GoLGrid *ref_gg)
{
	if (!obj_gg || !obj_gg->grid || !ref_gg || !ref_gg->grid || !GoLGrid_is_same_grid_specs (obj_gg, ref_gg))
		return ffsc (__func__);
	
	if (obj_gg->pop_x_on != ref_gg->pop_x_on || obj_gg->pop_x_off != ref_gg->pop_x_off || obj_gg->pop_y_on != ref_gg->pop_y_on || obj_gg->pop_y_off != ref_gg->pop_y_off)
		return FALSE;
	
	if (obj_gg->pop_x_off <= obj_gg->pop_x_on)
		return TRUE;
	
	int row_ix;
	int col_ix;
	
	if (obj_gg->row_offset == 1)
	{
		for (row_ix = obj_gg->pop_y_on; row_ix < obj_gg->pop_y_off; row_ix++)
			if (obj_gg->grid [row_ix] != ref_gg->grid [row_ix])
				return FALSE;
	}
	else
	{
		int byte_col_on = obj_gg->pop_x_on >> 3;
		int byte_col_off = (obj_gg->pop_x_off + 7) >> 3;
		int strip_start = byte_col_on - lowest_of (byte_col_on & 0x00000007, (byte_col_on - byte_col_off) & 0x00000007);
		int strip_cnt = ((byte_col_off - byte_col_on) + 7) >> 3;
		
		if (strip_cnt == 1)
		{
			for (row_ix = obj_gg->pop_y_on; row_ix < obj_gg->pop_y_off; row_ix++)
				if (*((u64 *) (((u8 *) (obj_gg->grid + (obj_gg->row_offset * row_ix))) + strip_start)) != *((u64 *) (((u8 *) (ref_gg->grid + (ref_gg->row_offset * row_ix))) + strip_start)))
					return FALSE;
		}
		else
			for (row_ix = obj_gg->pop_y_on; row_ix < obj_gg->pop_y_off; row_ix++)
				for (col_ix = 0; col_ix < strip_cnt; col_ix++)
					if (*((u64 *) (((u8 *) (obj_gg->grid + (obj_gg->row_offset * row_ix) + col_ix)) + strip_start)) !=
							*((u64 *) (((u8 *) (ref_gg->grid + (ref_gg->row_offset * row_ix) + col_ix)) + strip_start)))
						return FALSE;
	}
	
	return TRUE;
}

static int GoLGrid_is_subset (const GoLGrid *obj_gg, const GoLGrid *ref_gg)
{
  int *x_of_first_not_in_ref = NULL;
  int *y_of_first_not_in_ref = NULL;
	if (x_of_first_not_in_ref)
		*x_of_first_not_in_ref = 0;
	if (y_of_first_not_in_ref)
		*y_of_first_not_in_ref = 0;
	
	if (!obj_gg || !obj_gg->grid || !ref_gg || !ref_gg->grid || !GoLGrid_is_same_grid_specs (obj_gg, ref_gg))
		return ffsc (__func__);
	
	if (obj_gg->pop_x_off <= obj_gg->pop_x_on)
		return TRUE;
	
	if (x_of_first_not_in_ref == NULL && y_of_first_not_in_ref == NULL)
		if (obj_gg->pop_x_on < ref_gg->pop_x_on || obj_gg->pop_x_off > ref_gg->pop_x_off || obj_gg->pop_y_on < ref_gg->pop_y_on || obj_gg->pop_y_off > ref_gg->pop_y_off)
			return FALSE;
	
	int row_ix;
	int col_ix;
	
	if (obj_gg->row_offset == 1)
		for (row_ix = obj_gg->pop_y_on; row_ix < obj_gg->pop_y_off; row_ix++)
		{
			u64 and_not_word = obj_gg->grid [row_ix] & ~ref_gg->grid [row_ix];
			if (and_not_word != 0)
			{
				if (x_of_first_not_in_ref)
					*x_of_first_not_in_ref = obj_gg->grid_rect.left_x + least_significant_bit_u64 (and_not_word);
				if (y_of_first_not_in_ref)
					*y_of_first_not_in_ref = obj_gg->grid_rect.top_y + row_ix;
				
				return FALSE;
			}
		}
	else
	{
		int byte_col_on = obj_gg->pop_x_on >> 3;
		int byte_col_off = (obj_gg->pop_x_off + 7) >> 3;
		int strip_start = byte_col_on - lowest_of (byte_col_on & 0x00000007, (byte_col_on - byte_col_off) & 0x00000007);
		int strip_cnt = ((byte_col_off - byte_col_on) + 7) >> 3;
		
		if (strip_cnt == 1)
			for (row_ix = obj_gg->pop_y_on; row_ix < obj_gg->pop_y_off; row_ix++)
			{
				u64 and_not_word = *((u64 *) (((u8 *) (obj_gg->grid + (obj_gg->row_offset * row_ix))) + strip_start)) & ~*((u64 *) (((u8 *) (ref_gg->grid + (ref_gg->row_offset * row_ix))) + strip_start));
				if (and_not_word != 0)
				{
					if (x_of_first_not_in_ref)
						*x_of_first_not_in_ref = obj_gg->grid_rect.left_x + (8 * strip_start) + least_significant_bit_u64 (and_not_word);
					if (y_of_first_not_in_ref)
						*y_of_first_not_in_ref = obj_gg->grid_rect.top_y + row_ix;
					
					return FALSE;
				}
			}
		else
			for (row_ix = obj_gg->pop_y_on; row_ix < obj_gg->pop_y_off; row_ix++)
				for (col_ix = 0; col_ix < strip_cnt; col_ix++)
				{
					u64 and_not_word = *((u64 *) (((u8 *) (obj_gg->grid + (obj_gg->row_offset * row_ix) + col_ix)) + strip_start)) &
							~*((u64 *) (((u8 *) (ref_gg->grid + (ref_gg->row_offset * row_ix) + col_ix)) + strip_start));
					if (and_not_word != 0)
					{
						if (x_of_first_not_in_ref)
							*x_of_first_not_in_ref = obj_gg->grid_rect.left_x + (8 * strip_start) + (64 * col_ix) + least_significant_bit_u64 (and_not_word);
						if (y_of_first_not_in_ref)
							*y_of_first_not_in_ref = obj_gg->grid_rect.top_y + row_ix;
						
						return FALSE;
					}
				}
	}
	
	return TRUE;
}

static u64 GoLGrid_get_hash_key (const GoLGrid *gg, const RandomDataArray *rda)
{
	if (!gg || !gg->grid || !rda || !RandomDataArray_verify_compatibility (rda, gg->row_offset * gg->grid_rect.height))
		return ffsc (__func__);
	
	u64 hash_key = 0x0123456789abcdefULL;
	int row_ix;
	int col_ix;
	
	for (row_ix = gg->pop_y_on; row_ix < gg->pop_y_off; row_ix++)
		for (col_ix = (gg->pop_x_on >> 6); col_ix <= ((gg->pop_x_off - 1) >> 6); col_ix++)
		{
			u64 grid_word = gg->grid [(gg->row_offset * row_ix) + col_ix];
			u64 grid_word_l = (grid_word & 0x00000000ffffffffULL);
			u64 grid_word_h = (grid_word >> 32);
			
			u64 random_word = rda->random_data [(gg->row_offset * row_ix) + col_ix];
			u64 random_word_l = (random_word & 0x00000000ffffffffULL);
			u64 random_word_h = (random_word >> 32);
			
			// This is inspired by a 64-bit to 128-bit multiplication
			u64 mid_word = (grid_word_l * random_word_h) ^ (grid_word_h * random_word_l);
			hash_key ^= (grid_word_l * random_word_l) ^ ((mid_word << 32) | (mid_word >> 32)) ^ (grid_word_h * random_word_h);
		}
	
	return hash_key;
}

static void GoLGrid_or (const GoLGrid *src_gg, GoLGrid *dst_gg)
{
	if (!src_gg || !src_gg->grid || !dst_gg || !dst_gg->grid || !GoLGrid_is_same_grid_specs (src_gg, dst_gg))
		return (void) ffsc (__func__);
	
	if (src_gg->pop_x_off <= src_gg->pop_x_on)
		return;
	
	int row_ix;
	int col_ix;
	
	if (src_gg->row_offset == 1)
		for (row_ix = src_gg->pop_y_on; row_ix < src_gg->pop_y_off; row_ix++)
			dst_gg->grid [row_ix] |= src_gg->grid [row_ix];
	else
	{
		int byte_col_on = src_gg->pop_x_on >> 3;
		int byte_col_off = (src_gg->pop_x_off + 7) >> 3;
		int strip_start = byte_col_on - lowest_of (byte_col_on & 0x00000007, (byte_col_on - byte_col_off) & 0x00000007);
		int strip_cnt = ((byte_col_off - byte_col_on) + 7) >> 3;
		
		if (strip_cnt == 1)
			for (row_ix = src_gg->pop_y_on; row_ix < src_gg->pop_y_off; row_ix++)
				*((u64 *) (((u8 *) (dst_gg->grid + (dst_gg->row_offset * row_ix))) + strip_start)) |= *((u64 *) (((u8 *) (src_gg->grid + (src_gg->row_offset * row_ix))) + strip_start));
		else
			for (row_ix = src_gg->pop_y_on; row_ix < src_gg->pop_y_off; row_ix++)
				for (col_ix = 0; col_ix < strip_cnt; col_ix++)
					*((u64 *) (((u8 *) (dst_gg->grid + (dst_gg->row_offset * row_ix) + col_ix)) + strip_start)) |= *((u64 *) (((u8 *) (src_gg->grid + (src_gg->row_offset * row_ix) + col_ix)) + strip_start));
	}
	
	if (dst_gg->pop_x_off <= dst_gg->pop_x_on)
	{
		dst_gg->pop_x_on = src_gg->pop_x_on;
		dst_gg->pop_x_off = src_gg->pop_x_off;
		dst_gg->pop_y_on = src_gg->pop_y_on;
		dst_gg->pop_y_off = src_gg->pop_y_off;
	}
	else
	{
		dst_gg->pop_x_on = lowest_of (dst_gg->pop_x_on, src_gg->pop_x_on);
		dst_gg->pop_x_off = highest_of (dst_gg->pop_x_off, src_gg->pop_x_off);
		dst_gg->pop_y_on = lowest_of (dst_gg->pop_y_on, src_gg->pop_y_on);
		dst_gg->pop_y_off = highest_of (dst_gg->pop_y_off, src_gg->pop_y_off);
	}
}

static int GoLGrid_or_cell_list (GoLGrid *gg, const CellList_s8 *cl, int x_offs, int y_offs)
{
	if (!gg || !gg->grid || !cl)
		return ffsc (__func__);
	
	int not_clipped = TRUE;
	int cell_ix;
	for (cell_ix = 0; cell_ix < cl->cell_count; cell_ix++)
		not_clipped &= GoLGrid_set_cell_on (gg, cl->cell [cell_ix].x + x_offs, cl->cell [cell_ix].y + y_offs);
	
	return not_clipped;
}

static int GoLGrid_or_glider (GoLGrid *gg, const Glider *gl)
{
	if (!gg || !gg->grid || !gl)
		return ffsc (__func__);
	
	int x_offs;
	int y_offs;
	const CellList_s8 *glider_cells = CellList_s8_glider_cells (gl, gg->generation, &x_offs, &y_offs);
	
	if (!glider_cells)
		return ffsc (__func__);
	
	return GoLGrid_or_cell_list (gg, glider_cells, x_offs, y_offs);
}

static void GoLGrid_or_filled_circle (GoLGrid *gg, double cent_x, double cent_y, double radius)
{
	if (!gg || !gg->grid || radius < 0.0)
		return (void) ffsc (__func__);
	
	int top_y = round_double (cent_y - radius);
	int y_off = 1 + round_double (cent_y + radius);
	int left_x = round_double (cent_x - radius);
	int x_off = 1 + round_double (cent_x + radius);

	int y;
	int x;
	for (y = top_y; y < y_off; y++)
		for (x = left_x; x < x_off; x++)
			if (((double) x - cent_x) * ((double) x - cent_x) + ((double) y - cent_y) * ((double) y - cent_y) < (radius * radius))
				GoLGrid_set_cell_on (gg, x, y);
}

static u64 GoLGrid_evolve_strip_overwrite (const u64 *in_entry, u64 *out_entry, u64 row_offset, int row_count)
{
	// Function sanity check handled by caller
	
	u64 upper_word = *(in_entry - row_offset);
	u64 mid_word = *in_entry;
	in_entry += row_offset;
	u64 lower_word = *in_entry;
	
	u64 or_of_out_words = 0;
	while (TRUE)
	{
		u64 nb_to_add = (upper_word >> 1);
		u64 nb_sum_bit_0 = nb_to_add;
				
		nb_to_add = upper_word;
		u64 nb_sum_bit_1 = nb_sum_bit_0 & nb_to_add;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = (upper_word << 1);
		u64 carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		nb_sum_bit_1 = nb_sum_bit_1 | carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = (mid_word >> 1);
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		u64 nb_sum_overflow = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = (mid_word << 1);
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		u64 carry_1_to_2 = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_overflow = nb_sum_overflow | carry_1_to_2;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = (lower_word >> 1);
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		carry_1_to_2 = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_overflow = nb_sum_overflow | carry_1_to_2;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = lower_word;
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		carry_1_to_2 = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_overflow = nb_sum_overflow | carry_1_to_2;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;
		
		nb_to_add = (lower_word << 1);
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		carry_1_to_2 = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_overflow = nb_sum_overflow | carry_1_to_2;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;
		
		u64 out_word = ~(nb_sum_overflow) & nb_sum_bit_1 & (nb_sum_bit_0 | mid_word);
		or_of_out_words |= out_word;
		
		*out_entry = out_word;
		out_entry += row_offset;
		
		row_count--;
		if (row_count <= 0)
			break;
		
		upper_word = mid_word;
		mid_word = lower_word;
		
		in_entry += row_offset;
		lower_word = *in_entry;
	}
	
	return or_of_out_words;
}

static u64 GoLGrid_evolve_strip_keep_bit_0 (const u64 *in_entry, u64 *out_entry, u64 row_offset, int row_count)
{
	// Function sanity check handled by caller
	
	u64 upper_word = *(in_entry - row_offset);
	u64 mid_word = *in_entry;
	in_entry += row_offset;
	u64 lower_word = *in_entry;
	
	u64 or_of_out_words = 0;
	while (TRUE)
	{
		u64 nb_to_add = (upper_word >> 1);
		u64 nb_sum_bit_0 = nb_to_add;
				
		nb_to_add = upper_word;
		u64 nb_sum_bit_1 = nb_sum_bit_0 & nb_to_add;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = (upper_word << 1);
		u64 carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		nb_sum_bit_1 = nb_sum_bit_1 | carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = (mid_word >> 1);
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		u64 nb_sum_overflow = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = (mid_word << 1);
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		u64 carry_1_to_2 = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_overflow = nb_sum_overflow | carry_1_to_2;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = (lower_word >> 1);
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		carry_1_to_2 = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_overflow = nb_sum_overflow | carry_1_to_2;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;

		nb_to_add = lower_word;
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		carry_1_to_2 = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_overflow = nb_sum_overflow | carry_1_to_2;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;
		
		nb_to_add = (lower_word << 1);
		carry_0_to_1 = nb_sum_bit_0 & nb_to_add;
		carry_1_to_2 = nb_sum_bit_1 & carry_0_to_1;
		nb_sum_overflow = nb_sum_overflow | carry_1_to_2;
		nb_sum_bit_1 = nb_sum_bit_1 ^ carry_0_to_1;
		nb_sum_bit_0 = nb_sum_bit_0 ^ nb_to_add;
		
		u64 out_word = (~(nb_sum_overflow) & nb_sum_bit_1 & (nb_sum_bit_0 | mid_word)) & 0xfffffffffffffffeULL;
		or_of_out_words |= out_word;
		
		*out_entry = (*out_entry & 0x0000000000000001ULL) | out_word;
		out_entry += row_offset;
		
		row_count--;
		if (row_count <= 0)
			break;
		
		upper_word = mid_word;
		mid_word = lower_word;
		
		in_entry += row_offset;
		lower_word = *in_entry;
	}
	
	return or_of_out_words;
}

static void GoLGrid_evolve (const GoLGrid *in_gg, GoLGrid *out_gg)
{
	if (!in_gg || !in_gg->grid || !out_gg || !out_gg->grid || !GoLGrid_is_same_grid_specs (in_gg, out_gg))
		return (void) ffsc (__func__);
	
	if (in_gg->pop_x_off <= in_gg->pop_x_on)
		GoLGrid_clear (out_gg);
	else
	{
		int make_row_on = highest_of (in_gg->pop_y_on - 1, 1);
		int make_row_off = lowest_of (in_gg->pop_y_off + 1, in_gg->grid_rect.height - 1);
		
		if (in_gg->row_offset == 1)
		{
			if (out_gg->pop_x_on < out_gg->pop_x_off && out_gg->pop_y_on < make_row_on)
				GoLGrid_clear_one_col (out_gg->grid + out_gg->pop_y_on, 1, make_row_on - out_gg->pop_y_on);
			
			u64 or_of_strip = GoLGrid_evolve_strip_overwrite (in_gg->grid + make_row_on, out_gg->grid + make_row_on, 1, make_row_off - make_row_on);
			
			if (out_gg->pop_x_on < out_gg->pop_x_off && out_gg->pop_y_off > make_row_off)
				GoLGrid_clear_one_col (out_gg->grid + make_row_off, 1, out_gg->pop_y_off - make_row_off);
			
			if (or_of_strip == 0)
				GoLGrid_set_empty_population_rect (out_gg);
			else
			{
				out_gg->pop_x_on = least_significant_bit_u64 (or_of_strip);
				out_gg->pop_x_off = most_significant_bit_u64 (or_of_strip);
				GoLGrid_shrink_population_y_limits_single_col (out_gg, make_row_on, make_row_off);
			}
		}
		else
		{
			if (out_gg->pop_x_on < out_gg->pop_x_off && out_gg->pop_y_on < make_row_on)
				GoLGrid_clear_strip (out_gg, out_gg->pop_y_on, make_row_on, out_gg->pop_x_on >> 3, (out_gg->pop_x_off - 1) >> 3);
			
			// The byte columns in the output grid that need to be generated
			int make_col_first = highest_of (in_gg->pop_x_on - 1, 0) >> 3;
			int make_col_last = lowest_of (in_gg->pop_x_off, in_gg->grid_rect.width - 1) >> 3;
			
			// The number of one-byte-overlapping 64-bit strips that will be required (this uses -1 / 7 = 0)
			// One-byte-overlaps means that 64-bit words will be read and written on unaligned adresses. This is surprisingly cheap on recent CPU:s
			int strip_cnt = (((make_col_last - make_col_first) - 1) / 7) + 1;
			
			// The byte offset of the last 64-bit strip from the second to last. The normal offset for each pair of strips is 7 bytes, but this one may need to be reduced later, to not extend too far
			int last_strip_delta_offset = 7;
			
			// The number of extra byte columns that will be cleared on the left and right side of those that must be generated. We start with having these on the right side and adjust them later
			int left_cleared_cols = 0;
			int right_cleared_cols = (7 * strip_cnt) - (make_col_last - make_col_first);
			
//			printf ("Init : make_col_first = %d, make_col_last = %d, strip_cnt = %d,\n       last_strip_delta_offset = %d\n       left_cleared_cols = %d, right_cleared_cols = %d\n\n",
//					make_col_first, make_col_last, strip_cnt, last_strip_delta_offset, left_cleared_cols, right_cleared_cols);
			
			// If cleared byte columns extends too far to the right we adjust them to the left
			int overflow = (make_col_last + right_cleared_cols + 1) - (in_gg->grid_rect.width >> 3);
			if (overflow > 0)
			{
				left_cleared_cols += overflow;
				right_cleared_cols -= overflow;
				
				// If this caused cleared byte columns to extend too far to the left instead, we must increase the overlap of the last 64-bit strip generated
				overflow = left_cleared_cols - make_col_first;
				if (overflow > 0)
				{
					last_strip_delta_offset -= overflow;
					left_cleared_cols -= overflow;
				}
			}
			
//			printf ("Oflow: last_strip_delta_offset = %d\n       left_cleared_cols = %d, right_cleared_cols = %d\n\n", last_strip_delta_offset, left_cleared_cols, right_cleared_cols);
			
			// For efficiency we generate the output right into a previously used output grid, without clearing it of previous contents. The idea is that we can often adjust the columns of
			// the generated strip so that it will overwrite all or most of the previous contents of the output grid.
			
			int max_strip_adjust = right_cleared_cols;
			
//			printf ("Cl-01: max_strip_adjust = %d\n\n", max_strip_adjust);
			
			// First make sure the output grid isn't empty
			if (out_gg->pop_x_on < out_gg->pop_x_off)
			{
				// The byte columns in the output grid that need to be cleared somehow
				int clear_col_first = out_gg->pop_x_on >> 3;
				int clear_col_last = (out_gg->pop_x_off - 1) >> 3;
				
//				printf ("Cl-02: clear_col_first = %d, clear_col_last = %d\n\n", clear_col_first, clear_col_last);
				
				// Don't adjust left_cleared_cols and right_cleared_cols so much that an extra 64-bit (unaligned) column on the right side needs to be cleared, besides what's cleared by generating
				if (clear_col_last >= make_col_last)
					max_strip_adjust = lowest_of (max_strip_adjust, ((make_col_last + right_cleared_cols) - clear_col_last) & 0x00000007);
				
//				printf ("Cl-03: max_strip_adjust = %d\n\n", max_strip_adjust);
				
				// Try to adjust the generated columns to help clear the left side
				int uncleared_left_side = (make_col_first - left_cleared_cols) - clear_col_first;
				if (uncleared_left_side > 0)
				{
					int wanted_adjustment = uncleared_left_side & 0x00000007;
					if (wanted_adjustment > 0 && wanted_adjustment <= max_strip_adjust)
					{
						left_cleared_cols += wanted_adjustment;
						right_cleared_cols -= wanted_adjustment;
						max_strip_adjust -= wanted_adjustment;
					}
				}
				
				// If needed, clear the non-empty parts of the output grid that are to the left and right side of the generated strips
				if (clear_col_first < make_col_first - left_cleared_cols || clear_col_last > make_col_last + right_cleared_cols)
				{
					if (clear_col_last <= make_col_last + right_cleared_cols)
						GoLGrid_clear_strip (out_gg, make_row_on, make_row_off, clear_col_first, make_col_first - left_cleared_cols - 1);
					else if (clear_col_first >= make_col_first - left_cleared_cols)
						GoLGrid_clear_strip (out_gg, make_row_on, make_row_off, make_col_last + right_cleared_cols + 1, clear_col_last);
					else
						GoLGrid_clear_left_right_strips (out_gg, make_row_on, make_row_off, clear_col_first, make_col_first - left_cleared_cols - 1, make_col_last + right_cleared_cols + 1, clear_col_last);
				}
			}
			
//			printf ("Cl-04: left_cleared_cols = %d, right_cleared_cols = %d\n       max_strip_adjust = %d\n\n", left_cleared_cols, right_cleared_cols, max_strip_adjust);
			
			// If possible we adjust the first 64-bit strip to the left to make it 8-byte-aligned. This could improve performance on some CPU:s
			int wanted_adjustment = (make_col_first - left_cleared_cols) & 0x00000007;
			if (max_strip_adjust >= wanted_adjustment)
			{
				left_cleared_cols += wanted_adjustment;
				right_cleared_cols -= wanted_adjustment;
				max_strip_adjust -= wanted_adjustment;
			}
			
//			printf ("Align: left_cleared_cols = %d, right_cleared_cols = %d\n       max_strip_adjust = %d\n\n", left_cleared_cols, right_cleared_cols, max_strip_adjust);
			
			u8 *in_strip_entry = ((u8 *) (in_gg->grid + (in_gg->row_offset * make_row_on)) + (make_col_first - left_cleared_cols));
			u8 *out_strip_entry = ((u8 *) (out_gg->grid + (out_gg->row_offset * make_row_on)) + (make_col_first - left_cleared_cols));
			int bit_offset_of_strip = 8 * (make_col_first - left_cleared_cols);
			int new_pop_x_on = 0;
			int new_pop_x_off = 0;
			int found_leftmost_cell = FALSE;
			u64 bit_offset_of_last_nonempty_strip = 0;
			u64 or_of_last_nonempty_strip = 0;
			int strip_ix = 0;
			
			while (TRUE)
			{
				u64 or_of_strip;
				if (strip_ix == 0)
					or_of_strip = GoLGrid_evolve_strip_overwrite ((u64 *) in_strip_entry, (u64 *) out_strip_entry, out_gg->row_offset, make_row_off - make_row_on);
				else
					or_of_strip = GoLGrid_evolve_strip_keep_bit_0 ((u64 *) in_strip_entry, (u64 *) out_strip_entry, out_gg->row_offset, make_row_off - make_row_on);
				
				if (or_of_strip != 0)
				{
					if (!found_leftmost_cell)
					{
						new_pop_x_on = bit_offset_of_strip + least_significant_bit_u64 (or_of_strip);
						found_leftmost_cell = TRUE;
					}
					
					bit_offset_of_last_nonempty_strip = bit_offset_of_strip;
					or_of_last_nonempty_strip = or_of_strip;
				}
				
				strip_ix++;
				if (strip_ix >= strip_cnt)
				{
					if (found_leftmost_cell)
						new_pop_x_off = bit_offset_of_last_nonempty_strip + most_significant_bit_u64 (or_of_last_nonempty_strip) + 1;
					
					break;
				}
				
				u64 delta_offset = 7;
				if (strip_ix == strip_cnt - 1)
					delta_offset = (u64) last_strip_delta_offset;
				
				in_strip_entry += delta_offset;
				out_strip_entry += delta_offset;
				bit_offset_of_strip += (8 * delta_offset);
			}
			
//			printf ("Old rect: xon = %d, xoff = %d, yon = %d, yoff = %d\n", out_gg->pop_x_on, out_gg->pop_x_off, out_gg->pop_y_on, out_gg->pop_y_off);

			if (out_gg->pop_x_on < out_gg->pop_x_off && out_gg->pop_y_off > make_row_off)
				GoLGrid_clear_strip (out_gg, make_row_off, out_gg->pop_y_off, out_gg->pop_x_on >> 3, (out_gg->pop_x_off - 1) >> 3);
			
			if (!found_leftmost_cell)
				GoLGrid_set_empty_population_rect (out_gg);
			else
			{
				out_gg->pop_x_on = new_pop_x_on;
				out_gg->pop_x_off = new_pop_x_off;
				out_gg->pop_y_on = make_row_on;
				out_gg->pop_y_off = make_row_off;
				
				GoLGrid_increase_pop_y_on (out_gg);
				GoLGrid_decrease_pop_y_off (out_gg);
			}
			
//			printf ("New rect: xon = %d, xoff = %d, yon = %d, yoff = %d\n", out_gg->pop_x_on, out_gg->pop_x_off, out_gg->pop_y_on, out_gg->pop_y_off);
		}
	}
	
	out_gg->generation = in_gg->generation + 1;
}

/*
static void GoLGrid_get_extract (const GoLGrid *gg, int center_x, int center_y, GolSlate *slate)
{
	if (!gg || !gg->grid || !slate)
		return (void) ffsc (__func__);
	
	center_x -= gg->grid_rect.left_x;
	center_y -= gg->grid_rect.top_y;
	
	int x_on = center_x - 16;
	int x_off = center_x + 16;
	
	int y_on = center_y - (slate->height >> 1);
	int y_off = center_y + (slate->height >> 1) + 1;
	
	int copy_y_on = highest_of (y_on, 0);
	int copy_y_off = lowest_of (y_off, gg->grid_rect.height);
	
	// Pre-clear the slate if all the requested area falls outside the grid horizontally, or if part of it falls outside the grid vertically
	if (x_on >= gg->grid_rect.width || x_off <= 0 || y_on < copy_y_on || y_off > copy_y_off)
		GolSlate_clear (slate);
	
	// We can't continue if all the requested area falls outside the grid horizontally, but if all of it falls outside the grid vertically, it is safe
	if (x_on >= gg->grid_rect.width || x_off <= 0)
		return;
	
	int byte_col_on = x_on >> 3;
	int word_shift = x_on & 0x00000007;
	if (byte_col_on < 0)
	{
		word_shift += (8 * byte_col_on);
		byte_col_on = 0;
	}
	
	int byte_col_off = (lowest_of (x_off, gg->grid_rect.width) + 7) >> 3;
	int strip_start = byte_col_on - lowest_of (byte_col_on & 0x00000007, (byte_col_on - byte_col_off) & 0x00000007);
	word_shift += (8 * (byte_col_on - strip_start));
	
	u32 *grid_entry = slate->grid + (copy_y_on - y_on);
	int row_ix;
	
	if (word_shift >= 0)
		for (row_ix = copy_y_on; row_ix < copy_y_off; row_ix++)
			*grid_entry++ = (u32) ((*((u64 *) (((u8 *) (gg->grid + (gg->row_offset * row_ix))) + strip_start))) >> word_shift);
	else
		for (row_ix = copy_y_on; row_ix < copy_y_off; row_ix++)
			*grid_entry++ = (u32) ((*((u64 *) (((u8 *) (gg->grid + (gg->row_offset * row_ix))) + strip_start))) << (-word_shift));
}
*/

static int GoLGrid_to_cell_list (const GoLGrid *gg, CellList_s8 *cl, int max_cells)
{
	if (!gg || !gg->grid || !cl)
		return ffsc (__func__);
	
	int byte_col_on = gg->pop_x_on >> 3;
	int byte_col_off = (gg->pop_x_off + 7) >> 3;
	int strip_start = byte_col_on - lowest_of (byte_col_on & 0x00000007, (byte_col_on - byte_col_off) & 0x00000007);
	int strip_cnt = ((byte_col_off - byte_col_on) + 7) >> 3;
	
	int cell_cnt = 0;
	
	int row_ix;
	int col_ix;
	for (row_ix = gg->pop_y_on; row_ix < gg->pop_y_off; row_ix++)
		for (col_ix = 0; col_ix < strip_cnt; col_ix++)
		{
			u64 grid_word = *((u64 *) (((u8 *) (gg->grid + (gg->row_offset * row_ix) + col_ix)) + strip_start));
			while (grid_word != 0)
			{
				int first_bit = least_significant_bit_u64 (grid_word);
				int cell_x = gg->grid_rect.left_x + (8 * strip_start) + (64 * col_ix) + first_bit;
				int cell_y = gg->grid_rect.top_y + row_ix;
				
				if (cell_x < -128 || cell_x > 127 || cell_y < -128 || cell_y > 127 || cell_cnt >= max_cells)
				{
					cl->cell_count = 0;
					return FALSE;
				}
				
				cl->cell [cell_cnt].x = cell_x;
				cl->cell [cell_cnt].y = cell_y;
				cell_cnt++;
				
				grid_word &= ~(0x0000000000000001ULL << first_bit);
			}
		}
	
	cl->cell_count = cell_cnt;
	
	return TRUE;
}

static void GoLGrid_print (const GoLGrid *gg, const Rect *print_rect)
{
	if (!gg || !gg->grid || (print_rect != NULL && (print_rect->width < 0 || print_rect->height < 0)))
		return (void) ffsc (__func__);
	
	const Rect *pr = (print_rect ? print_rect : &gg->grid_rect);
	
	int y;
	int x;
	for (y = pr->top_y; y < pr->top_y + pr->height; y++)
	{
		for (x = pr->left_x; x < pr->left_x + pr->width; x++)
			if (GoLGrid_get_cell (gg, x, y) != 0)
				printf ("@");
			else
				printf (".");
			
			printf ("\n");
	}
	
	printf ("\n");
}

static void GoLGrid_print_life_history_symbol (char symbol, int count, int *line_length)
{
	if (!line_length)
		return (void) ffsc (__func__);
	
	if (count == 0)
		return;
	else if (count == 1)
	{
		printf ("%c", symbol);
		(*line_length)++;
	}
	else if (count > 1)
	{
		printf ("%d%c", count, symbol);
		(*line_length) += (1 + digits_in_number (count));
	}
	
	if (*line_length > 68)
	{
		printf ("\n");
		(*line_length) = 0;
	}
}

static void GoLGrid_print_life_history (const GoLGrid *cur_gg, const GoLGrid *start_gg, const GoLGrid *envelope_gg, const Rect *print_rect)
{
	if ((!cur_gg && !start_gg && !envelope_gg) || (cur_gg && !cur_gg->grid) || (start_gg && !start_gg->grid) || (envelope_gg && !envelope_gg->grid) ||
			(print_rect != NULL && (print_rect->width < 0 || print_rect->height < 0)))
		return (void) ffsc (__func__);
	
	const Rect *pr = (print_rect ? print_rect : &cur_gg->grid_rect);
	
	printf ("x = %d, y = %d, rule = LifeHistory\n", pr->width, pr->height);
	
	int line_length = 0;
	int unwritten_cell_state = 0;
	int unwritten_cell_count = 0;
	int unwritten_newline_count = 0;
	
	int y;
	int x;
	for (y = pr->top_y; y < pr->top_y + pr->height; y++)
	{
		for (x = pr->left_x; x < pr->left_x + pr->width; x++)
		{
			int cur_cell = (cur_gg ? GoLGrid_get_cell (cur_gg, x, y) : 0);
			int start_cell = (start_gg ? GoLGrid_get_cell (start_gg, x, y) : 0);
			int envelope_cell = (envelope_gg ? GoLGrid_get_cell (envelope_gg, x, y) : 0);
			
			int cell_state = cur_cell + 4 * start_cell;
			if (cell_state == 0 && envelope_cell != 0)
				cell_state = 2;
			
			if (unwritten_newline_count > 0 && cell_state != 0)
			{
				GoLGrid_print_life_history_symbol ('$', unwritten_newline_count, &line_length);
				unwritten_newline_count = 0;
			}
			
			if (unwritten_cell_count > 0 && cell_state != unwritten_cell_state)
			{
				char symbol = (unwritten_cell_state == 0 ? '.' : 'A' + (unwritten_cell_state - 1));
				GoLGrid_print_life_history_symbol (symbol, unwritten_cell_count, &line_length);
				
				unwritten_cell_count = 0;
			}
			
			unwritten_cell_state = cell_state;
			unwritten_cell_count++;
		}
		
		if (unwritten_cell_count > 0 && unwritten_cell_state != 0)
			GoLGrid_print_life_history_symbol ('A' + (unwritten_cell_state - 1), unwritten_cell_count, &line_length);
		
		unwritten_cell_count = 0;
		unwritten_newline_count++;
	}
	
	printf ("!\n");
}
