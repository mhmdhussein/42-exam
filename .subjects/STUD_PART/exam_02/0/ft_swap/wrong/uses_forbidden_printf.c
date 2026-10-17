/* wrong: leftover debug printf, which is not allowed */
#include <unistd.h>
#include <stdio.h>

void    ft_swap(int *a, int *b)
{
	int	temp;

	printf("swap %d %d\n", *a, *b);
	temp = *a;
	*a = *b;
	*b = temp;
}