#include "util.h"

void insert_str(char* dest, size_t d_size, const char* str_to_insert, int pos)
{
	int o_length = strlen(str_to_insert);
	int end_index = pos + o_length;
	if (pos < 0 || pos >= d_size) return;
	int i;
	for (i = d_size - 2; i >= end_index; i--)
	{
		dest[i] = dest[i - o_length];
	}
	int j;
	for (i = pos, j = 0; i < end_index; i++, j++)
	{
		dest[i] = str_to_insert[j];
	}
}