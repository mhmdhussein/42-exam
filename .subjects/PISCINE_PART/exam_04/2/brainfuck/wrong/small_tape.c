/* wrong: the tape is only 256 bytes, not 2048 */
#include <unistd.h>

int	main(int ac, char **av)
{
	char	mem[256] = {0};
	int		p = 0;
	int		i = 0;
	int		depth;

	if (ac != 2)
		return (write(1, "\n", 1), 0);
	while (av[1][i])
	{
		char c = av[1][i];
		if (c == '>') p = (p + 1) % 256;
		else if (c == '<') p = (p + 255) % 256;
		else if (c == '+') mem[p]++;
		else if (c == '-') mem[p]--;
		else if (c == '.') write(1, &mem[p], 1);
		else if (c == '[' && !mem[p])
		{
			depth = 1;
			while (depth)
			{
				i++;
				depth += (av[1][i] == '[') - (av[1][i] == ']');
			}
		}
		else if (c == ']' && mem[p])
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
