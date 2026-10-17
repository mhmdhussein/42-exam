/* wrong: allocates with calloc, which is not allowed */
#include <stdlib.h>
#include <stdio.h>

int	*ft_range(int start, int end)
{
	int	size;
	int	*res;
	int	i;

	size = (end > start ? end - start : start - end) + 1;
	res = calloc(size, sizeof(int));
	if (!res)
		return (NULL);
	i = 0;
	if (start < end)
	{
		while (start <= end)
		{
			res[i] = start;
			start += 1;
			i += 1;
		}
	}
	else
	{
		while (start >= end)
		{
			res[i] = start;
			start -= 1;
			i += 1;
		}
	}
	return (res);
}
