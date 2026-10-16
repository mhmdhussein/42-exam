#include <stdio.h>

int ft_scanf(const char *, ...);

/* argv[1] picks the format; input comes from stdin. */
int main(int argc, char **argv)
{
    int     a = -1, b = -1, c = -1, r = -2;
    char    s1[256] = "unset", s2[256] = "unset";
    char    c1 = '?', c2 = '?', c3 = '?';
    char    t = argc > 1 ? argv[1][0] : '1';

    if (t == '1')
    {
        r = ft_scanf("%d %s %c", &a, s1, &c1);
        printf("%d %d [%s] [%c]\n", r, a, s1, c1);
    }
    else if (t == '2')
    {
        r = ft_scanf("%s %s", s1, s2);
        printf("%d [%s] [%s]\n", r, s1, s2);
    }
    else if (t == '3')
    {
        r = ft_scanf("%d %d %d", &a, &b, &c);
        printf("%d %d %d %d\n", r, a, b, c);
    }
    else if (t == '4')
    {
        r = ft_scanf("%c%c%c", &c1, &c2, &c3);
        printf("%d [%c] [%c] [%c]\n", r, c1, c2, c3);
    }
    else if (t == '5')
    {
        r = ft_scanf("%d,%d", &a, &b);
        printf("%d %d %d\n", r, a, b);
    }
    else if (t == '6')
    {
        r = ft_scanf("x=%d %c", &a, &c1);
        printf("%d %d [%c]\n", r, a, c1);
    }
    return (0);
}
