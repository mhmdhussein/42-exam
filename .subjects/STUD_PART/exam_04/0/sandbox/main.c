#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

int sandbox(void (*f)(void), unsigned int timeout, bool verbose);

static void nice_fn(void) { }
static void nice_slow(void) { sleep(1); }
static void exit_code(void) { exit(42); }
static void exit_one(void) { exit(1); }
static void segfault(void) { *(volatile int *)0 = 42; }
static void do_abort(void) { abort(); }
static void loop(void) { while (1) ; }
static void ignore_alarm(void) { signal(SIGALRM, SIG_IGN); while (1) ; }
static void kill_self(void) { kill(getpid(), SIGKILL); }
static void fpe(void) { kill(getpid(), SIGFPE); }
static void bus(void) { kill(getpid(), SIGBUS); }
static void spawn(void) { if (fork() == 0) sleep(3); exit(0); }

static void (*funcs[])(void) = {nice_fn, nice_slow, exit_code, exit_one, segfault,
    do_abort, loop, ignore_alarm, kill_self, fpe, bus, spawn};

/* argv[1] = function index, argv[2] = timeout, argv[3] = verbose (0/1) */
int main(int argc, char **argv)
{
    int r;

    if (argc != 4)
        return (1);
    r = sandbox(funcs[atoi(argv[1])], atoi(argv[2]), atoi(argv[3]));
    fflush(stdout);
    printf("return: %d\n", r);
    if (waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD)
        printf("no process left\n");
    else
        printf("leaked process (zombie or still running)\n");
    return (0);
}
