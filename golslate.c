// A simple representation of a small Game of Life object or subpattern
// The width is always 32 cells, and the height is always an odd number
// X coordinates go from -16 to 15, and y coordinated från -(height >> 1) to (height >> 1)

typedef struct
{
	int height;
	u32 *grid;
} GolSlate;

Work in progress... remove references to GoLGrid, etc.

static void GoLSlate_bleed_sqroot_5 (const GoLGrid *gg, int orig_oncell_x, int orig_oncell_y, int area_height, int max_rounds, u32 *obj)
{
	if (!gg || !gg->grid || area_height < 1 || area_height > 31 || (area_height & 0x00000001) == 0 || max_rounds < 1 || !obj)
		return (void) ffsc (__func__);
	
// What's min ok area_height?

	u32 excerpt [31];
	GoLGrid_get_excerpt (gg, orig_oncell_x, orig_oncell_y, area_height, excerpt);
	printf ("Excerpt:\n");
	print_excerpt (excerpt, area_height);
	
	int obj_row_on = (area_height >> 1) - 2;
	int obj_row_off = (area_height >> 1) + 3;
	
	printf ("obj_row_on = %d, obj_row_off = %d\n", obj_row_on, obj_row_off);
	
	u32 mask [31];
	
// remove
memset (obj, 0xff, 4 * area_height);	
memset (mask, 0xff, 124);	
	
	mask [obj_row_on + 0] = 0x00038000;
	mask [obj_row_on + 1] = 0x0007c000;
	mask [obj_row_on + 2] = 0x0007c000;
	mask [obj_row_on + 3] = 0x0007c000;
	mask [obj_row_on + 4] = 0x00038000;
	
	printf ("First mask:\n");
	print_excerpt (mask, area_height);
	
	int round_ix = 1;
	u64 prev_obj_row_sum = 0x0000000000010000ULL;
	
	while (TRUE)
	{
		u64 obj_row_sum = 0;
		int row_ix;
		
		for (row_ix = obj_row_on; row_ix < obj_row_off; row_ix++)
		{
			u32 masked_word = excerpt [row_ix] & mask [row_ix];
			obj [row_ix] = masked_word;
			obj_row_sum += masked_word;
		}
		
		printf ("round_ix = %d, prev_obj_row_sum = %d, obj_row_sum = %d\n", round_ix, (u32) prev_obj_row_sum, (u32) obj_row_sum);
		printf ("New object bleed:\n");
		print_excerpt (obj, area_height);
		
		if (obj_row_sum == prev_obj_row_sum || round_ix >= max_rounds)
			break;
		
		prev_obj_row_sum = obj_row_sum;
			
		u32 mask_word_m2 = 0;
		u32 mask_word_m1 = 0;
		u32	mask_word_p0 = 0;
		u32	mask_word_p1 = 0;
		u32 mask_word_p2;
		
		row_ix = obj_row_on;
		while (TRUE)
		{
			u32 obj_word = obj [row_ix];
			u32 obj_word_nbhood_1 = (obj_word << 1) | obj_word | (obj_word >> 1);
			u32 obj_word_nbhood_2 = (obj_word << 2) | obj_word_nbhood_1 | (obj_word >> 2);
			
			mask_word_m2 = mask_word_m2 | obj_word_nbhood_1;
			mask_word_m1 = mask_word_m1 | obj_word_nbhood_2;
			mask_word_p0 = mask_word_p0 | obj_word_nbhood_2;
			mask_word_p1 = mask_word_p1 | obj_word_nbhood_2;
			mask_word_p2 = obj_word_nbhood_1;
			
			mask [row_ix - 2] = mask_word_m2;
			
			printf ("--- row_ix = %d\n", row_ix);
			
			if (row_ix >= obj_row_off - 1)
				break;
			
			row_ix++;
			
			mask_word_m2 = mask_word_m1;
			mask_word_m1 = mask_word_p0;
			mask_word_p0 = mask_word_p1;
			mask_word_p1 = mask_word_p2;
		}
		
		mask [row_ix - 1] = mask_word_m1;
		mask [row_ix + 0] = mask_word_p0;
		mask [row_ix + 1] = mask_word_p1;
		mask [row_ix + 2] = mask_word_p2;
		
		printf ("New mask:\n");
		print_excerpt (mask, area_height);
	
		obj_row_on = highest_of (obj_row_on - 2, 2);
		obj_row_off = lowest_of (obj_row_off + 2, area_height - 2);
		
		printf ("obj_row_on = %d, obj_row_off = %d\n", obj_row_on, obj_row_off);
		
		round_ix++;
	}
}

static void print_excerpt (const u32 *excerpt, int height)
{
	int y;
	int x;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < 32; x++)
			if (((excerpt [y] >> x) & 1) != 0)
				printf ("@");
			else
				printf (".");
		
		printf ("\n");
	}
}
