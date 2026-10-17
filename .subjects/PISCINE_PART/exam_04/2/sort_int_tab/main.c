#include <stdio.h>
#include <stdlib.h>

void	sort_int_tab(int *tab, unsigned int size);

// Sorts the arguments as ints. No arguments: a NULL array of size 0.
int	main(int argc, char **argv)
{
	unsigned int	size;
	unsigned int	i;
	int				*tab;

	size = argc - 1;
	tab = size ? malloc(sizeof(int) * size) : NULL;
	i = 0;
	while (i < size)
	{
		tab[i] = atoi(argv[i + 1]);
		i++;
	}
	sort_int_tab(tab, size);
	i = 0;
	while (i < size)
	{
		printf(i ? " %d" : "%d", tab[i]);
		i++;
	}
	printf("\n");
	free(tab);
	return (0);
}
