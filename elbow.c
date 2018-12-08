#include <stdlib.h>
#include <inttypes.h>
#include <memory.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#include "lib.c"
#include "hashtable.c"
#include "store.c"
#include "celllist.c"
#include "rect.c"
#include "golgrid.c"

#define CACHE_LINE_SIZE 64
#define MEM_PAGE_SIZE 4096
#define MAX_MAX_GLIDERS 128

static int result_cnt = 0;

static void or_object (GoLGrid *grid, int object_id, int x, int y)
{
	if (object_id == 0)
		GoLGrid_or_cell_list (grid, CellList_s8_block_cells (), x, y);
	else if (object_id == 1)
		GoLGrid_or_cell_list (grid, CellList_s8_hive_cells (0), x, y);
	else if (object_id == 2)
		GoLGrid_or_cell_list (grid, CellList_s8_hive_cells (1), x, y);
	else if (object_id == 3)
		GoLGrid_or_cell_list (grid, CellList_s8_boat_cells (0), x, y);
	else if (object_id == 4)
		GoLGrid_or_cell_list (grid, CellList_s8_boat_cells (1), x, y);
	else if (object_id == 5)
		GoLGrid_or_cell_list (grid, CellList_s8_boat_cells (2), x, y);
	else if (object_id == 6)
		GoLGrid_or_cell_list (grid, CellList_s8_boat_cells (3), x, y);
	else if (object_id == 7)
		GoLGrid_or_cell_list (grid, CellList_s8_pond_cells (), x, y);
}

static void or_elbow_object (GoLGrid *grid, int object_id, int diagonal)
{
	if (object_id == 0)
		GoLGrid_or_cell_list (grid, CellList_s8_block_cells (), -1 + diagonal, -4 + -(diagonal));
	else if (object_id == 1)
		GoLGrid_or_cell_list (grid, CellList_s8_hive_cells (1), -2 + diagonal, -5 + -(diagonal));
	else if (object_id == 2)
		GoLGrid_or_cell_list (grid, CellList_s8_boat_cells (2), -2 + diagonal, -5 + -(diagonal));
	else if (object_id == 3)
		GoLGrid_or_cell_list (grid, CellList_s8_pond_cells (), -2 + diagonal, -6 + -(diagonal));
	else if (object_id == 4)
		GoLGrid_or_cell_list (grid, CellList_s8_block_cells (), 3 + diagonal, -1 + -(diagonal));
	else if (object_id == 5)
		GoLGrid_or_cell_list (grid, CellList_s8_hive_cells (0), 3 + diagonal, -2 + -(diagonal));
	else if (object_id == 6)
		GoLGrid_or_cell_list (grid, CellList_s8_boat_cells (0), 3 + diagonal, -1 + -(diagonal));
	else if (object_id == 7)
		GoLGrid_or_cell_list (grid, CellList_s8_pond_cells (), 3 + diagonal, -2 + -(diagonal));
}

static void make_wanted_results_hashtable_glider (HashTable_u64 *ht, const RandomDataArray *rda, const Rect *grid_rect, int row_offset)
{
	GoLGrid grid;
	GoLGrid_create (&grid, grid_rect, row_offset, CACHE_LINE_SIZE, 0);
	
	int diag;
	int object_id;
	int timing;
	Glider gl;
	for (diag = -32; diag <= 32; diag++)
		for (object_id = 0; object_id < 8; object_id++)
			for (gl.dir = 0; gl.dir < 4; gl.dir++)
				for (gl.lane = -56; gl.lane <= 56; gl.lane++)
					for (timing = -(2 * gl.lane); timing >= -(2 * gl.lane) - 112; timing--)
					{
						fprintf (stderr, "This need to be fixed now!\n");
						exit (0);
						
						if (gl.dir == 0 || gl.dir == 3)
							gl.timing = timing;
						else
							gl.timing = -112 - timing;
						
						GoLGrid_clear (&grid);
						or_elbow_object (&grid, object_id, diag);
						GoLGrid_or_glider (&grid, &gl);
						HashTable_u64_store (ht, GoLGrid_get_hash_key (&grid, rda), 0, FALSE, NULL);
					}
	
	GoLGrid_free (&grid);
}

