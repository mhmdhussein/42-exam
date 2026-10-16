#include "get_next_line.h"

/* Prints every line returned by get_next_line for each file in argv
   (stdin when there is none), then checks NULL is returned again at EOF. */
static void read_all(int fd, const char *name)
{
    char    *line;
    int     n = 0;

    while ((line = get_next_line(fd)) != NULL)
    {
        printf("%s %d: [%s]\n", name, ++n, line);
        free(line);
    }
    line = get_next_line(fd);
    printf("%s: %d lines, then %s\n", name, n, line ? "NOT NULL" : "NULL");
    free(line);
}

int main(int argc, char **argv)
{
    int i;
    int fd;

    if (argc == 1)
        read_all(0, "stdin");
    for (i = 1; i < argc; i++)
    {
        fd = open(argv[i], O_RDONLY);
        read_all(fd, argv[i]);
        if (fd >= 0)
            close(fd);
    }
    return (0);
}
