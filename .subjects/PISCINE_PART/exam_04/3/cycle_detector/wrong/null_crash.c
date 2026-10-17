/* wrong: reads list->next without checking for an empty list */
#include "list.h"

int	cycle_detector(const t_list *list)
{
	const t_list	*slow;
	const t_list	*fast;

	slow = list;
	fast = list->next;
	while (fast && fast->next)
	{
		if (fast == slow)
			return (1);
		slow = slow->next;
		fast = fast->next->next;
	}
	return (0);
}
