/* wrong: prints the bits from a to z instead of z to a */
#include <unistd.h>

static int	usage(void)
{
	write(1, "options: abcdefghijklmnopqrstuvwxyz\n", 36);
	return (0);
}

int	main(int ac, char **av)
{
	unsigned int	opts;
	int				i;
	int				j;
	int				bit;

	if (ac == 1)
		return (usage());
	opts = 0;
	i = 0;
	while (++i < ac)
	{
		if (av[i][0] != '-')
			continue ;
		j = 0;
		while (av[i][++j])
		{
			if (av[i][j] < 'a' || av[i][j] > 'z')
				return (write(1, "Invalid Option\n", 15), 0);
			if (av[i][j] == 'h')
				return (usage());
			opts |= 1u << (av[i][j] - 'a');
		}
	}
	bit = -1;
	while (++bit < 32)
	{
		write(1, (opts >> bit) & 1 ? "1" : "0", 1);
		if (bit != 31 && bit % 8 == 7)
			write(1, " ", 1);
	}
	write(1, "\n", 1);
	return (0);
}