static void make_wanted_results_hashtable_hand (HashTable_u64 *ht, const RandomDataArray *rda, const Rect *grid_rect, int row_offset)
{
	GoLGrid grid;
	GoLGrid_create (&grid, grid_rect, row_offset, CACHE_LINE_SIZE, 0);
	
	int diag;
	int hand_x;
	int hand_y;
	int object_id1;
	int object_id2;
	for (diag = -56; diag <= 56; diag++)
		for (hand_x = -56; hand_x <= 56; hand_x++)
			for (hand_y = -56; hand_y <= 56; hand_y++)
				if (abs (hand_x + hand_y) > 16)
					for (object_id1 = 0; object_id1 < 8; object_id1++)
						for (object_id2 = 0; object_id2 < 8; object_id2++)
						{
							GoLGrid_clear (&grid);
							or_elbow_object (&grid, object_id1, diag);
							or_object (&grid, object_id2, hand_x, hand_y);
							HashTable_u64_store (ht, GoLGrid_get_hash_key (&grid, rda), 0, FALSE, NULL);
						}
	
	GoLGrid_free (&grid);
}

static void make_wanted_results_hashtable_dupl (HashTable_u64 *ht, const RandomDataArray *rda, const Rect *grid_rect, int row_offset)
{
	GoLGrid grid;
	GoLGrid_create (&grid, grid_rect, row_offset, CACHE_LINE_SIZE, 0);
	
	int diag1;
	int diag2;
	int object_id1;
	int object_id2;
	for (diag1 = -56; diag1 <= 56; diag1++)
		for (diag2 = diag1 + 4; diag2 <= 56; diag2++)
			for (object_id1 = 0; object_id1 < 8; object_id1++)
				for (object_id2 = 0; object_id2 < 8; object_id2++)
				{
					GoLGrid_clear (&grid);
					or_elbow_object (&grid, object_id1, diag1);
					or_elbow_object (&grid, object_id2, diag2);
					HashTable_u64_store (ht, GoLGrid_get_hash_key (&grid, rda), 0, FALSE, NULL);
				}
	
	GoLGrid_free (&grid);
}

static void make_wanted_results_hashtable_pi_elbows (HashTable_u64 *ht, const RandomDataArray *rda, const Rect *grid_rect, int row_offset)
{
	GoLGrid grid;
	GoLGrid_create (&grid, grid_rect, row_offset, CACHE_LINE_SIZE, 0);
	
	HashTable_u64_store (ht, GoLGrid_get_hash_key (&grid, rda), -99, FALSE, NULL);
	
	int diag;
	int object_id;
	for (diag = -56; diag <= 56; diag++)
		for (object_id = 0; object_id < 8; object_id++)
		{
			GoLGrid_clear (&grid);
			or_elbow_object (&grid, object_id, diag);
			HashTable_u64_store (ht, GoLGrid_get_hash_key (&grid, rda), 2 * diag, FALSE, NULL);
		}
	
	GoLGrid_free (&grid);
}

static int make_allowed_area (GoLGrid *allowed_area, double center_x, double center_y, double radius, int glider_margin, int max_x_y_sum)
{
	GoLGrid_or_filled_circle (allowed_area, center_x, center_y, radius);
	
	Glider gl;
	gl.dir = 1;
	gl.lane = 0;
	
	int max_glider_timing = 4 * ((int) ((0.5 * (center_y - center_x)) + (radius / sqrt (2))) + (2 + glider_margin));
	
	for (gl.timing = max_glider_timing; gl.timing >= 0; gl.timing--)
		GoLGrid_or_glider (allowed_area, &gl);
	
	int x;
	int y;
	for (x = -64; x < 64; x++)
		for (y = -64; y < 64; y++)
			if (x + y > max_x_y_sum)
				GoLGrid_set_cell_off (allowed_area, x, y);
	
	return max_glider_timing;
}

