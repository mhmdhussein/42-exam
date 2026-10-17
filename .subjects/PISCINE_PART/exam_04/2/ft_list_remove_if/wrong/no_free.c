/* wrong: unlinks the removed elements without freeing them */
#include <stdlib.h>
#include "ft_list.h"

void	ft_list_remove_if(t_list **begin_list, void *data_ref, int (*cmp)())
{
	t_list	*curr;

	while (*begin_list)
	{
		curr = *begin_list;
		if (cmp(curr->data, data_ref) == 0)
		{
			*begin_list = curr->next;
		}
		else
			begin_list = &curr->next;
	}
}
