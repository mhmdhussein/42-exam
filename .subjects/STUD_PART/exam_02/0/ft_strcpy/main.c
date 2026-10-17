char	*ft_strcpy(char *s1, char *s2);
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    char *str;
    char *ret;
    int i;

    if (argc != 2)
        return (0);
    // fill the destination so a missing '\0' shows up
    str = malloc(sizeof(char) * 100);
    for (i = 0; i < 99; i++)
        str[i] = 'X';
    str[99] = '\0';
    ret = ft_strcpy(str, argv[1]);
    printf("ft_strcpy(\"%s\", str) = %s\n", argv[1], ret);
    printf("returns dest: %s\n", ret == str ? "yes" : "no");
    free(str);
    return(0);
}