static void make_unreactive_area (GoLGrid *unreactive_area)
{
	int y;
	int x;
	for (y = -64; y < 64; y++)
		for (x = -64; x < 64; x++)
			if (x + y <= -6 || x + y >= 5)
				GoLGrid_set_cell_on (unreactive_area, x, y);
	
	GoLGrid_print_life_history (unreactive_area, NULL, NULL, NULL);
}

static void print_recipe (FILE *stream, int *recipe, int glider_cnt)
{
	int phase = 0;
	
	fprintf (stream, "{");
	int glider_ix;
	for (glider_ix = 0; glider_ix < glider_cnt; glider_ix++)
	{
		int delay = recipe [glider_ix];
		if (delay == 0)
		{
			if (phase == 0)
				delay = 256;
			else
				delay = 255;
		}
		else if (delay == 1)
		{
			if (phase == 0)
				delay = 255;
			else
				delay = 256;
		}
		
		phase = (phase + delay) % 2;
		
		if (glider_ix > 0)
			fprintf (stream, "%3d,", delay);
	}
	fprintf (stream, "  0},");
}

static int store_pattern (const GoLGrid *gg, int *recipe, int glider_cnt, ByteSeqStore *bss)
{
	u16 cl_buf [1025];
	CellList_s8 *cl = (CellList_s8 *) cl_buf;
	
	// Shouldn't be needed, but compiler insists...
	cl->cell_count = 0;
	
	if (!GoLGrid_to_cell_list (gg, cl, 1024))
		return FALSE;
	
	int byte_seq_ix = 0;
	u8 byte_seq [3072];
	
	byte_seq [byte_seq_ix++] = glider_cnt;
	
	int glider_ix;
	for (glider_ix = 0; glider_ix < glider_cnt; glider_ix++)
		byte_seq [byte_seq_ix++] = (u8) recipe [glider_ix];
	
	int cell_ix;
	for (cell_ix = 0; cell_ix < cl->cell_count; cell_ix++)
	{
		byte_seq [byte_seq_ix++] = (u8) cl->cell [cell_ix].x;
		byte_seq [byte_seq_ix++] = (u8) cl->cell [cell_ix].y;
	}	
	
	ByteSeqStore_store (bss, byte_seq, byte_seq_ix);
	
	return TRUE;
}

static void byte_seq_to_pattern (u8 *byte_seq, int byte_seq_size, int *recipe, int *glider_cnt, GoLGrid *gg)
{
	*glider_cnt = byte_seq [0];
	int cells_offset = 1 + *glider_cnt;
	
	int glider_ix;
	for (glider_ix = 0; glider_ix < *glider_cnt; glider_ix++)
		recipe [glider_ix] = byte_seq [1 + glider_ix];
	
	GoLGrid_clear (gg);
	
	int cell_cnt = (byte_seq_size - cells_offset) / 2;
	int cell_ix;
	for (cell_ix = 0; cell_ix < cell_cnt; cell_ix++)
		GoLGrid_set_cell_on (gg, (s8) byte_seq [cells_offset + (2 * cell_ix)], (s8) byte_seq [cells_offset + (2 * cell_ix) + 1]);
}

