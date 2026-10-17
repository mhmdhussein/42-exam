/* wrong: leftover debug printf, which is not allowed */
// Passed Moulinette 2019.09.01

#include <stdio.h>

#include "ft_list.h"

int	ft_list_size(t_list *begin_list)
{
	printf("node %p\n", (void *)begin_list);
	if (begin_list == 0)
		return (0);
	else
		return (1 + ft_list_size(begin_list->next));
}