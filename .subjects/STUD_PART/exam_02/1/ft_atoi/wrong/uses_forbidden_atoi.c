/* wrong: calls atoi, which is not allowed */
#include <stdlib.h>

int	ft_atoi(const char *str)
{
	return (atoi(str));
}
