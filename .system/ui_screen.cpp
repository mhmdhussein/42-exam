#include "ui.hpp"
#include "term.hpp"
#include <algorithm>

namespace ui
{

void split_h(const Rect &r, int size, Rect &left, Rect &right)
{
    int first = size >= 0 ? size : r.w + size;
    first = std::min(std::max(first, 0), r.w);
    left = {r.x, r.y, first, r.h};
    right = {r.x + first, r.y, r.w - first, r.h};
}

void split_v(const Rect &r, int size, Rect &top, Rect &bottom)
{
    int first = size >= 0 ? size : r.h + size;
    first = std::min(std::max(first, 0), r.h);
    top = {r.x, r.y, r.w, first};
    bottom = {r.x, r.y + first, r.w, r.h - first};
}

Screen::Screen(void) : width_(0), height_(0), full_(true)
{
}

void Screen::resize(int width, int height)
{
    width_ = std::max(width, 1);
    height_ = std::max(height, 1);
    cells_.assign(width_ * height_, Cell{" ", ""});
    shown_.clear();
    full_ = true;
}

int Screen::width(void) const
{
    return (width_);
}

int Screen::height(void) const
{
    return (height_);
}

void Screen::clear(void)
{
    std::fill(cells_.begin(), cells_.end(), Cell{" ", ""});
}

void Screen::fill(const Rect &r)
{
    for (int y = std::max(r.y, 0); y < r.y + r.h && y < height_; y++)
        for (int x = std::max(r.x, 0); x < r.x + r.w && x < width_; x++)
            cells_[y * width_ + x] = Cell{" ", ""};
}

// ==> The style is the SGR codes met since the last reset, so paint() nests as on a terminal
int Screen::put(int x, int y, const std::string &text, int max_width)
{
    if (y < 0 || y >= height_)
        return (0);
    int limit = max_width < 0 ? width_ - x : std::min(max_width, width_ - x);
    std::string style;
    int col = 0;
    for (size_t i = 0; i < text.size() && col < limit;)
    {
        unsigned char b = text[i];
        if (b == '\e' && i + 1 < text.size() && text[i + 1] == '[')
        {
            size_t j = i + 2;
            while (j < text.size() && !(text[j] >= '@' && text[j] <= '~'))
                j++;
            std::string seq = text.substr(i, j - i + 1);
            if (j < text.size() && text[j] == 'm')
                style = (seq == "\e[0m" || seq == "\e[m") ? "" : style + seq;
            i = j + 1;
            continue;
        }
        size_t len = 1;
        if (b >= 0xC0)
            while (i + len < text.size() && ((unsigned char)text[i + len] & 0xC0) == 0x80)
                len++;
        std::string ch = b < 32 || b == 127 ? " " : text.substr(i, len); // control characters would move the cursor
        if (x + col >= 0)
            cells_[y * width_ + x + col] = Cell{ch, style};
        col++;
        i += len;
    }
    return (col);
}

void Screen::invalidate(void)
{
    full_ = true;
}

void Screen::flush(void)
{
    std::string out;
    if (full_ || shown_.size() != cells_.size())
    {
        out = "\e[0m\e[H\e[2J";
        shown_.assign(cells_.size(), Cell{"", ""}); // matches no cell, so all are sent
    }
    std::string style;
    int cur_x = -1, cur_y = -1;
    for (int y = 0; y < height_; y++)
        for (int x = 0; x < width_; x++)
        {
            const Cell &c = cells_[y * width_ + x];
            if (c == shown_[y * width_ + x])
                continue;
            if (cur_y != y || cur_x != x)
                out += "\e[" + std::to_string(y + 1) + ";" + std::to_string(x + 1) + "H";
            if (c.style != style)
            {
                out += "\e[0m" + c.style;
                style = c.style;
            }
            out += c.ch;
            cur_x = x + 1;
            cur_y = y;
        }
    if (!style.empty())
        out += "\e[0m";
    shown_ = cells_;
    full_ = false;
    if (!out.empty())
        term_write(out);
}

std::string border(Border b)
{
    bool u = term_unicode();
    switch (b)
    {
    case Border::TopLeft:
        return (u ? "┌" : "+");
    case Border::TopRight:
        return (u ? "┐" : "+");
    case Border::BottomLeft:
        return (u ? "└" : "+");
    case Border::BottomRight:
        return (u ? "┘" : "+");
    case Border::Horizontal:
        return (u ? "─" : "-");
    case Border::Vertical:
        return (u ? "│" : "|");
    case Border::TeeRight:
        return (u ? "├" : "+");
    case Border::TeeLeft:
        return (u ? "┤" : "+");
    case Border::TeeDown:
        return (u ? "┬" : "+");
    case Border::TeeUp:
        return (u ? "┴" : "+");
    }
    return ("");
}

void panel(Screen &s, const Rect &r, const std::string &title, const std::string &label, Color c)
{
    if (r.w < 2 || r.h < 2)
        return;
    std::string h = border(Border::Horizontal), v = border(Border::Vertical);
    std::string line;
    for (int i = 0; i < r.w - 2; i++)
        line += h;
    s.put(r.x, r.y, paint(c, border(Border::TopLeft) + line + border(Border::TopRight)));
    for (int y = r.y + 1; y < r.y + r.h - 1; y++)
    {
        s.put(r.x, y, paint(c, v));
        s.put(r.x + r.w - 1, y, paint(c, v));
    }
    s.put(r.x, r.y + r.h - 1, paint(c, border(Border::BottomLeft) + line + border(Border::BottomRight)));

    // ┌─ title ───── label ─┐
    int room = r.w - 4;
    std::string t = title.empty() ? "" : " " + truncate(title, std::max(room - 2, 1)) + " ";
    int used = s.put(r.x + 2, r.y, t, room);
    if (display_width(label) > 0 && room - used > 6)
    {
        std::string l = " " + truncate(label, room - used - 4) + " ";
        s.put(r.x + r.w - 2 - display_width(l), r.y, l);
    }
}

void text(Screen &s, const Rect &r, const std::vector<std::string> &lines, int scroll)
{
    for (int row = 0; row < r.h; row++)
    {
        size_t i = scroll + row;
        if (scroll + row < 0 || i >= lines.size())
            continue;
        s.put(r.x, r.y + row, truncate(lines[i], r.w), r.w);
    }
}

Rect frame(Screen &s, const FrameOpts &o)
{
    int W = s.width(), H = s.height();
    Rect head = {0, 0, W, 3};
    Rect content = {0, 2, W, H - 3}; // shares the title panel's bottom border, leaves the key bar
    panel(s, head, o.title, o.label, Color::Box);
    int in_w = W - 4;
    int right_w = display_width(o.line_right);
    s.put(2, 1, truncate(o.line, std::max(in_w - right_w - 1, 1)));
    if (right_w > 0)
        s.put(W - 2 - right_w, 1, o.line_right);
    panel(s, content, o.panel_title, o.panel_label, o.color);
    std::string jc = esc(o.color), end = reset();
    s.put(0, 2, jc + border(Border::TeeRight) + end);
    s.put(W - 1, 2, jc + border(Border::TeeLeft) + end);
    return (Rect{3, 4, W - 6, content.h - 4}); // a blank row and 3 columns inside the border
}

void key_bar(Screen &s, const std::string &left, const std::string &right)
{
    int W = s.width(), H = s.height();
    std::string r = right.empty() ? "" : right + " ";
    int right_w = display_width(r);
    s.put(1, H - 1, truncate(left, std::max(W - right_w - 3, 1)));
    if (right_w > 0)
        s.put(W - right_w, H - 1, r);
}

std::string highlight(const std::string &s)
{
    if (!term_color())
        return (s);
    return ("\e[30;102m" + s + "\e[0m");
}

// ==> Same columns as Modern's menus: "     1   label" and "  [ ▸ 1   label ]"
std::vector<std::string> list_lines(const std::vector<ListRow> &rows, int cursor)
{
    std::vector<std::string> lines;
    for (int i = 0; i < (int)rows.size(); i++)
    {
        if (i == cursor)
            lines.push_back("  " + highlight(" " + sym(Sym::Prompt) + " " + rows[i].plain + " "));
        else
            lines.push_back("     " + rows[i].normal);
    }
    return (lines);
}

int list_key(const std::vector<ListRow> &rows, int &cursor, Key k, char c, bool can_back)
{
    int count = rows.size();
    if (count == 0)
        return ((k == Key::Esc || (k == Key::Char && c == 'q')) && can_back ? LIST_BACK : LIST_NONE);
    if (cursor < 0 || cursor >= count)
        cursor = 0;
    if (k == Key::Up || (k == Key::Char && c == 'k'))
        cursor = (cursor + count - 1) % count;
    else if (k == Key::Down || (k == Key::Char && c == 'j'))
        cursor = (cursor + 1) % count;
    else if (k == Key::Enter)
        return (cursor);
    else if ((k == Key::Esc || (k == Key::Char && c == 'q')) && can_back)
        return (LIST_BACK);
    else if (k == Key::Char)
    {
        for (int i = 0; i < count; i++)
            if (rows[i].key == std::string(1, c))
            {
                cursor = i;
                return (i);
            }
    }
    return (LIST_NONE);
}

// ==> Byte offset after `cols` characters of a plain string
static size_t cut_at(const std::string &s, int cols)
{
    size_t i = 0;
    for (int n = 0; i < s.size() && n < cols; n++)
    {
        i++;
        while (i < s.size() && ((unsigned char)s[i] & 0xC0) == 0x80)
            i++;
    }
    return (i);
}

std::vector<std::string> wrap(const std::string &line, int width)
{
    std::vector<std::string> out;
    if (width < 1)
        width = 1;
    std::string rest = line;
    while (display_width(rest) > width)
    {
        size_t limit = cut_at(rest, width);
        size_t space = rest.rfind(' ', limit);
        size_t lead = rest.find_first_not_of(' ');
        if (space == std::string::npos || space == 0 || (lead != std::string::npos && space < lead))
        {
            out.push_back(rest.substr(0, limit)); // no space to break at: cut the word
            rest = rest.substr(limit);
        }
        else
        {
            out.push_back(rest.substr(0, space));
            rest = rest.substr(space + 1);
        }
    }
    out.push_back(rest);
    return (out);
}

}
