/* wrong: calls strtol, which is not allowed */
#include <stdlib.h>

int	ft_atoi_base(const char *str, int str_base)
{
	return ((int)strtol(str, NULL, str_base));
}
