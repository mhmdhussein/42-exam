/* wrong: prints the top of the stack even when operands are left over */
#include <stdio.h>
#include <stdlib.h>

static int	is_op(char *s)
{
	return ((*s == '+' || *s == '-' || *s == '*' || *s == '/' || *s == '%')
		&& (s[1] == ' ' || s[1] == '\0'));
}

static int	is_number(char *s)
{
	if (*s == '+' || *s == '-')
		s++;
	if (*s < '0' || *s > '9')
		return (0);
	while (*s >= '0' && *s <= '9')
		s++;
	return (*s == ' ' || *s == '\0');
}

static int	rpn_calc(char *s, int *result)
{
	int	stack[4096];
	int	top;
	int	a;
	int	b;

	top = 0;
	while (*s)
	{
		if (*s == ' ')
		{
			s++;
			continue ;
		}
		if (is_op(s))
		{
			if (top < 2)
				return (0);
			b = stack[--top];
			a = stack[--top];
			if ((*s == '/' || *s == '%') && b == 0)
				return (0);
			if (*s == '+')
				stack[top++] = a + b;
			else if (*s == '-')
				stack[top++] = a - b;
			else if (*s == '*')
				stack[top++] = a * b;
			else if (*s == '/')
				stack[top++] = a / b;
			else
				stack[top++] = a % b;
		}
		else if (is_number(s) && top < 4096)
			stack[top++] = atoi(s);
		else
			return (0);
		while (*s && *s != ' ')
			s++;
	}
	if (top < 1)
		return (0);
	*result = stack[0];
	return (1);
}

int	main(int ac, char **av)
{
	int	result;

	if (ac == 2 && rpn_calc(av[1], &result))
		printf("%d\n", result);
	else
		printf("Error\n");
	return (0);
}
