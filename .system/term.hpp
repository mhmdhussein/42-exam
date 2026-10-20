#pragma once

#include <string>

// ==> Terminal info and raw key reading, shared by every interactive screen

int term_width(void);  // columns of stdout, 80 when unknown
int term_height(void); // rows of stdout, 24 when unknown
bool term_color(void);   // stdout is a terminal and NO_COLOR is not set
bool term_unicode(void); // UTF-8 locale and EXAMSHELL_ASCII is not 1

// raw mode: no line buffering, no echo, hidden cursor
bool term_raw_enter(void);   // false when stdin/stdout is not a terminal
void term_raw_restore(void); // safe to call any time, also from signal handlers

// full screen: separate screen buffer, raw mode, hidden cursor, SIGWINCH noted.
// Raw mode stays on until term_screen_leave(), which puts back the screen shown before.
bool term_screen_enter(void); // false when stdin/stdout is not a terminal
void term_screen_leave(void); // safe to call any time, also from signal handlers and atexit
bool term_screen_active(void);
bool term_resized(void);                 // the window changed size since the last call
void term_write(const std::string &s);   // one write() to the terminal, even while stdout is redirected
// stdin/stdout/stderr on /dev/null while a child program runs under the full screen
void term_quiet_begin(void);
void term_quiet_end(void); // also done by term_screen_leave()

enum class Key
{
    Up,
    Down,
    Left,
    Right,
    PageUp,
    PageDown,
    Tab,
    Enter,
    Esc,
    Backspace,
    Char, // printable character, stored in c
    Other,
    None, // term_read_key_timeout: nothing typed in time, or interrupted by a signal
    Eof
};

// ==> Block until one key is read (raw mode must be on)
Key term_read_key(char &c);
// ==> Same, but Key::None after `ms` milliseconds without input
Key term_read_key_timeout(char &c, int ms);
