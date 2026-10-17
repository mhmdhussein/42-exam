/* wrong: compares the data of the nodes instead of the nodes themselves */
#include "list.h"

int	cycle_detector(const t_list *list)
{
	const t_list	*slow;
	const t_list	*fast;

	slow = list;
	fast = list;
	while (fast && fast->next)
	{
		slow = slow->next;
		fast = fast->next->next;
		if (fast && fast->data == slow->data)
			return (1);
	}
	return (0);
}
