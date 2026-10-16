#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>

int picoshell(char **cmds[]);

static int count_fds(void)
{
    DIR             *dir = opendir("/proc/self/fd");
    struct dirent   *e;
    int             n = 0;

    if (!dir)
        return (-1);
    while ((e = readdir(dir)))
        if (e->d_name[0] != '.')
            n++;
    closedir(dir);
    return (n);
}

/* Same as the given main: argv split on "|" into cmds. After the pipeline
   it reports the return value, fd leaks and leftover child processes. */
int main(int argc, char **argv)
{
    int     cmds_size = 1;
    char    ***cmds;
    int     i, j, ret, fds;

    for (i = 1; i < argc; i++)
        if (!strcmp(argv[i], "|"))
            cmds_size++;
    cmds = calloc(cmds_size + 1, sizeof(char **));
    if (!cmds)
        return (1);
    cmds[0] = argv + 1;
    for (i = 1, j = 1; i < argc; i++)
        if (!strcmp(argv[i], "|"))
        {
            cmds[j++] = argv + i + 1;
            argv[i] = NULL;
        }
    fds = count_fds();
    ret = picoshell(cmds);
    printf("return: %d\n", ret);
    printf("fds: %s\n", count_fds() == fds ? "ok" : "LEAKED");
    if (waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD)
        printf("no process left\n");
    else
        printf("leaked process (zombie or still running)\n");
    free(cmds);
    return (0);
}