static void run_pattern (const GoLGrid *tested_pattern, const GoLGrid *elbow_pattern, GoLGrid evolve_gg [3], int *recipe, int first_glider_ix, int gliders_in_salvo, const RandomDataArray *rda, 
		const HashTable_u64 *wanted_results, HashTable_u64 *seen_patterns, const GoLGrid *allowed_area, const GoLGrid *unreactive_area, int glider_introduction_timing, int settle_wait_time,
		ByteSeqStore settled_store [])
{
	GoLGrid_clear (&evolve_gg [0]);
	GoLGrid_or (tested_pattern, &evolve_gg [0]);
	
	Glider gl;
	gl.dir = 1;
	gl.lane = 0;
	
	int salvo_glider_ix = 0;
	int next_glider_introduction_time = recipe [first_glider_ix + salvo_glider_ix];
	
	int gen = 0;
	while (TRUE)
	{
		if (salvo_glider_ix < gliders_in_salvo && gen >= next_glider_introduction_time)
		{
			gl.timing = next_glider_introduction_time + glider_introduction_timing;
			GoLGrid_or_glider (&evolve_gg [gen % 3], &gl);
			
			salvo_glider_ix++;
			if (salvo_glider_ix < gliders_in_salvo)
				next_glider_introduction_time += recipe [first_glider_ix + salvo_glider_ix];
		}
		
		GoLGrid_evolve (&evolve_gg [gen % 3], &evolve_gg [(gen + 1) % 3]);
		gen++;
		
		if (!GoLGrid_is_subset (&evolve_gg [gen % 3], allowed_area))
		{
//			u64 hash_key = GoLGrid_get_hash_key (&evolve_gg [gen % 3], rda);
//			
//			// Note: get instead of store
//			if (!HashTable_u64_get_data (seen_patterns, hash_key, NULL))
//			{
//				u64 move_dist;
//				if (HashTable_u64_get_data (wanted_results, hash_key, &move_dist))
//				{
//					result_cnt++;
//					
//					// Now we store it
//					HashTable_u64_store (seen_patterns, hash_key, 0, FALSE, NULL);
//					
//					printf ("------------- Result -------------\n/* %+03d,%02d */  ", (int) move_dist, first_glider_ix + gliders_in_salvo);
//					print_recipe (stdout, recipe, first_glider_ix + gliders_in_salvo);
//					printf ("\n");
//					
//					GoLGrid_print_life_history (&evolve_gg [gen % 3], elbow_pattern, NULL, NULL);
//					printf ("\n");
//				}
//			}
//			
			break;
		}
		
		if ((gen % 2) == 0 && salvo_glider_ix >= gliders_in_salvo && GoLGrid_is_equal_to (&evolve_gg [gen % 3], &evolve_gg [((gen - 2) + 3) % 3]))
		{
			u64 hash_key = GoLGrid_get_hash_key (&evolve_gg [gen % 3], rda);
			
			// Shouldn't be necessary...
			int was_present = FALSE;
			HashTable_u64_store (seen_patterns, hash_key, 0, FALSE, &was_present);
			
			if (!was_present)
			{
				u64 move_dist;
				if (HashTable_u64_get_data (wanted_results, hash_key, &move_dist))
				{
					result_cnt++;
					
					printf ("------------- Result -------------\n/* %+03d,%02d */  ", (int) move_dist, first_glider_ix + gliders_in_salvo);
					print_recipe (stdout, recipe, first_glider_ix + gliders_in_salvo);
					printf ("\n");
					
					GoLGrid_print_life_history (&evolve_gg [gen % 3], elbow_pattern, NULL, NULL);
					printf ("\n");
				}
				
				if (!GoLGrid_is_subset (&evolve_gg [gen % 3], unreactive_area) || !GoLGrid_is_subset (&evolve_gg [((gen - 1) + 3) % 3], unreactive_area))
					store_pattern (&evolve_gg [gen % 3], recipe, first_glider_ix + gliders_in_salvo, &settled_store [first_glider_ix + gliders_in_salvo - 1]);
			}
			
			break;
		}
		
		if (salvo_glider_ix >= gliders_in_salvo && (gen - next_glider_introduction_time) > settle_wait_time)
			break;
	}
}

