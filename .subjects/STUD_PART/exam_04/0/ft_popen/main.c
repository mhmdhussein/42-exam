#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>

int ft_popen(const char *file, char *const argv[], char type);

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

static void report(int fds)
{
    while (wait(NULL) > 0)
        ;
    printf("fds: %s\n", count_fds() == fds ? "ok" : "LEAKED");
}

/* ./a.out r cmd args...  : print what cmd writes
   ./a.out w cmd args...  : send "hello\nworld\n42\n" to cmd
   ./a.out chain          : ls | grep c done with two ft_popen calls
   ./a.out errors         : invalid parameters must return -1 */
int main(int argc, char **argv)
{
    char    buf[4096];
    ssize_t n;
    int     fd;
    int     fds = count_fds();

    if (argc >= 3 && (argv[1][0] == 'r' || argv[1][0] == 'w'))
    {
        fd = ft_popen(argv[2], argv + 2, argv[1][0]);
        if (fd < 0)
            return (printf("ft_popen returned %d\n", fd), 0);
        if (argv[1][0] == 'r')
            while ((n = read(fd, buf, sizeof(buf))) > 0)
                write(1, buf, n);
        else
            write(fd, "hello\nworld\n42\n", 15);
        close(fd);
        report(fds);
    }
    else if (argc == 2 && !strcmp(argv[1], "chain"))
    {
        int saved = dup(0);

        fd = ft_popen("printf", (char *const []){"printf", "abc\nxyz\ncat\n", NULL}, 'r');
        dup2(fd, 0);
        close(fd);
        fd = ft_popen("grep", (char *const []){"grep", "c", NULL}, 'r');
        dup2(saved, 0);
        close(saved);
        while ((n = read(fd, buf, sizeof(buf))) > 0)
            write(1, buf, n);
        close(fd);
        report(fds);
    }
    else if (argc == 2 && !strcmp(argv[1], "errors"))
    {
        printf("type 'x': %d\n", ft_popen("ls", (char *const []){"ls", NULL}, 'x'));
        printf("type 0: %d\n", ft_popen("ls", (char *const []){"ls", NULL}, 0));
        printf("file NULL: %d\n", ft_popen(NULL, (char *const []){"ls", NULL}, 'r'));
        printf("argv NULL: %d\n", ft_popen("ls", NULL, 'r'));
        report(fds);
    }
    return (0);
}
