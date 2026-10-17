#include <stdio.h>
#include <stdlib.h>

#include "ft_list.h"

void	ft_list_foreach(t_list *begin_list, void (*f)(void *));

static void	print_data(void *data)
{
	printf("[%s]\n", (char *)data);
}

static void	upper_first(void *data)
{
	char	*s = data;

	if (*s >= 'a' && *s <= 'z')
		*s -= 32;
}

// One node per argument, in order; no arguments: an empty list (NULL).
int	main(int argc, char **argv)
{
	t_list	*head = NULL;
	t_list	*node;
	int		i;

	i = argc;
	while (--i > 0)
	{
		node = malloc(sizeof(*node));
		node->data = argv[i];
		node->next = head;
		head = node;
	}
	ft_list_foreach(head, print_data);
	ft_list_foreach(head, upper_first);
	ft_list_foreach(head, print_data);
	printf("end\n");
	return (0);
}
