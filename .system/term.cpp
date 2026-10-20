#include "term.hpp"
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>

// terminal state, global so sigc/sigd can restore it
static struct termios g_saved_termios;
static bool g_raw_active = false;

int term_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return (ws.ws_col);
    return (80);
}

int term_height(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0)
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

Key term_read_key(char &c)
{
    if (read(STDIN_FILENO, &c, 1) <= 0)
        return (Key::Eof);
    if (c == '\e')
    {
        // a lone Esc has nothing following it within a short delay
        struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
        char seq[2];
        if (poll(&pfd, 1, 50) <= 0 || read(STDIN_FILENO, &seq[0], 1) <= 0)
            return (Key::Esc);
        if (poll(&pfd, 1, 50) > 0 && read(STDIN_FILENO, &seq[1], 1) > 0 && (seq[0] == '[' || seq[0] == 'O'))
        {
            if (seq[1] == 'A')
                return (Key::Up);
            if (seq[1] == 'B')
                return (Key::Down);
            if (seq[1] == 'C')
                return (Key::Right);
            if (seq[1] == 'D')
                return (Key::Left);
        }
        return (Key::Other);
    }
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
