#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

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
