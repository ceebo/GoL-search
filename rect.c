typedef struct
{
	int left_x;
	int top_y;
	int width;
	int height;
} Rect;

static void Rect_make (Rect *r, int left_x, int top_y, int width, int height)
{
	if (!r)
		return (void) ffsc (__func__);
	
	if (width < 0 || height < 0)
	{
		width = 0;
		height = 0;
		ffsc (__func__);
	}
	
	r->left_x = left_x;
	r->top_y = top_y;
	r->width = width;
	r->height = height;
}

static int Rect_is_equal_to (const Rect *rect_1, const Rect *rect_2)
{
	if (!rect_1 || !rect_2)
		return ffsc (__func__);
	
	return (rect_1->left_x == rect_2->left_x && rect_1->top_y == rect_2->top_y && rect_1->width == rect_2->width && rect_1->height == rect_2->height);
}

static int Rect_within (const Rect *r, int x, int y)
{
	return (x >= r->left_x && x < (r->left_x + r->width) && y >= r->top_y && y < (r->top_y + r->height));
}

static void Rect_copy (const Rect *src, Rect *dest)
{
	if (!src || !dest)
		return (void) ffsc (__func__);
	
	dest->left_x = src->left_x;
	dest->top_y = src->top_y;
	dest->width = src->width;
	dest->height = src->height;
}

static void Rect_union (const Rect *src_1, const Rect *src_2, Rect *dest)
{
	if (!src_1 || !src_2 || !dest)
		return (void) ffsc (__func__);
	
	int left_x = lowest_of (src_1->left_x, src_2->left_x);
	int x_off = highest_of (src_1->left_x + src_1->width, src_2->left_x + src_2->width);
	int top_y = lowest_of (src_1->top_y, src_2->top_y);
	int y_off = highest_of (src_1->top_y + src_1->height, src_2->top_y + src_2->height);
	
	dest->left_x = left_x;
	dest->top_y = top_y;
	dest->width = x_off - left_x;
	dest->height = y_off - top_y;
}

static int Rect_intersection (const Rect *src_1, const Rect *src_2, Rect *dest)
{
	if (!src_1 || !src_2 || !dest)
		return ffsc (__func__);
	
	int left_x = highest_of (src_1->left_x, src_2->left_x);
	int x_off = lowest_of (src_1->left_x + src_1->width, src_2->left_x + src_2->width);
	int top_y = highest_of (src_1->top_y, src_2->top_y);
	int y_off = lowest_of (src_1->top_y + src_1->height, src_2->top_y + src_2->height);
	
	dest->left_x = left_x;
	dest->top_y = top_y;
	
	if (x_off < left_x || y_off < top_y)
	{
		dest->width = 0;
		dest->height = 0;
		return FALSE;
	}
	else
	{
		dest->width = x_off - left_x;
		dest->height = y_off - top_y;
		return (x_off > left_x && y_off > top_y);
	}
}

static void Rect_add_borders (Rect *r, int border_size)
{
	if (!r || border_size < 0)
		return (void) ffsc (__func__);
	
	r->left_x -= border_size;
	r->top_y -= border_size;
	r->width += (2 * border_size);
	r->height += (2 * border_size);
}
