// A description of a glider
// (dir) is 0 for a NW, 1 for a NE, 2 for a SE and 3 for SW-bound glider
// (lane) is the x-coordinate for the center cell of the glider if it is moved backwards or forwards in time, so that its center cell has y-coordinate 0
// and it is in the phase with three cells in a horizontal line
// (timing) is the generation if the glider is moved backwards or forwards in time, so that its center cell has x-coordinate 0 (instead of y-coordinate
// as for (lane)) and with the same phase as above

typedef struct
{
	int dir;
	int lane;
	int timing;
} Glider;

// A description of a standard spaceship
// (size) is 0 for glider, 1 for LWSS, 2 for MWSS and 3 for HWSS
// For a glider, (dir), (lane) and (timing) is defined as for the Glider type above, and (sync) is 0
// For a XWSS, (dir) is 0 for a N, 1 for an E, 2 for a S and 3 for a W-bound ship
// (lane) is the x- or y-coordinate of the symmetry axis of the different phases
// (timing) is the generation if the spaceship is moved backwards or forwards in time, so that it is in the phase with the low population count, and the off-cell with 5 on-cell neighbours has y-coordinate 0
// for a N or S-bound ship, or x-coordinate 0 for a W or E-bound ship
// (sync) is 0 if the "arrow" in the phase with the low population count points towards NW or SE when generation = (timing), and 1 if it points towards NE or SW at that time

// typedef etc...

typedef struct
{
	s8 x;
	s8 y;
} CellCoord_s8;

typedef struct
{
	u16 cell_count;
	CellCoord_s8 cell [];
} CellList_s8;

