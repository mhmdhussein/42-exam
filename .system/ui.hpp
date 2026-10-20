#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include "term.hpp"

// ==> Building blocks for the modern screens. Colour and Unicode follow term_color() / term_unicode().
namespace ui
{

enum class Color
{
    Text,
    Dim,
    Box,
    Success,
    Failure,
    Current,
    Warning,
    Mode,
    Bold
};

enum class Sym
{
    Check,
    Cross,
    Current,
    Pending,
    Clock,
    Cooldown,
    Prompt,
    Ellipsis
};

enum class Align
{
    Left,
    Right
};

std::string esc(Color c); // empty without colour
std::string reset(void);
std::string paint(Color c, const std::string &s);
std::string sym(Sym s);

// widths count characters on screen: escape codes are skipped, a UTF-8 character counts once
int display_width(const std::string &s);
std::string truncate(const std::string &s, int width); // ends with … when cut
std::string pad(const std::string &s, int width, Align align = Align::Left);

struct BoxOpts
{
    std::string title;  // left of the top border
    std::string label;  // right of the top border
    std::string footer; // left of the bottom border
    Color color = Color::Box;
    int width = 0;     // outer width, 0 = fit content and the terminal
    int min_width = 0; // with width 0: never narrower than this (screens shown together line up)
};

// ==> Rounded box indented by 2, one string with a newline after each row
std::string box(const std::vector<std::string> &lines, const BoxOpts &opts = BoxOpts());
std::string divider(int width = 0, const std::string &label = "");
std::vector<std::string> columns(const std::vector<std::vector<std::string> > &rows, int gap = 2);
std::string progress(int value, int max, int width, const std::string &label = "");

// ==> Full-screen drawing (ui_screen.cpp): a grid of cells, sent as the difference with the last frame

struct Rect
{
    int x;
    int y;
    int w;
    int h;
};

// size > 0: width (height) of the first part, size < 0: of the second part
void split_h(const Rect &r, int size, Rect &left, Rect &right);
void split_v(const Rect &r, int size, Rect &top, Rect &bottom);

class Screen
{
public:
    Screen(void);
    void resize(int width, int height); // also invalidates
    int width(void) const;
    int height(void) const;
    void clear(void);
    // styled text (escape codes from paint()) from (x, y), cut at max_width columns (-1 = screen edge);
    // returns the columns written
    int put(int x, int y, const std::string &text, int max_width = -1);
    void fill(const Rect &r);    // blanks
    void flush(void);            // writes the changed cells with one term_write()
    void invalidate(void);       // next flush redraws everything

private:
    struct Cell
    {
        std::string ch;
        std::string style;
        bool operator==(const Cell &o) const { return (ch == o.ch && style == o.style); }
    };
    int width_;
    int height_;
    bool full_;
    std::vector<Cell> cells_;
    std::vector<Cell> shown_;
};

enum class Border
{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Horizontal,
    Vertical,
    TeeRight, // ├
    TeeLeft,  // ┤
    TeeDown,  // ┬
    TeeUp     // ┴
};
std::string border(Border b); // square box-drawing character, or + - | in ASCII

// ==> Square box on the rect: ┌─ title ──── label ─┐ (title and label may hold colour)
void panel(Screen &s, const Rect &r, const std::string &title, const std::string &label, Color c);
// ==> Lines from `scroll` inside the rect, each cut to its width
void text(Screen &s, const Rect &r, const std::vector<std::string> &lines, int scroll = 0);
// ==> Plain text word-wrapped to `width` columns; words longer than a line are cut
std::vector<std::string> wrap(const std::string &line, int width);

// ==> Page frame: title panel (3 rows), one content panel under it, the last row left for key_bar
struct FrameOpts
{
    std::string title;      // title panel border, left
    std::string label;      // title panel border, right
    std::string line;       // inside the title panel, left
    std::string line_right; // inside the title panel, right
    std::string panel_title;
    std::string panel_label;
    Color color = Color::Box; // content panel
};
Rect frame(Screen &s, const FrameOpts &o); // returns the inside of the content panel
// ==> Last row: left text cut so the right one fits
void key_bar(Screen &s, const std::string &left, const std::string &right);

// ==> Inverted row; s must not hold escape codes
std::string highlight(const std::string &s);

// ==> Full-screen list: one row per item, the one at the cursor highlighted
struct ListRow
{
    std::string key;    // typed to pick the row directly ("" = none)
    std::string normal; // may hold colour
    std::string plain;  // same without colour, shown highlighted
};
std::vector<std::string> list_lines(const std::vector<ListRow> &rows, int cursor);
// ↑↓/j/k move, enter picks, a row's key picks it, esc/q goes back when allowed.
// Returns the picked row, LIST_BACK, or LIST_NONE when the key only moved the cursor (or did nothing).
const int LIST_NONE = -1;
const int LIST_BACK = -2;
int list_key(const std::vector<ListRow> &rows, int &cursor, Key k, char c, bool can_back);

// ==> One-line spinner redrawn in place while a blocking call runs on the calling thread
class Spinner
{
public:
    Spinner(void);
    ~Spinner(void);
    // suffix, when given, is called each frame and its result shown dimmed after the text
    void start(const std::string &text, std::function<std::string(void)> suffix = nullptr);
    void set_text(const std::string &text);
    void stop(void); // clears the line

private:
    void run(void);

    std::thread thread_;
    std::mutex mutex_;
    std::atomic<bool> running_;
    std::string text_;
    std::function<std::string(void)> suffix_;
    bool animated_;
};

}
