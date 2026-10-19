#pragma once

// ==> Terminal info and raw key reading, shared by every interactive screen

int term_width(void);  // columns of stdout, 80 when unknown
int term_height(void); // rows of stdout, 24 when unknown
bool term_color(void);   // stdout is a terminal and NO_COLOR is not set
bool term_unicode(void); // UTF-8 locale and EXAMSHELL_ASCII is not 1

// raw mode: no line buffering, no echo, hidden cursor
bool term_raw_enter(void);   // false when stdin/stdout is not a terminal
void term_raw_restore(void); // safe to call any time, also from signal handlers

enum class Key
{
    Up,
    Down,
    Left,
    Right,
    Enter,
    Esc,
    Backspace,
    Char, // printable character, stored in c
    Other,
    Eof
};

// ==> Block until one key is read (raw mode must be on)
Key term_read_key(char &c);
