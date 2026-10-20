#include "term.hpp"
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <string>

// terminal state, global so sigc/sigd can restore it
static struct termios g_saved_termios;
static bool g_raw_active = false;
static bool g_screen_active = false;
static int g_tty_fd = -1; // stdout as it was when full screen started
static volatile sig_atomic_t g_resized = 0;
static int g_quiet_fds[3] = {-1, -1, -1}; // stdin/out/err saved by term_quiet_begin

int term_width(void)
{
    struct winsize ws;
    if (ioctl(g_tty_fd >= 0 ? g_tty_fd : STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return (ws.ws_col);
    return (80);
}

int term_height(void)
{
    struct winsize ws;
    if (ioctl(g_tty_fd >= 0 ? g_tty_fd : STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0)
        return (ws.ws_row);
    return (24);
}

bool term_color(void)
{
    static int color = -1;
    if (color == -1)
        color = isatty(STDOUT_FILENO) && getenv("NO_COLOR") == NULL;
    return (color);
}

// ==> First locale variable that is set decides, like setlocale does
static bool utf8_locale(void)
{
    const char *vars[] = {"LC_ALL", "LC_CTYPE", "LANG"};
    for (int i = 0; i < 3; i++)
    {
        const char *value = getenv(vars[i]);
        if (value == NULL || *value == '\0')
            continue;
        std::string s = value;
        for (size_t j = 0; j < s.size(); j++)
            s[j] = tolower(s[j]);
        return (s.find("utf-8") != std::string::npos || s.find("utf8") != std::string::npos);
    }
    return (false);
}

bool term_unicode(void)
{
    static int unicode = -1;
    if (unicode == -1)
    {
        const char *ascii = getenv("EXAMSHELL_ASCII");
        unicode = !(ascii && std::string(ascii) == "1") && utf8_locale();
    }
    return (unicode);
}

// ==> Put the terminal back in line mode with a visible cursor. Keys still waiting are
// dropped (TCSAFLUSH), so stray arrow-key bytes don't land in the next line read.
void term_raw_restore(void)
{
    if (!g_raw_active)
        return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_saved_termios);
    write(STDOUT_FILENO, "\e[?25h", 6);
    g_raw_active = false;
}

// ==> Already in raw mode: nothing to do, and the saved line-mode settings are kept.
// TCSANOW keeps keys typed ahead (TCSAFLUSH would drop them).
bool term_raw_enter(void)
{
    if (g_raw_active)
        return (true);
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
        return (false);
    if (tcgetattr(STDIN_FILENO, &g_saved_termios) == -1)
        return (false);
    struct termios raw = g_saved_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1)
        return (false);
    g_raw_active = true;
    write(STDOUT_FILENO, "\e[?25l", 6);
    return (true);
}

// ==> Full screen

static void on_winch(int)
{
    g_resized = 1;
}

static void write_all(int fd, const char *s, size_t len)
{
    while (len > 0)
    {
        ssize_t n = write(fd, s, len);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            return;
        s += n;
        len -= n;
    }
}

// ==> Raw mode stays on until term_screen_leave(): switching it per key would drop keys typed ahead
bool term_screen_enter(void)
{
    if (g_screen_active)
        return (true);
    if (!term_raw_enter())
        return (false);
    g_tty_fd = dup(STDOUT_FILENO);
    if (g_tty_fd < 0)
    {
        term_raw_restore();
        return (false);
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_winch;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // no SA_RESTART: a resize wakes up the poll() of term_read_key_timeout
    sigaction(SIGWINCH, &sa, NULL);
    g_resized = 0;
    g_screen_active = true;
    const char seq[] = "\e[?1049h\e[?25l\e[?7l\e[H\e[2J"; // no autowrap: writing the last column never scrolls
    write_all(g_tty_fd, seq, sizeof(seq) - 1);
    return (true);
}

// ==> Point stdin, stdout and stderr at /dev/null while a child program runs over the full screen:
// its output would draw over the frame, and it would read the keys typed meanwhile (they stay queued)
void term_quiet_begin(void)
{
    if (g_quiet_fds[0] >= 0)
        return;
    int null_fd = open("/dev/null", O_RDWR);
    if (null_fd < 0)
        return;
    for (int fd = 0; fd < 3; fd++)
    {
        g_quiet_fds[fd] = dup(fd);
        dup2(null_fd, fd);
    }
    close(null_fd);
}

void term_quiet_end(void)
{
    if (g_quiet_fds[0] < 0)
        return;
    for (int fd = 0; fd < 3; fd++)
    {
        dup2(g_quiet_fds[fd], fd);
        close(g_quiet_fds[fd]);
        g_quiet_fds[fd] = -1;
    }
}

// ==> Only write(), dup2() and tcsetattr(), so it can run from a signal handler
void term_screen_leave(void)
{
    term_quiet_end();
    if (!g_screen_active)
        return;
    g_screen_active = false;
    signal(SIGWINCH, SIG_DFL);
    const char seq[] = "\e[0m\e[?7h\e[?1049l\e[?25h";
    write_all(g_tty_fd, seq, sizeof(seq) - 1);
    close(g_tty_fd);
    g_tty_fd = -1;
    term_raw_restore();
}

bool term_screen_active(void)
{
    return (g_screen_active);
}

bool term_resized(void)
{
    if (!g_resized)
        return (false);
    g_resized = 0;
    return (true);
}

void term_write(const std::string &s)
{
    write_all(g_tty_fd >= 0 ? g_tty_fd : STDOUT_FILENO, s.data(), s.size());
}

// ==> Next byte of an escape sequence, if it arrives within a short delay
static bool next_byte(char &b)
{
    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
    return (poll(&pfd, 1, 50) > 0 && read(STDIN_FILENO, &b, 1) > 0);
}

Key term_read_key_timeout(char &c, int ms)
{
    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
    int ready = poll(&pfd, 1, ms);
    if (ready <= 0)
        return (Key::None); // timeout, or a signal such as SIGWINCH
    return (term_read_key(c));
}

Key term_read_key(char &c)
{
    if (read(STDIN_FILENO, &c, 1) <= 0)
        return (Key::Eof);
    if (c == '\e')
    {
        // a lone Esc has nothing following it within a short delay
        char b;
        if (!next_byte(b))
            return (Key::Esc);
        if (b != '[' && b != 'O')
            return (Key::Other);
        // CSI: parameters, then a final byte in @..~ (\e[A, \e[5~, \e[1;5A)
        std::string params;
        while (next_byte(b) && !(b >= '@' && b <= '~'))
            params += b;
        if (b == 'A')
            return (Key::Up);
        if (b == 'B')
            return (Key::Down);
        if (b == 'C')
            return (Key::Right);
        if (b == 'D')
            return (Key::Left);
        if (b == '~' && params == "5")
            return (Key::PageUp);
        if (b == '~' && params == "6")
            return (Key::PageDown);
        return (Key::Other);
    }
    if (c == '\t')
        return (Key::Tab);
    if (c == 16)
        return (Key::Up); // Ctrl-P
    if (c == 14)
        return (Key::Down); // Ctrl-N
    if (c == '\n' || c == '\r')
        return (Key::Enter);
    if (c == 127 || c == 8)
        return (Key::Backspace);
    if (c >= 32 && c < 127)
        return (Key::Char);
    return (Key::Other);
}
