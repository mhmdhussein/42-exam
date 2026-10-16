/* wrong: calls strcpy, which is not allowed */
#include <string.h>

char	*ft_strcpy(char *s1, char *s2)
{
	return (strcpy(s1, s2));
}