static void run_store (const ByteSeqStore *in_bss, ByteSeqStore out_bss [], int first_glider_ix, int max_salvo, int in_patterns_p1_only, const Rect *grid_rect, int row_offset, const GoLGrid *elbow_pattern,
		const RandomDataArray *rda, const HashTable_u64 *wanted_results, HashTable_u64 *seen_patterns, const GoLGrid *allowed_area, const GoLGrid *unreactive_area, int glider_introduction_timing,
		int min_glider_dist, int max_glider_dist, int settle_wait_time, int status_update_interval)
{
	GoLGrid start_pattern;
	GoLGrid_create (&start_pattern, grid_rect, row_offset, MEM_PAGE_SIZE, 0);
	
	GoLGrid evolve_gg [3];
	int patt_ix;
	for (patt_ix = 0; patt_ix < 3; patt_ix++)
		GoLGrid_create (&evolve_gg [patt_ix], grid_rect, row_offset, MEM_PAGE_SIZE, 0);
	
	ByteSeqStoreNode *bss_node;
	int node_data_offset;
	ByteSeqStore_start_get_iteration (in_bss, &bss_node, &node_data_offset);
	
	u8 byte_seq [3072];
	int seq_size;
	int recipe [MAX_MAX_GLIDERS];
	int glider_cnt;
	int run_cnt = 0;	
	
	while (TRUE)
	{
		if (!ByteSeqStore_get_next (in_bss, &bss_node, &node_data_offset, byte_seq, 3072, &seq_size))
			break;
		
		if (run_cnt % status_update_interval == 0)
			fprintf (stderr, "Testing pattern %d (singletons)\n", run_cnt);
		
		run_cnt++;
		
		GoLGrid_clear (&start_pattern);
		byte_seq_to_pattern (byte_seq, seq_size, recipe, &glider_cnt, &start_pattern);
		
		recipe [first_glider_ix] = 0;
		run_pattern (&start_pattern, elbow_pattern, evolve_gg, recipe, first_glider_ix, 1, rda, wanted_results, seen_patterns, allowed_area, unreactive_area, glider_introduction_timing, settle_wait_time, out_bss);
		
		recipe [first_glider_ix] = 1;
		if (!in_patterns_p1_only)
			run_pattern (&start_pattern, elbow_pattern, evolve_gg, recipe, first_glider_ix, 1, rda, wanted_results, seen_patterns, allowed_area, unreactive_area, glider_introduction_timing, settle_wait_time, out_bss);
	}
	
	if (max_salvo > 1)
	{
		ByteSeqStore_start_get_iteration (in_bss, &bss_node, &node_data_offset);
		run_cnt = 0;
		
		while (TRUE)
		{
			if (!ByteSeqStore_get_next (in_bss, &bss_node, &node_data_offset, byte_seq, 3072, &seq_size))
				break;
			
			if (run_cnt % status_update_interval == 0)
				fprintf (stderr, "Testing pattern %d (pairs)\n", run_cnt);
			
			run_cnt++;
			
			GoLGrid_clear (&start_pattern);
			byte_seq_to_pattern (byte_seq, seq_size, recipe, &glider_cnt, &start_pattern);
			
			recipe [first_glider_ix] = 0;
			int glider_dist;
			for (glider_dist = min_glider_dist; glider_dist <= max_glider_dist; glider_dist++)
			{
				recipe [first_glider_ix + 1] = glider_dist;
				run_pattern (&start_pattern, elbow_pattern, evolve_gg, recipe, first_glider_ix, 2, rda, wanted_results, seen_patterns, allowed_area, unreactive_area, glider_introduction_timing, settle_wait_time, out_bss);
			}
			
			recipe [first_glider_ix] = 1;
			if (!in_patterns_p1_only)
				for (glider_dist = min_glider_dist; glider_dist <= max_glider_dist; glider_dist++)
				{
					recipe [first_glider_ix + 1] = glider_dist;
					run_pattern (&start_pattern, elbow_pattern, evolve_gg, recipe, first_glider_ix, 2, rda, wanted_results, seen_patterns, allowed_area, unreactive_area, glider_introduction_timing, settle_wait_time, out_bss);
				}
		}
	}
}

