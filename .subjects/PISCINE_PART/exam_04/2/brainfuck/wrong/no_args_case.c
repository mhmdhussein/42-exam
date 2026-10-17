/* wrong: prints nothing when there are no arguments */
#include <unistd.h>

int	main(int ac, char **av)
{
	char	mem[2048] = {0};
	char	*p = mem;
	int		i = 0;
	int		depth;

	if (ac != 2)
		return (0);
	while (av[1][i])
	{
		char c = av[1][i];
		if (c == '>') p++;
		else if (c == '<') p--;
		else if (c == '+') (*p)++;
		else if (c == '-') (*p)--;
		else if (c == '.') write(1, p, 1);
		else if (c == '[' && !*p)
		{
			depth = 1;
			while (depth)
			{
				i++;
				depth += (av[1][i] == '[') - (av[1][i] == ']');
			}
		}
		else if (c == ']' && *p)
		{
			depth = 1;
			while (depth)
			{
				i--;
				depth += (av[1][i] == ']') - (av[1][i] == '[');
			}
		}
		i++;
	}
	return (0);
}
