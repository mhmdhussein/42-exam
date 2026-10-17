/* wrong: sorts with qsort, which is not allowed */
#include <stdlib.h>

static int	cmp(const void *a, const void *b)
{
	int	x = *(const int *)a;
	int	y = *(const int *)b;

	return ((x > y) - (x < y));
}

void	sort_int_tab(int *tab, unsigned int size)
{
	if (size > 1)
		qsort(tab, size, sizeof(int), cmp);
}
