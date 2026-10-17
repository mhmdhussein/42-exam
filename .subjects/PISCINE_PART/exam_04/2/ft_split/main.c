/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pandalaf <pandalaf@student.42wolfsburg.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/06 15:58:50 by pandalaf          #+#    #+#             */
/*   Updated: 2022/11/06 15:59:01 by pandalaf         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>

char	**ft_split(char *str);

// Leaves freed blocks full of 'X', so a string missing its '\0' shows up.
static void dirty_heap(void)
{
    for (int size = 1; size <= 128; size++)
    {
        char *p = malloc(size);
        for (int i = 0; i < size; i++)
            p[i] = 'X';
        free(p);
    }
}

// Prints each word in brackets, up to the NULL that ends the array.
int main(int argc, char **argv)
{
	char	**split;
	int		i;

	if (argc == 2)
	{
		dirty_heap();
		split = ft_split(argv[1]);
		i = 0;
		while (split[i] != 0)
		{
			printf("[%s]", split[i]);
			i++;
		}
	}
	printf("\n");
    return (0);
}
