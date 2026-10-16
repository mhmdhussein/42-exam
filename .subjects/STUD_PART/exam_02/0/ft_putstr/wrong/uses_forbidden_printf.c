/* wrong: prints with printf, which is not allowed */
#include <stdio.h>

void    ft_putstr(char *str)
{
	int	i;

	i = 0;
	while (str[i] != '\0')
	{
		printf("%c", str[i]);
		i++;
	}
}