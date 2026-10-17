/* wrong: counts the NULL at the end as an element */
// Passed Moulinette 2019.09.01

#include "ft_list.h"

int	ft_list_size(t_list *begin_list)
{
	if (begin_list == 0)
		return (1);
	else
		return (1 + ft_list_size(begin_list->next));
}