static int count_fit (const ByteSeqStore *bss, const GoLGrid *size_category, const Rect *grid_rect, int row_offset)
{
	GoLGrid gg;
	GoLGrid_create (&gg, grid_rect, row_offset, CACHE_LINE_SIZE, 0);
	
	ByteSeqStoreNode *bss_node;
	int node_data_offset;
	ByteSeqStore_start_get_iteration (bss, &bss_node, &node_data_offset);
	
	int fit_cnt = 0;	
	while (TRUE)
	{
		u8 byte_seq [3072];
		int seq_size;
		if (!ByteSeqStore_get_next (bss, &bss_node, &node_data_offset, byte_seq, 3072, &seq_size))
			break;
		
		int recipe [MAX_MAX_GLIDERS];
		int glider_cnt;
		
		GoLGrid_clear (&gg);
		byte_seq_to_pattern (byte_seq, seq_size, recipe, &glider_cnt, &gg);
		
		if (GoLGrid_is_subset (&gg, size_category))
			fit_cnt++;
	}
	
	return fit_cnt;
}

static void filter_bss (const ByteSeqStore *in_bss, const GoLGrid *size_category, ByteSeqStore *out_bss, const Rect *grid_rect, int row_offset)
{
	GoLGrid gg;
	GoLGrid_create (&gg, grid_rect, row_offset, CACHE_LINE_SIZE, 0);
	
	ByteSeqStoreNode *bss_node;
	int node_data_offset;
	ByteSeqStore_start_get_iteration (in_bss, &bss_node, &node_data_offset);
	
	while (TRUE)
	{
		u8 byte_seq [3072];
		int seq_size;
		if (!ByteSeqStore_get_next (in_bss, &bss_node, &node_data_offset, byte_seq, 3072, &seq_size))
			break;
		
		int recipe [MAX_MAX_GLIDERS];
		int glider_cnt;
		
		GoLGrid_clear (&gg);
		byte_seq_to_pattern (byte_seq, seq_size, recipe, &glider_cnt, &gg);
		
		if (GoLGrid_is_subset (&gg, size_category))
			store_pattern (&gg, recipe, glider_cnt, out_bss);
	}
}

