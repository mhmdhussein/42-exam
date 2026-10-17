/* wrong: prints OK with printf, which is not allowed */
#include <stdio.h>
#include <unistd.h>

static int	check(char *s)
{
	char	stack[4096];
	int		top;

	top = 0;
	while (*s)
	{
		if (*s == '(' || *s == '[' || *s == '{')
			stack[top++] = *s;
		else if (*s == ')' || *s == ']' || *s == '}')
		{
			if (top == 0 || stack[--top] != (*s == ')' ? '(' : *s - 2))
				return (0);
		}
		s++;
	}
	return (top == 0);
}

int	main(int ac, char **av)
{
	int	i;

	if (ac == 1)
		write(1, "\n", 1);
	i = 1;
	while (i < ac)
	{
		if (check(av[i]))
			printf("OK\n");
			fflush(stdout);
		else
			write(1, "Error\n", 6);
		i++;
	}
	return (0);
}
