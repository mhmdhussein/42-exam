/* wrong: calls strlen, which is not allowed */
#include <string.h>

int	ft_strlen(char *str)
{
	return ((int)strlen(str));
}
