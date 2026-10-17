/* wrong: jumps to the nearest bracket instead of the matching one */
#include <unistd.h>

int	main(int ac, char **av)
{
	char	mem[2048] = {0};
	char	*p = mem;
	int		i = 0;

	if (ac != 2)
		return (write(1, "\n", 1), 0);
	while (av[1][i])
	{
		char c = av[1][i];
		if (c == '>') p++;
		else if (c == '<') p--;
		else if (c == '+') (*p)++;
		else if (c == '-') (*p)--;
		else if (c == '.') write(1, p, 1);
		else if (c == '[' && !*p)
			while (av[1][i] != ']') i++;
		else if (c == ']' && *p)
			while (av[1][i] != '[') i--;
		i++;
	}
	return (0);
}
