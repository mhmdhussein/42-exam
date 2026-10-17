/* wrong: swaps without a temporary, so b gets its own value back */
#include <unistd.h>
#include <stdio.h>

void    ft_swap(int *a, int *b)
{
	int	temp;

	(void)temp;
	*a = *b;
	*b = *a;
}