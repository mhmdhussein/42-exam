/* wrong: prints the final newline with printf, which is not allowed */
#include <unistd.h>
#include <stdio.h>

int	main(int ac, char **av)
{
	int	i;

	i = 0;
	if (ac == 2)
	{
		while(av[1][i] != '\0')
			i++;
		while (i >= 1)
		{
			write(1, &av[1][i - 1], 1);
			i--;
		}
	}
	printf("\n");
}