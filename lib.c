#define FALSE 0
#define TRUE 1

#define s8 signed char
#define u8 unsigned char
#define u16 unsigned short int
#define u32 unsigned int
#define s64 long long int
#define u64 unsigned long long int

static int ffsc (const char *fname)
{
  fprintf (stderr, "%s failed in function entry sanity check\n", (fname ? fname : "<unknown function namn>"));
  return FALSE;
}

static void *ffsc_p (const char *fname)
{
  fprintf (stderr, "%s failed in function entry sanity check\n", (fname ? fname : "<unknown function namn>"));
  return NULL;
}

static int lowest_of (int arg1, int arg2)
{
	if (arg1 <= arg2)
		return arg1;
	else
		return arg2;
}

static int highest_of (int arg1, int arg2)
{
	if (arg1 >= arg2)
		return arg1;
	else
		return arg2;
}

static int lowest_of_u64 (u64 arg1, u64 arg2)
{
	if (arg1 <= arg2)
		return arg1;
	else
		return arg2;
}

static int highest_of_u64 (u64 arg1, u64 arg2)
{
	if (arg1 >= arg2)
		return arg1;
	else
		return arg2;
}

static int bit_count_u64 (u64 arg)
{
	arg = (arg & 0x5555555555555555ULL) + ((arg >>  1) & 0x5555555555555555ULL);
	arg = (arg & 0x3333333333333333ULL) + ((arg >>  2) & 0x3333333333333333ULL);
	arg = (arg & 0x0f0f0f0f0f0f0f0fULL) + ((arg >>  4) & 0x0f0f0f0f0f0f0f0fULL);
	return (arg * 0x0101010101010101ULL) >> 56;	
}

static int least_significant_bit_u64 (u64 arg)
{
	int lsb_bit_ix = 32;
	int delta_shift_size = 16;
	
	int iteration;
	for (iteration = 0; iteration < 6; iteration++)
	{
		if (arg << (63 - lsb_bit_ix) == 0)
			lsb_bit_ix += delta_shift_size;
		else if (arg << (63 - lsb_bit_ix) == 0x8000000000000000ULL)
			break;
		else
			lsb_bit_ix -= delta_shift_size;
		
		delta_shift_size = (delta_shift_size + 1) >> 1;
	}
	
	return lsb_bit_ix;
}

static int most_significant_bit_u64 (u64 arg)
{
	int msb_bit_ix = 31;
	int delta_shift_size = 16;
	
	int iteration;
	for (iteration = 0; iteration < 6; iteration++)
	{
		if (arg >> msb_bit_ix == 0)
			msb_bit_ix -= delta_shift_size;
		else if (arg >> msb_bit_ix == 1)
			break;
		else
			msb_bit_ix += delta_shift_size;
		
		delta_shift_size = (delta_shift_size + 1) >> 1;
	}
	
	return msb_bit_ix;
}

static int digits_in_number (int arg)
{
	if (arg == (int) 0x80000000)
		return 11;

	int digits = 1;
	if (arg < 0)
	{
		arg = -arg;
		digits++;
	}
	
	while (arg > 9)
	{
		arg /= 10;
		digits++;
	}
	
	return digits;
}

static int round_double (double arg)
{
	if (arg < 0.0)
		return (int) (arg - 0.5);
	else
		return (int) (arg + 0.5);
}

// This is the "xorshift128plus" PRNG by Sebastiano Vigna

static u64 random_state_0 = 0x19803c70561f8414ULL;
static u64 random_state_1 = 0xcaca61eeae213995ULL;

static void random_u64_set_seed (u64 seed_1, u64 seed_2, int xor_with_current_time)
{
	u64 cur_time = 0;
	if (xor_with_current_time)
		cur_time = (u64) time (0);
	
	random_state_0 = seed_1 ^ cur_time;
	random_state_1 = seed_2 ^ cur_time;
}

static u64 random_u64 ()
{
	u64 x = random_state_0;
	u64 y = random_state_1;
	random_state_0 = y;
	x ^= x << 23;
	random_state_1 = x ^ y ^ (x >> 17) ^ (y >> 26);
	return random_state_1 + y;
}

static void print_u64_hex (char *text, u64 arg)
{
	printf ("%s%08x%08x\n", text, (unsigned int) (arg >> 32), (unsigned int) (arg & 0x00000000ffffffffULL));
}
