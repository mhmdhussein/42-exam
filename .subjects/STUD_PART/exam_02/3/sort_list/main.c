#include "list.h"
#include <stdio.h>
#include <stdlib.h>

void	swap_values(t_list *a, t_list *b);
t_list	*sort_list(t_list *lst, int (*cmp)(int, int));

int ascending(int a, int b)
{
	return (a <= b);
}

void	print_list(t_list *cur)
{
	while (cur)
	{
		printf("%d", cur->data);
		if (cur->next != 0)
			printf(", ");
		cur = cur->next;
	}
	printf("\n");
}

// No arguments: the list 108, 73, 45. Otherwise one node per argument.
int	main(int argc, char **argv)
{
	int		defaults[] = {108, 73, 45};
	int		count = (argc == 1) ? 3 : argc - 1;
	t_list	*head = 0;

	while (count-- > 0)
	{
		t_list *node = malloc(sizeof(t_list));
		node->data = (argc == 1) ? defaults[count] : atoi(argv[count + 1]);
		node->next = head;
		head = node;
	}
	print_list(head);
	print_list(sort_list(head, ascending));
	return (0);
}
