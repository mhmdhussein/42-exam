char    *ft_strdup(char *src);
#include <stdlib.h>
#include <stdio.h>

// Leaves freed blocks full of 'X', so a copy missing its '\0' shows up.
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
    char *src = (argc == 1) ? "" : argv[1];
    char *copy;

    dirty_heap();
    copy = ft_strdup(src);
    printf("ft_strdup(\"%s\") = %s\n", src, copy);
    if (copy == src)
    {
        printf("the copy is the same pointer as the source\n");
        return (0);
    }
    // the copy must be separate memory: changing it leaves the source alone
    if (copy[0] != '\0')
        copy[0] = '#';
    printf("source after changing the copy: %s\n", src);
    free(copy);
    return(0);
}
