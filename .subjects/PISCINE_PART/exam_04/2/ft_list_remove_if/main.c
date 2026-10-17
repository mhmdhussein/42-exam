#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ft_list.h"

void	ft_list_remove_if(t_list **begin_list, void *data_ref, int (*cmp)());

// argv[1] is data_ref; one node per other argument, in order.
int	main(int argc, char **argv)
{
	t_list	*head = NULL;
	t_list	*node;
	int		i;

	if (argc < 2)
		return (printf("\n"), 0);
	i = argc;
	while (--i > 1)
	{
		node = malloc(sizeof(*node));
		node->data = argv[i];
		node->next = head;
		head = node;
	}
	ft_list_remove_if(&head, argv[1], strcmp);
	while (head)
	{
		printf("[%s]", (char *)head->data);
		node = head->next;
		free(head);
		head = node;
	}
	printf("\n");
	return (0);
}
