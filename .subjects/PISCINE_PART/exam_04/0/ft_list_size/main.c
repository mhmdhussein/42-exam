#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ft_list.h"


// No arguments: a list of 3 nodes. "empty": an empty list (NULL).
// Otherwise one node per argument.
int	main(int argc, char **argv)
{
	t_list	*head = 0;
	int		count = argc - 1;

	if (argc == 1)
		count = 3;
	else if (argc == 2 && strcmp(argv[1], "empty") == 0)
		count = 0;
	while (count-- > 0)
	{
		t_list *node = malloc(sizeof(*node));
		node->next = head;
		node->data = argv[0];
		head = node;
	}
	printf("%d\n", ft_list_size(head));
	return (0);
}
