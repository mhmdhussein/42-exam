/* wrong: calls strdup, which is not allowed */
#include <string.h>

char	*ft_strdup(char *src)
{
	return (strdup(src));
}
