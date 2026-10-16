#include <stdio.h>
#include <stdlib.h>

/* Prints every subset of set[0..n) whose elements sum to target,
   keeping the original order of the elements. */
static void	solve(int *set, int n, int i, int target, int *sub, int len, int sum)
{
	int	j;

	if (i == n)
	{
		if (sum != target)
			return ;
		for (j = 0; j < len; j++)
			printf(j ? " %d" : "%d", sub[j]);
		printf("\n");
		return ;
	}
	sub[len] = set[i];
	solve(set, n, i + 1, target, sub, len + 1, sum + set[i]);
	solve(set, n, i + 1, target, sub, len, sum);
}

int	main(int argc, char **argv)
{
	int	*set;
	int	*sub;
	int	n;
	int	i;

	if (argc < 2)
		return (0);
	n = argc - 2;
	set = malloc(sizeof(int) * (n + 1));
	sub = malloc(sizeof(int) * (n + 1));
	if (!set || !sub)
	{
		free(set);
		free(sub);
		return (1);
	}
	for (i = 0; i < n; i++)
		set[i] = atoi(argv[i + 2]);
	solve(set, n, 0, atoi(argv[1]), sub, 0, 0);
	free(set);
	free(sub);
	return (0);
}
