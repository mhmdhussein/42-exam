/* wrong: tests the pointer instead of the character, so it reads past the end */
#include <unistd.h>

void    ft_putstr(char *str)
{
	int	i;

	i = 0;
	while (str)
	{
		write(1, &str[i], 1);
		i++;
	}
}