int main (int argc, char *argv[])
{
	Rect grid_rect;
	Rect_make (&grid_rect, -96, -64, 192, 128);
	
	int row_offset = grid_rect.width / 8;
	GoLGrid elbow_pattern;
	
	// We at least check the result of the first create, to make sure we're running on a little-endian machine
	if (!GoLGrid_create (&elbow_pattern, &grid_rect, row_offset, MEM_PAGE_SIZE, 0))
		return EXIT_FAILURE;
	
	u16 cl_buf [1025];
	CellList_s8 *cl = (CellList_s8 *) cl_buf;

	for (int i = 1; i < argc; i += 2) {
	  cl->cell[i / 2].x = atoi(argv[i]);
	  cl->cell[i / 2].y = atoi(argv[i+1]);
	}

	cl->cell_count = argc / 2;
	
	if (argc > 1)
	  GoLGrid_or_cell_list(&elbow_pattern, cl, 0, 0);
	else
	
	GoLGrid_or_cell_list (&elbow_pattern, CellList_s8_block_cells (), -1, -4);
//	GoLGrid_or_cell_list (&elbow_pattern, CellList_s8_block_cells (), 3, -1);
//	GoLGrid_or_cell_list (&elbow_pattern, CellList_s8_hive_cells (0), 0, -8);
//	GoLGrid_or_cell_list (&elbow_pattern, CellList_s8_hive_cells (0), 0, 1);
//	GoLGrid_or_cell_list (&elbow_pattern, CellList_s8_hive_cells (1), -5, -3);
//	GoLGrid_or_cell_list (&elbow_pattern, CellList_s8_hive_cells (1), 4, -3);
	
//	GoLGrid_or_cell_list (&elbow_pattern, CellList_s8_hive_cells (0), 3, 1);

// Boat:
//	GoLGrid_set_cell_on (&elbow_pattern, 1, -6);
//	GoLGrid_set_cell_on (&elbow_pattern, 0, -5);
//	GoLGrid_set_cell_on (&elbow_pattern, 2, -5);
//	GoLGrid_set_cell_on (&elbow_pattern, 0, -4);
//	GoLGrid_set_cell_on (&elbow_pattern, 1, -4);
	
	ByteSeqStore filtered_store;
	ByteSeqStore_create (&filtered_store, 65536);
	
	store_pattern (&elbow_pattern, NULL, 0, &filtered_store);
	
	RandomDataArray rda;
	RandomDataArray_create (&rda, row_offset * grid_rect.height);
	
	HashTable_u64 wanted_results;
	HashTable_u64_create (&wanted_results, 64);
	make_wanted_results_hashtable_pi_elbows (&wanted_results, &rda, &grid_rect, row_offset);
	
	HashTable_u64 seen_results;
	HashTable_u64_create (&seen_results, 64);
	
	int elbow_is_p1 = TRUE;
	int max_gliders = 24;
	int max_glider_salvo = 2;
	int min_glider_dist = 90;
	int max_glider_dist = 176;
	int settle_wait_time = 352;
	int min_pool_size = 100000;
	
	double center_x = 0.0;
	double center_y = 0.0;
	double allowed_radius = 42.50;
	int max_x_y_sum = 128;
	
	int status_update_interval = 5000;
	
	GoLGrid allowed_area_gg;
	GoLGrid_create (&allowed_area_gg, &grid_rect, row_offset, CACHE_LINE_SIZE, 0);
	int glider_introduction_timing = make_allowed_area (&allowed_area_gg, center_x, center_y, allowed_radius, 3, max_x_y_sum);
	
	GoLGrid size_category_gg [129];
	int cat_ix;
	for (cat_ix = 0; cat_ix <= 128; cat_ix++)
	{
		GoLGrid_create (&size_category_gg [cat_ix], &grid_rect, row_offset, CACHE_LINE_SIZE, 0);
		GoLGrid_or_filled_circle (&size_category_gg [cat_ix], center_x, center_y, allowed_radius * ((double) (cat_ix) / 128.0));
	}
	
	GoLGrid unreactive_area_gg;
	GoLGrid_create (&unreactive_area_gg, &grid_rect, row_offset, CACHE_LINE_SIZE, 0);
	make_unreactive_area (&unreactive_area_gg);
	
	ByteSeqStore pattern_store [MAX_MAX_GLIDERS];
	int store_ix;
	for (store_ix = 0; store_ix < max_gliders; store_ix++)
		ByteSeqStore_create (&pattern_store [store_ix], 65536);
	
	cat_ix = 128;
	store_ix = 0;
	while (TRUE)
	{
		fprintf (stderr, "Starting depth %d, %d filtered patterns, radius = %2.2f\n", store_ix + 1, (int) filtered_store.seq_count, allowed_radius * ((double) (cat_ix) / 128.0));
		
		int max_new_gliders = lowest_of (max_glider_salvo, max_gliders - store_ix);
		int is_p1_only = (store_ix == 0 && elbow_is_p1);
		
		run_store (&filtered_store, pattern_store, store_ix, max_new_gliders, is_p1_only, &grid_rect, row_offset, &elbow_pattern, &rda, &wanted_results, &seen_results, &allowed_area_gg, &unreactive_area_gg, 
				glider_introduction_timing, min_glider_dist, max_glider_dist, settle_wait_time, status_update_interval);
		
		if (store_ix > 0)
			ByteSeqStore_free (&pattern_store [store_ix - 1]);
		
		fprintf (stderr, "Results so far: %d\n\n", result_cnt);
		
		if (store_ix + 1 >= max_gliders)
			break;
		
		for (cat_ix = 0; cat_ix < 128; cat_ix++)
			if (count_fit (&pattern_store [store_ix], &size_category_gg [cat_ix], &grid_rect, row_offset) > min_pool_size)
				break;
		
		ByteSeqStore_clear (&filtered_store);
		filter_bss (&pattern_store [store_ix], &size_category_gg [cat_ix], &filtered_store, &grid_rect, row_offset);
		
		store_ix++;
	}
	
	return EXIT_SUCCESS;
}
