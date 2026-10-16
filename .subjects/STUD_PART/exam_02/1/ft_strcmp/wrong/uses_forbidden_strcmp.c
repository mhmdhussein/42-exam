/* wrong: calls strcmp, which is not allowed */
#include <string.h>

int	ft_strcmp(char *s1, char *s2)
{
	return (strcmp(s1, s2));
}
