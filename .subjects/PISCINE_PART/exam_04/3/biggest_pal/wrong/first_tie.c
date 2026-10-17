/* wrong: prints the first palindrome of the biggest length instead of the last */
#include <unistd.h>

// Grows a palindrome around the center [l, r] and returns its length.
static int	expand(char *s, int len, int *l, int r)
{
	while (*l >= 0 && r < len && s[*l] == s[r])
	{
		(*l)--;
		r++;
	}
	(*l)++;
	return (r - *l);
}

int	main(int ac, char **av)
{
	char	*s;
	int		len;
	int		best;
	int		start;
	int		i;
	int		l;
	int		n;

	if (ac == 2)
	{
		s = av[1];
		len = 0;
		while (s[len])
			len++;
		best = 0;
		start = 0;
		i = -1;
		while (++i < len)
		{
			l = i;
			n = expand(s, len, &l, i);
			if (n > best && (best = n))
				start = l;
			l = i;
			n = expand(s, len, &l, i + 1);
			if (n > 0 && n > best && (best = n))
				start = l;
		}
		write(1, s + start, best);
	}
	write(1, "\n", 1);
	return (0);
}
