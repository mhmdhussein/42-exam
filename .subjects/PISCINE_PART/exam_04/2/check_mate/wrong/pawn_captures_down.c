/* wrong: pawns capture downwards instead of upwards */
#include <unistd.h>

static int	is_piece(char c)
{
	return (c == 'P' || c == 'B' || c == 'R' || c == 'Q' || c == 'K');
}

// Walks from the King in direction (dy, dx) and returns the first piece met.
static char	first_piece(char **board, int size, int y, int x, int dy, int dx)
{
	y += dy;
	x += dx;
	while (y >= 0 && y < size && x >= 0 && x < size)
	{
		if (is_piece(board[y][x]))
			return (board[y][x]);
		y += dy;
		x += dx;
	}
	return (0);
}

static int	in_check(char **board, int size, int y, int x)
{
	int		dy;
	int		dx;
	char	c;

	if (y > 0 && ((x > 0 && board[y - 1][x - 1] == 'P')
			|| (x + 1 < size && board[y - 1][x + 1] == 'P')))
		return (1);
	dy = -1;
	while (dy <= 1)
	{
		dx = -1;
		while (dx <= 1)
		{
			c = (dy || dx) ? first_piece(board, size, y, x, dy, dx) : 0;
			if (c == 'Q' || (c == 'R' && (!dy || !dx))
				|| (c == 'B' && dy && dx))
				return (1);
			dx++;
		}
		dy++;
	}
	return (0);
}

int	main(int ac, char **av)
{
	int	size;
	int	y;
	int	x;

	if (ac < 2)
		return (write(1, "\n", 1), 0);
	size = ac - 1;
	y = 0;
	while (y < size)
	{
		x = 0;
		while (x < size)
		{
			if (av[y + 1][x] == 'K')
			{
				if (in_check(av + 1, size, y, x))
					write(1, "Success\n", 8);
				else
					write(1, "Fail\n", 5);
				return (0);
			}
			x++;
		}
		y++;
	}
	write(1, "Fail\n", 5);
	return (0);
}
