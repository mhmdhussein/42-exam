/* wrong: capitalizes the last letter of a word even when a non-letter ends it */
#include <unistd.h>

static int	is_sep(char c)
{
	return (c == ' ' || c == '\t' || c == '\0');
}

static int	is_alpha(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static void	rstr(char *s)
{
	int	i;
	int	j;

	i = 0;
	while (s[i])
	{
		if (s[i] >= 'A' && s[i] <= 'Z')
			s[i] += 32;
		i++;
	}
	i = 0;
	while (s[i])
	{
		if (!is_sep(s[i]) && is_sep(s[i + 1]))
		{
			j = i;
			while (j >= 0 && !is_sep(s[j]) && !is_alpha(s[j]))
				j--;
			if (j >= 0 && is_alpha(s[j]) && s[j] >= 'a')
				s[j] -= 32;
		}
		i++;
	}
	while (*s)
		write(1, s++, 1);
	write(1, "\n", 1);
}

int	main(int argc, char **argv)
{
	int	i;

	if (argc == 1)
		write(1, "\n", 1);
	i = 1;
	while (i < argc)
		rstr(argv[i++]);
	return (0);
}