static const CellList_s8 CellList_s8_glider_cells_00 = {5, {{-1, -1}, { 0, -1}, { 1, -1}, {-1,  0}, { 0,  1}}};
static const CellList_s8 CellList_s8_glider_cells_01 = {5, {{ 0, -1}, { 1, -1}, {-1,  0}, { 0,  0}, { 1,  1}}};
static const CellList_s8 CellList_s8_glider_cells_02 = {5, {{ 0, -1}, { 1, -1}, { 0,  0}, { 2,  0}, { 0,  1}}};
static const CellList_s8 CellList_s8_glider_cells_03 = {5, {{ 1, -1}, { 0,  0}, { 1,  0}, { 0,  1}, { 2,  1}}};
static const CellList_s8 CellList_s8_glider_cells_10 = {5, {{-1, -1}, { 0, -1}, { 1, -1}, { 1,  0}, { 0,  1}}};
static const CellList_s8 CellList_s8_glider_cells_11 = {5, {{-1, -1}, { 0, -1}, { 0,  0}, { 1,  0}, {-1,  1}}};
static const CellList_s8 CellList_s8_glider_cells_12 = {5, {{-1, -1}, { 0, -1}, {-2,  0}, { 0,  0}, { 0,  1}}};
static const CellList_s8 CellList_s8_glider_cells_13 = {5, {{-1, -1}, {-1,  0}, { 0,  0}, {-2,  1}, { 0,  1}}};
static const CellList_s8 CellList_s8_glider_cells_20 = {5, {{ 0, -1}, { 1,  0}, {-1,  1}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_glider_cells_21 = {5, {{-1, -1}, { 0,  0}, { 1,  0}, {-1,  1}, { 0,  1}}};
static const CellList_s8 CellList_s8_glider_cells_22 = {5, {{ 0, -1}, {-2,  0}, { 0,  0}, {-1,  1}, { 0,  1}}};
static const CellList_s8 CellList_s8_glider_cells_23 = {5, {{-2, -1}, { 0, -1}, {-1,  0}, { 0,  0}, {-1,  1}}};
static const CellList_s8 CellList_s8_glider_cells_30 = {5, {{ 0, -1}, {-1,  0}, {-1,  1}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_glider_cells_31 = {5, {{ 1, -1}, {-1,  0}, { 0,  0}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_glider_cells_32 = {5, {{ 0, -1}, { 0,  0}, { 2,  0}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_glider_cells_33 = {5, {{ 0, -1}, { 2, -1}, { 0,  0}, { 1,  0}, { 1,  1}}};

static const CellList_s8 *CellList_s8_glider_cells_nn [4] [4] =
		{{&CellList_s8_glider_cells_00, &CellList_s8_glider_cells_01, &CellList_s8_glider_cells_02, &CellList_s8_glider_cells_03},
		 {&CellList_s8_glider_cells_10, &CellList_s8_glider_cells_11, &CellList_s8_glider_cells_12, &CellList_s8_glider_cells_13},
		 {&CellList_s8_glider_cells_20, &CellList_s8_glider_cells_21, &CellList_s8_glider_cells_22, &CellList_s8_glider_cells_23},
		 {&CellList_s8_glider_cells_30, &CellList_s8_glider_cells_31, &CellList_s8_glider_cells_32, &CellList_s8_glider_cells_33}};

static const int CellList_s8_glider_dir [4] [2] =
{{ 1,  1}, {-1,  1}, {-1, -1}, { 1, -1}};

static const CellList_s8 *CellList_s8_glider_cells (const Glider *gl, int generation, int *x_to_add, int *y_to_add)
{
	if (!gl || gl->dir < 0 || gl->dir > 3 || !x_to_add || !y_to_add)
		return ffsc_p (__func__);
	
	int gen0_timing = gl->timing - generation;
	*x_to_add = CellList_s8_glider_dir [gl->dir] [0] * (gen0_timing >> 2);
	*y_to_add = (CellList_s8_glider_dir [gl->dir] [1] * (gen0_timing >> 2)) - (CellList_s8_glider_dir [gl->dir] [0] * CellList_s8_glider_dir [gl->dir] [1] * gl->lane);
	return CellList_s8_glider_cells_nn [gl->dir] [gen0_timing & 0x00000003];
}

/*
static const CellList_s8 CellList_s8_lwss_cells_00 = { 9, {{-1, -1}, { 0, -1}, { 1, -1}, {-1,  0}, { 2,  0}, {-1,  1}, {-1,  2}, { 0,  3}, { 2,  3}}};
static const CellList_s8 CellList_s8_lwss_cells_01 = {12, {{ 0, -1}, {-1,  0}, { 0,  0}, { 1,  0}, {-1,  1}, { 1,  1}, { 2,  1}, { 0,  2}, { 1,  2}, { 2,  2}, { 0,  3}, { 1,  3}}};
static const CellList_s8 CellList_s8_lwss_cells_02 = { 9, {{-1,  0}, { 0,  0}, { 1,  0}, {-2,  1}, { 1,  1}, { 1,  2}, { 1,  3}, {-2,  4}, { 0,  4}}};
static const CellList_s8 CellList_s8_lwss_cells_03 = {12, {{ 0,  0}, {-1,  1}, { 0,  1}, { 1,  1}, {-2,  2}, {-1,  2}, { 1,  2}, {-2,  3}, {-1,  3}, { 0,  3}, {-1,  4}, { 0,  4}}};
static const CellList_s8 CellList_s8_lwss_cells_10 = { 9, {{-3, -2}, { 0, -2}, { 1, -1}, {-3,  0}, { 1,  0}, {-2,  1}, {-1,  1}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_lwss_cells_11 = {12, {{-2, -2}, {-1, -2}, {-3, -1}, {-2, -1}, {-1, -1}, { 0, -1}, {-3,  0}, {-2,  0}, { 0,  0}, { 1,  0}, {-1,  1}, { 0,  1}}};
static const CellList_s8 CellList_s8_lwss_cells_12 = { 9, {{-3, -1}, {-2, -1}, {-1, -1}, { 0, -1}, {-4,  0}, { 0,  0}, { 0,  1}, {-4,  2}, {-1,  2}}};
static const CellList_s8 CellList_s8_lwss_cells_13 = {12, {{-2, -1}, {-1, -1}, {-4,  0}, {-3,  0}, {-1,  0}, { 0,  0}, {-4,  1}, {-3,  1}, {-2,  1}, {-1,  1}, {-3,  2}, {-2,  2}}};
static const CellList_s8 CellList_s8_lwss_cells_20 = { 9, {{-2, -3}, { 0, -3}, { 1, -2}, { 1, -1}, {-2,  0}, { 1,  0}, {-1,  1}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_lwss_cells_21 = {12, {{-1, -3}, { 0, -3}, {-2, -2}, {-1, -2}, { 0, -2}, {-2, -1}, {-1, -1}, { 1, -1}, {-1,  0}, { 0,  0}, { 1,  0}, { 0,  1}}};
static const CellList_s8 CellList_s8_lwss_cells_22 = { 9, {{ 0, -4}, { 2, -4}, {-1, -3}, {-1, -2}, {-1, -1}, { 2, -1}, {-1,  0}, { 0,  0}, { 1,  0}}};
static const CellList_s8 CellList_s8_lwss_cells_23 = {12, {{ 0, -4}, { 1, -4}, { 0, -3}, { 1, -3}, { 2, -3}, {-1, -2}, { 1, -2}, { 2, -2}, {-1, -1}, { 0, -1}, { 1, -1}, { 0,  0}}};
static const CellList_s8 CellList_s8_lwss_cells_30 = { 9, {{-1, -1}, { 0, -1}, { 1, -1}, { 2, -1}, {-1,  0}, { 3,  0}, {-1,  1}, { 0,  2}, { 3,  2}}};
static const CellList_s8 CellList_s8_lwss_cells_31 = {12, {{ 0, -1}, { 1, -1}, {-1,  0}, { 0,  0}, { 2,  0}, { 3,  0}, { 0,  1}, { 1,  1}, { 2,  1}, { 3,  1}, { 1,  2}, { 2,  2}}};
static const CellList_s8 CellList_s8_lwss_cells_32 = { 9, {{ 1, -2}, { 4, -2}, { 0, -1}, { 0,  0}, { 4,  0}, { 0,  1}, { 1,  1}, { 2,  1}, { 3,  1}}};
static const CellList_s8 CellList_s8_lwss_cells_33 = {12, {{ 2, -2}, { 3, -2}, { 1, -1}, { 2, -1}, { 3, -1}, { 4, -1}, { 0,  0}, { 1,  0}, { 3,  0}, { 4,  0}, { 1,  1}, { 2,  1}}};

static const CellList_s8 CellList_s8_mwss_cells_00 = {11, {{-1, -1}, { 0, -1}, { 1, -1}, {-1,  0}, { 2,  0}, {-1,  1}, {-1,  2}, { 3,  2}, {-1,  3}, { 0,  4}, { 2,  4}}};
static const CellList_s8 CellList_s8_mwss_cells_01 = {15, {{ 0, -1}, {-1,  0}, { 0,  0}, { 1,  0}, {-1,  1}, { 1,  1}, { 2,  1}, { 0,  2}, { 1,  2}, { 2,  2}, { 0,  3}, { 1,  3}, { 2,  3}, { 0,  4}, { 1,  4}}};
static const CellList_s8 CellList_s8_mwss_cells_02 = {11, {{-1,  0}, { 0,  0}, { 1,  0}, {-2,  1}, { 1,  1}, { 1,  2}, {-3,  3}, { 1,  3}, { 1,  4}, {-2,  5}, { 0,  5}}};
static const CellList_s8 CellList_s8_mwss_cells_03 = {15, {{ 0,  0}, {-1,  1}, { 0,  1}, { 1,  1}, {-2,  2}, {-1,  2}, { 1,  2}, {-2,  3}, {-1,  3}, { 0,  3}, {-2,  4}, {-1,  4}, { 0,  4}, {-1,  5}, { 0,  5}}};
static const CellList_s8 CellList_s8_mwss_cells_10 = {11, {{-2, -3}, {-4, -2}, { 0, -2}, { 1, -1}, {-4,  0}, { 1,  0}, {-3,  1}, {-2,  1}, {-1,  1}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_mwss_cells_11 = {15, {{-3, -2}, {-2, -2}, {-1, -2}, {-4, -1}, {-3, -1}, {-2, -1}, {-1, -1}, { 0, -1}, {-4,  0}, {-3,  0}, {-2,  0}, { 0,  0}, { 1,  0}, {-1,  1}, { 0,  1}}};
static const CellList_s8 CellList_s8_mwss_cells_12 = {11, {{-4, -1}, {-3, -1}, {-2, -1}, {-1, -1}, { 0, -1}, {-5,  0}, { 0,  0}, { 0,  1}, {-5,  2}, {-1,  2}, {-3,  3}}};
static const CellList_s8 CellList_s8_mwss_cells_13 = {15, {{-2, -1}, {-1, -1}, {-5,  0}, {-4,  0}, {-3,  0}, {-1,  0}, { 0,  0}, {-5,  1}, {-4,  1}, {-3,  1}, {-2,  1}, {-1,  1}, {-4,  2}, {-3,  2}, {-2,  2}}};
static const CellList_s8 CellList_s8_mwss_cells_20 = {11, {{-2, -4}, { 0, -4}, { 1, -3}, {-3, -2}, { 1, -2}, { 1, -1}, {-2,  0}, { 1,  0}, {-1,  1}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_mwss_cells_21 = {15, {{-1, -4}, { 0, -4}, {-2, -3}, {-1, -3}, { 0, -3}, {-2, -2}, {-1, -2}, { 0, -2}, {-2, -1}, {-1, -1}, { 1, -1}, {-1,  0}, { 0,  0}, { 1,  0}, { 0,  1}}};
static const CellList_s8 CellList_s8_mwss_cells_22 = {11, {{ 0, -5}, { 2, -5}, {-1, -4}, {-1, -3}, { 3, -3}, {-1, -2}, {-1, -1}, { 2, -1}, {-1,  0}, { 0,  0}, { 1,  0}}};
static const CellList_s8 CellList_s8_mwss_cells_23 = {15, {{ 0, -5}, { 1, -5}, { 0, -4}, { 1, -4}, { 2, -4}, { 0, -3}, { 1, -3}, { 2, -3}, {-1, -2}, { 1, -2}, { 2, -2}, {-1, -1}, { 0, -1}, { 1, -1}, { 0,  0}}};
static const CellList_s8 CellList_s8_mwss_cells_30 = {11, {{-1, -1}, { 0, -1}, { 1, -1}, { 2, -1}, { 3, -1}, {-1,  0}, { 4,  0}, {-1,  1}, { 0,  2}, { 4,  2}, { 2,  3}}};
static const CellList_s8 CellList_s8_mwss_cells_31 = {15, {{ 0, -1}, { 1, -1}, {-1,  0}, { 0,  0}, { 2,  0}, { 3,  0}, { 4,  0}, { 0,  1}, { 1,  1}, { 2,  1}, { 3,  1}, { 4,  1}, { 1,  2}, { 2,  2}, { 3,  2}}};
static const CellList_s8 CellList_s8_mwss_cells_32 = {11, {{ 3, -3}, { 1, -2}, { 5, -2}, { 0, -1}, { 0,  0}, { 5,  0}, { 0,  1}, { 1,  1}, { 2,  1}, { 3,  1}, { 4,  1}}};
static const CellList_s8 CellList_s8_mwss_cells_33 = {15, {{ 2, -2}, { 3, -2}, { 4, -2}, { 1, -1}, { 2, -1}, { 3, -1}, { 4, -1}, { 5, -1}, { 0,  0}, { 1,  0}, { 3,  0}, { 4,  0}, { 5,  0}, { 1,  1}, { 2,  1}}};

static const CellList_s8 CellList_s8_hwss_cells_00 = {13, {{-1, -1}, { 0, -1}, { 1, -1}, {-1,  0}, { 2,  0}, {-1,  1}, {-1,  2}, { 3,  2}, {-1,  3}, { 3,  3}, {-1,  4}, { 0,  5}, { 2,  5}}};
static const CellList_s8 CellList_s8_hwss_cells_01 = {18, {{ 0, -1}, {-1,  0}, { 0,  0}, { 1,  0}, {-1,  1}, { 1,  1}, { 2,  1}, { 0,  2}, { 1,  2}, { 2,  2}, { 0,  3}, { 1,  3}, { 2,  3}, { 0,  4}, { 1,  4}, { 2,  4}, { 0,  5}, { 1,  5}}};
static const CellList_s8 CellList_s8_hwss_cells_02 = {13, {{-1,  0}, { 0,  0}, { 1,  0}, {-2,  1}, { 1,  1}, { 1,  2}, {-3,  3}, { 1,  3}, {-3,  4}, { 1,  4}, { 1,  5}, {-2,  6}, { 0,  6}}};
static const CellList_s8 CellList_s8_hwss_cells_03 = {18, {{ 0,  0}, {-1,  1}, { 0,  1}, { 1,  1}, {-2,  2}, {-1,  2}, { 1,  2}, {-2,  3}, {-1,  3}, { 0,  3}, {-2,  4}, {-1,  4}, { 0,  4}, {-2,  5}, {-1,  5}, { 0,  5}, {-1,  6}, { 0,  6}}};
static const CellList_s8 CellList_s8_hwss_cells_10 = {13, {{-3, -3}, {-2, -3}, {-5, -2}, { 0, -2}, { 1, -1}, {-5,  0}, { 1,  0}, {-4,  1}, {-3,  1}, {-2,  1}, {-1,  1}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_hwss_cells_11 = {18, {{-4, -2}, {-3, -2}, {-2, -2}, {-1, -2}, {-5, -1}, {-4, -1}, {-3, -1}, {-2, -1}, {-1, -1}, { 0, -1}, {-5,  0}, {-4,  0}, {-3,  0}, {-2,  0}, { 0,  0}, { 1,  0}, {-1,  1}, { 0,  1}}};
static const CellList_s8 CellList_s8_hwss_cells_12 = {13, {{-5, -1}, {-4, -1}, {-3, -1}, {-2, -1}, {-1, -1}, { 0, -1}, {-6,  0}, { 0,  0}, { 0,  1}, {-6,  2}, {-1,  2}, {-4,  3}, {-3,  3}}};
static const CellList_s8 CellList_s8_hwss_cells_13 = {18, {{-2, -1}, {-1, -1}, {-6,  0}, {-5,  0}, {-4,  0}, {-3,  0}, {-1,  0}, { 0,  0}, {-6,  1}, {-5,  1}, {-4,  1}, {-3,  1}, {-2,  1}, {-1,  1}, {-5,  2}, {-4,  2}, {-3,  2}, {-2,  2}}};
static const CellList_s8 CellList_s8_hwss_cells_20 = {13, {{-2, -5}, { 0, -5}, { 1, -4}, {-3, -3}, { 1, -3}, {-3, -2}, { 1, -2}, { 1, -1}, {-2,  0}, { 1,  0}, {-1,  1}, { 0,  1}, { 1,  1}}};
static const CellList_s8 CellList_s8_hwss_cells_21 = {18, {{-1, -5}, { 0, -5}, {-2, -4}, {-1, -4}, { 0, -4}, {-2, -3}, {-1, -3}, { 0, -3}, {-2, -2}, {-1, -2}, { 0, -2}, {-2, -1}, {-1, -1}, { 1, -1}, {-1,  0}, { 0,  0}, { 1,  0}, { 0,  1}}};
static const CellList_s8 CellList_s8_hwss_cells_22 = {13, {{ 0, -6}, { 2, -6}, {-1, -5}, {-1, -4}, { 3, -4}, {-1, -3}, { 3, -3}, {-1, -2}, {-1, -1}, { 2, -1}, {-1,  0}, { 0,  0}, { 1,  0}}};
static const CellList_s8 CellList_s8_hwss_cells_23 = {18, {{ 0, -6}, { 1, -6}, { 0, -5}, { 1, -5}, { 2, -5}, { 0, -4}, { 1, -4}, { 2, -4}, { 0, -3}, { 1, -3}, { 2, -3}, {-1, -2}, { 1, -2}, { 2, -2}, {-1, -1}, { 0, -1}, { 1, -1}, { 0,  0}}};
static const CellList_s8 CellList_s8_hwss_cells_30 = {13, {{-1, -1}, { 0, -1}, { 1, -1}, { 2, -1}, { 3, -1}, { 4, -1}, {-1,  0}, { 5,  0}, {-1,  1}, { 0,  2}, { 5,  2}, { 2,  3}, { 3,  3}}};
static const CellList_s8 CellList_s8_hwss_cells_31 = {18, {{ 0, -1}, { 1, -1}, {-1,  0}, { 0,  0}, { 2,  0}, { 3,  0}, { 4,  0}, { 5,  0}, { 0,  1}, { 1,  1}, { 2,  1}, { 3,  1}, { 4,  1}, { 5,  1}, { 1,  2}, { 2,  2}, { 3,  2}, { 4,  2}}};
static const CellList_s8 CellList_s8_hwss_cells_32 = {13, {{ 3, -3}, { 4, -3}, { 1, -2}, { 6, -2}, { 0, -1}, { 0,  0}, { 6,  0}, { 0,  1}, { 1,  1}, { 2,  1}, { 3,  1}, { 4,  1}, { 5,  1}}};
static const CellList_s8 CellList_s8_hwss_cells_33 = {18, {{ 2, -2}, { 3, -2}, { 4, -2}, { 5, -2}, { 1, -1}, { 2, -1}, { 3, -1}, { 4, -1}, { 5, -1}, { 6, -1}, { 0,  0}, { 1,  0}, { 3,  0}, { 4,  0}, { 5,  0}, { 6,  0}, { 1,  1}, { 2,  1}}};

static const CellList_s8 *CellList_s8_lwss_cells_nn [4] [4] =
		{{&CellList_s8_lwss_cells_00, &CellList_s8_lwss_cells_01, &CellList_s8_lwss_cells_02, &CellList_s8_lwss_cells_03},
		 {&CellList_s8_lwss_cells_10, &CellList_s8_lwss_cells_11, &CellList_s8_lwss_cells_12, &CellList_s8_lwss_cells_13},
		 {&CellList_s8_lwss_cells_20, &CellList_s8_lwss_cells_21, &CellList_s8_lwss_cells_22, &CellList_s8_lwss_cells_23},
		 {&CellList_s8_lwss_cells_30, &CellList_s8_lwss_cells_31, &CellList_s8_lwss_cells_32, &CellList_s8_lwss_cells_33}};

static const CellList_s8 *CellList_s8_mwss_cells_nn [4] [4] =
		{{&CellList_s8_mwss_cells_00, &CellList_s8_mwss_cells_01, &CellList_s8_mwss_cells_02, &CellList_s8_mwss_cells_03},
		 {&CellList_s8_mwss_cells_10, &CellList_s8_mwss_cells_11, &CellList_s8_mwss_cells_12, &CellList_s8_mwss_cells_13},
		 {&CellList_s8_mwss_cells_20, &CellList_s8_mwss_cells_21, &CellList_s8_mwss_cells_22, &CellList_s8_mwss_cells_23},
		 {&CellList_s8_mwss_cells_30, &CellList_s8_mwss_cells_31, &CellList_s8_mwss_cells_32, &CellList_s8_mwss_cells_33}};

static const CellList_s8 *CellList_s8_hwss_cells_nn [4] [4] =
		{{&CellList_s8_hwss_cells_00, &CellList_s8_hwss_cells_01, &CellList_s8_hwss_cells_02, &CellList_s8_hwss_cells_03},
		 {&CellList_s8_hwss_cells_10, &CellList_s8_hwss_cells_11, &CellList_s8_hwss_cells_12, &CellList_s8_hwss_cells_13},
		 {&CellList_s8_hwss_cells_20, &CellList_s8_hwss_cells_21, &CellList_s8_hwss_cells_22, &CellList_s8_hwss_cells_23},
		 {&CellList_s8_hwss_cells_30, &CellList_s8_hwss_cells_31, &CellList_s8_hwss_cells_32, &CellList_s8_hwss_cells_33}};
*/

static const CellList_s8 CellList_s8_block_cells_0 = {4, {{0, 0}, {0, 1}, {1, 0}, {1, 1}}};

static const CellList_s8 *CellList_s8_block_cells ()
{
	return &CellList_s8_block_cells_0;
}

static const CellList_s8 CellList_s8_blinker_cells_0 = {3, {{1, 0}, {1, 1}, {1, 2}}};
static const CellList_s8 CellList_s8_blinker_cells_1 = {3, {{0, 1}, {1, 1}, {2, 1}}};

static const CellList_s8 *CellList_s8_blinker_cells (int phase)
{
	if (phase == 0)
		return &CellList_s8_blinker_cells_0;
	else if (phase == 1)
		return &CellList_s8_blinker_cells_1;
	else
		return ffsc_p (__func__);
}

static const CellList_s8 CellList_s8_hive_cells_0 = {6, {{1, 0}, {0, 1}, {2, 1}, {0, 2}, {2, 2}, {1, 3}}};
static const CellList_s8 CellList_s8_hive_cells_1 = {6, {{1, 0}, {2, 0}, {0, 1}, {3, 1}, {1, 2}, {2, 2}}};

static const CellList_s8 *CellList_s8_hive_cells (int orientation)
{
	if (orientation == 0)
		return &CellList_s8_hive_cells_0;
	else if (orientation == 1)
		return &CellList_s8_hive_cells_1;
	else
		return ffsc_p (__func__);
}

static const CellList_s8 CellList_s8_boat_cells_0 = {5, {{0, 0}, {1, 0}, {0, 1}, {2, 1}, {1, 2}}};
static const CellList_s8 CellList_s8_boat_cells_1 = {5, {{1, 0}, {2, 0}, {0, 1}, {2, 1}, {1, 2}}};
static const CellList_s8 CellList_s8_boat_cells_2 = {5, {{1, 0}, {0, 1}, {2, 1}, {1, 2}, {2, 2}}};
static const CellList_s8 CellList_s8_boat_cells_3 = {5, {{1, 0}, {0, 1}, {2, 1}, {0, 2}, {1, 2}}};

static const CellList_s8 *CellList_s8_boat_cells (int orientation)
{
	if (orientation == 0)
		return &CellList_s8_boat_cells_0;
	else if (orientation == 1)
		return &CellList_s8_boat_cells_1;
	else if (orientation == 2)
		return &CellList_s8_boat_cells_2;
	else if (orientation == 3)
		return &CellList_s8_boat_cells_3;
	else
		return ffsc_p (__func__);
}

static const CellList_s8 CellList_s8_pond_cells_0 = {8, {{1, 0}, {2, 0}, {0, 1}, {3, 1}, {0, 2}, {3, 2}, {1, 3}, {2, 3}}};

static const CellList_s8 *CellList_s8_pond_cells ()
{
	return &CellList_s8_pond_cells_0;
}
