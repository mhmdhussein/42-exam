#include <stdio.h>
#include <stdlib.h>

#include "list.h"

int	cycle_detector(const t_list *list);

// argv[1]: number of nodes; argv[2] (optional): index of the node the last
// node links back to. "null": call with NULL.
int	main(int argc, char **argv)
{
	t_list	*nodes[2048];
	int		n;
	int		i;

	if (argc < 2)
		return (printf("%d\n", cycle_detector(NULL)), 0);
	n = atoi(argv[1]);
	if (n < 1 || n > 2048)
		return (printf("%d\n", cycle_detector(NULL)), 0);
	for (i = 0; i < n; i++)
	{
		nodes[i] = malloc(sizeof(t_list));
		nodes[i]->data = i % 3;
		nodes[i]->next = NULL;
	}
	for (i = 0; i + 1 < n; i++)
		nodes[i]->next = nodes[i + 1];
	if (argc > 2)
		nodes[n - 1]->next = nodes[atoi(argv[2])];
	printf("%d\n", cycle_detector(nodes[0]));
	return (0);
}
