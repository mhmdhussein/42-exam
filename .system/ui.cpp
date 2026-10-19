#include "ui.hpp"
#include "term.hpp"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>

namespace ui
{

std::string esc(Color c)
{
    if (!term_color())
        return ("");
    switch (c)
    {
    case Color::Text:
        return ("\e[97m");
    case Color::Dim:
    case Color::Box:
        return ("\e[90m");
    case Color::Success:
        return ("\e[92m");
    case Color::Failure:
        return ("\e[91m");
    case Color::Current:
        return ("\e[36m");
    case Color::Warning:
        return ("\e[93m");
    case Color::Mode:
        return ("\e[95m");
    case Color::Bold:
        return ("\e[1m");
    }
    return ("");
}

std::string reset(void)
{
    return (term_color() ? "\e[0m" : "");
}

std::string paint(Color c, const std::string &s)
{
    if (!term_color())
        return (s);
    return (esc(c) + s + reset());
}

std::string sym(Sym s)
{
    bool u = term_unicode();
    switch (s)
    {
    case Sym::Check:
        return (u ? "✓" : "v");
    case Sym::Cross:
        return (u ? "✗" : "x");
    case Sym::Current:
        return (u ? "▸" : ">");
    case Sym::Pending:
        return (u ? "·" : ".");
    case Sym::Clock:
        return (u ? "⏱" : "T");
    case Sym::Cooldown:
        return (u ? "⟳" : "~");
    case Sym::Prompt:
        return (u ? "❯" : ">");
    case Sym::Ellipsis:
        return (u ? "…" : "...");
    }
    return ("");
}

// ==> Length of the escape sequence starting at i, 0 if there is none
static size_t escape_len(const std::string &s, size_t i)
{
    if (s[i] != '\e' || i + 1 >= s.size() || s[i + 1] != '[')
        return (0);
    size_t j = i + 2;
    while (j < s.size() && !(s[j] >= '@' && s[j] <= '~'))
        j++;
    return (j < s.size() ? j - i + 1 : s.size() - i);
}

int display_width(const std::string &s)
{
    int width = 0;
    for (size_t i = 0; i < s.size(); i++)
    {
        size_t e = escape_len(s, i);
        if (e)
            i += e - 1;
        else if (((unsigned char)s[i] & 0xC0) != 0x80)
            width++;
    }
    return (width);
}

std::string truncate(const std::string &s, int width)
{
    if (display_width(s) <= width)
        return (s);
    std::string dots = sym(Sym::Ellipsis);
    int keep = std::max(width - display_width(dots), 0);
    std::string out;
    bool escaped = false;
    int count = 0;
    for (size_t i = 0; i < s.size(); i++)
    {
        size_t e = escape_len(s, i);
        if (e)
        {
            out += s.substr(i, e);
            escaped = true;
            i += e - 1;
            continue;
        }
        if (((unsigned char)s[i] & 0xC0) != 0x80)
        {
            if (count == keep)
                break;
            count++;
        }
        out += s[i];
    }
    if (escaped)
        out += "\e[0m";
    return (out + (width >= display_width(dots) ? dots : ""));
}

std::string pad(const std::string &s, int width, Align align)
{
    std::string cut = truncate(s, width);
    std::string fill(std::max(width - display_width(cut), 0), ' ');
    return (align == Align::Left ? cut + fill : fill + cut);
}

static std::string repeat(const std::string &s, int n)
{
    std::string out;
    for (int i = 0; i < n; i++)
        out += s;
    return (out);
}

std::string box(const std::vector<std::string> &lines, const BoxOpts &opts)
{
    bool u = term_unicode();
    std::string h = u ? "─" : "-", v = u ? "│" : "|";
    std::string tl = u ? "╭" : "+", tr = u ? "╮" : "+", bl = u ? "╰" : "+", br = u ? "╯" : "+";

    int max_width = std::max(term_width() - 2, 20);
    int width = opts.width;
    if (width <= 0)
    {
        width = 46;
        for (size_t i = 0; i < lines.size(); i++)
            width = std::max(width, display_width(lines[i]) + 6);
        width = std::max(width, display_width(opts.title) + display_width(opts.label) + 10);
        width = std::max(width, display_width(opts.footer) + 6);
    }
    width = std::min(std::max(width, 20), max_width);

    std::string border = esc(opts.color);
    std::string end = reset();
    std::ostringstream out;

    // top: ╭─ title ─────── label ─╮
    std::string title = opts.title.empty() ? "" : " " + truncate(opts.title, width - 8) + " ";
    std::string label = opts.label.empty() ? "" : " " + truncate(opts.label, std::max(width - 8 - display_width(title), 1)) + " ";
    int fill = std::max(width - 4 - display_width(title) - display_width(label), 0);
    out << "  " << border << tl << h << end << title << border << repeat(h, fill) << end << label << border << h << tr << end << "\n";

    for (size_t i = 0; i < lines.size(); i++)
        out << "  " << border << v << end << "  " << pad(lines[i], width - 6) << "  " << border << v << end << "\n";

    // bottom: ╰─ footer ───╯
    std::string footer = opts.footer.empty() ? "" : " " + truncate(opts.footer, width - 6) + " ";
    fill = std::max(width - 3 - display_width(footer), 0);
    out << "  " << border << bl << h << end << footer << border << repeat(h, fill) << br << end << "\n";
    return (out.str());
}

std::string divider(int width, const std::string &label)
{
    std::string h = term_unicode() ? "─" : "-";
    if (width <= 0)
        width = std::min(term_width() - 2, 78);
    std::string text = label.empty() ? "" : " " + truncate(label, width - 4) + " ";
    int left = label.empty() ? 0 : 2;
    int right = std::max(width - left - display_width(text), 0);
    return ("  " + esc(Color::Box) + repeat(h, left) + reset() + text + esc(Color::Box) + repeat(h, right) + reset());
}

std::vector<std::string> columns(const std::vector<std::vector<std::string> > &rows, int gap)
{
    std::vector<int> widths;
    for (size_t r = 0; r < rows.size(); r++)
        for (size_t c = 0; c < rows[r].size(); c++)
        {
            if (c >= widths.size())
                widths.push_back(0);
            widths[c] = std::max(widths[c], display_width(rows[r][c]));
        }
    std::vector<std::string> out;
    for (size_t r = 0; r < rows.size(); r++)
    {
        std::string line;
        for (size_t c = 0; c < rows[r].size(); c++)
        {
            if (c + 1 == rows[r].size())
                line += rows[r][c]; // no trailing spaces after the last column
            else
                line += pad(rows[r][c], widths[c]) + std::string(gap, ' ');
        }
        out.push_back(line);
    }
    return (out);
}

std::string progress(int value, int max, int width, const std::string &label)
{
    if (width < 1)
        width = 1;
    int filled = max > 0 ? std::min(std::max(value, 0), max) * width / max : 0;
    bool u = term_unicode();
    std::string bar = paint(Color::Success, repeat(u ? "━" : "#", filled)) + paint(Color::Box, repeat(u ? "░" : "-", width - filled));
    return (label.empty() ? bar : bar + "  " + label);
}

Spinner::Spinner(void) : running_(false), animated_(false)
{
}

Spinner::~Spinner(void)
{
    stop();
}

void Spinner::start(const std::string &text)
{
    stop();
    text_ = text;
    animated_ = isatty(STDOUT_FILENO);
    if (!animated_)
    {
        std::cout << text << std::endl;
        return;
    }
    running_ = true;
    thread_ = std::thread(&Spinner::run, this);
}

void Spinner::set_text(const std::string &text)
{
    std::lock_guard<std::mutex> lock(mutex_);
    text_ = text;
}

void Spinner::stop(void)
{
    if (!running_)
        return;
    running_ = false;
    thread_.join();
    std::cout << "\r\e[K" << std::flush;
}

void Spinner::run(void)
{
    static const char *unicode[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
    static const char *ascii[] = {"|", "/", "-", "\\"};
    bool u = term_unicode();
    int count = u ? 10 : 4;
    for (int f = 0; running_; f = (f + 1) % count)
    {
        std::string line;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            line = "\r  " + paint(Color::Current, u ? unicode[f] : ascii[f]) + " " + truncate(text_, term_width() - 5) + "\e[K";
        }
        std::cout << line << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(90));
    }
}

}
