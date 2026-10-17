/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcluzet <jcluzet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/21 02:22:36 by jcluzet           #+#    #+#             */
/*   Updated: 2022/09/01 23:10:52 by jcluzet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
char	*ft_itoa(int nbr);

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

int main(int argc, char **argv)
{
    if (argc != 2)
        return (0);
    dirty_heap();
    printf("%s\n", ft_itoa(atoi(argv[1])));
    return (0);
}
