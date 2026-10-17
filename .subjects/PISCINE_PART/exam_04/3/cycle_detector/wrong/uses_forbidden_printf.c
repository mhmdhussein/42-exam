/* wrong: leftover debug printf, which is not allowed */
#include <stdio.h>
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
		if (fast == slow)
			return (printf("%s", ""), 1);
	}
	return (0);
}
