/* wrong: converts with strtol, which is not allowed */
// Passed Moulinette 2019.09.01

#include <stdio.h>
#include <stdlib.h>

void	pgcd(int a, int b)
{
	int n = a;

	while (n > 0)
	{
		if (a % n == 0 && b % n == 0)
		{
			printf("%d", n);
			return;
		}
		--n;
	}
}

int		main(int argc, char **argv)
{
	if (argc == 3)
		pgcd(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 10));

	printf("\n");
	return (0);
}