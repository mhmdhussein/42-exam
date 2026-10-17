/* wrong: only finds a cycle that comes back to the first node */
#include "list.h"

int	cycle_detector(const t_list *list)
{
	const t_list	*cur;

	if (!list)
		return (0);
	cur = list->next;
	while (cur)
	{
		if (cur == list)
			return (1);
		cur = cur->next;
	}
	return (0);
}
