#include "exam.hpp"
#include "term.hpp"
#include "ui.hpp"
#include <algorithm>

using ui::Color;
using ui::Sym;
using ui::paint;
using ui::sym;

// ==> Small helpers for the modern screens

// unicode character, or its ASCII stand-in
static std::string uni(const char *u, const char *a)
{
    return (term_unicode() ? u : a);
}

// separator between items on one line
static std::string dot(void)
{
    return (uni("·", "-"));
}

static std::string bold(const std::string &s)
{
    return (paint(Color::Bold, paint(Color::Text, s)));
}

static std::string dim(const std::string &s)
{
    return (paint(Color::Dim, s));
}

// inverted row for the menu highlight; s must not hold escape codes
static std::string highlight(const std::string &s)
{
    if (!term_color())
        return (s);
    return ("\e[30;102m" + s + "\e[0m");
}

// ==> New screen: like clear(1), scrollback included, so old screens don't pile up above
static std::string clear_screen(void)
{
    return (isatty(STDOUT_FILENO) ? "\e[H\e[2J\e[3J" : "");
}

// ==> Paint `frame` over the previous one (`lines` lines high, -1 = none: start a new screen).
// Lines are overwritten and their ends erased; erasing the whole display instead makes many
// terminals push the old frame into the scrollback, one copy per key press.
static std::string repaint(int lines, const std::string &frame)
{
    if (!isatty(STDOUT_FILENO))
        return (frame);
    if (lines < 0)
        return (clear_screen() + frame);
    std::string out = "\r" + (lines > 0 ? "\e[" + std::to_string(lines) + "A" : std::string());
    for (size_t i = 0; i < frame.size(); i++)
    {
        if (frame[i] == '\n')
            out += "\e[K";
        out += frame[i];
    }
    return (out + "\e[K\e[J"); // \e[J only drops leftovers below a shorter frame
}

static int count_lines(const std::string &s)
{
    return (std::count(s.begin(), s.end(), '\n'));
}

static std::string read_line(void)
{
    std::string line;
    if (!std::getline(std::cin, line))
        sigd();
    return (line);
}

static std::string trim(std::string s)
{
    while (!s.empty() && isspace((unsigned char)s.back()))
        s.pop_back();
    while (!s.empty() && isspace((unsigned char)s.front()))
        s.erase(s.begin());
    return (s);
}

// ==> One key in raw mode; Ctrl+D or a closed stdin leaves like sigd()
static Key read_key(char &c)
{
    Key k = term_read_key(c);
    if (k == Key::Eof || (k == Key::Other && c == 4))
    {
        term_raw_restore();
        sigd();
    }
    return (k);
}

static std::string two(long n)
{
    return ((n < 10 ? "0" : "") + std::to_string(n));
}

// 2:41:07
static std::string clock_long(long s)
{
    if (s < 0)
        s = 0;
    return (std::to_string(s / 3600) + ":" + two(s % 3600 / 60) + ":" + two(s % 60));
}

// 2:41
static std::string clock_short(long s)
{
    if (s < 0)
        s = 0;
    return (std::to_string(s / 3600) + ":" + two(s % 3600 / 60));
}

// 1h 52m, 6m, 45s
static std::string duration(long s)
{
    if (s < 0)
        s = 0;
    if (s < 60)
        return (std::to_string(s) + "s");
    long h = s / 3600, m = s % 3600 / 60;
    if (h == 0)
        return (std::to_string(m) + "m");
    return (std::to_string(h) + "h" + (m ? " " + std::to_string(m) + "m" : ""));
}

// 4m, 4m 12s, 30s
static std::string wait_time(long s)
{
    if (s < 60)
        return (std::to_string(s < 0 ? 0 : s) + "s");
    return (std::to_string(s / 60) + "m" + (s % 60 ? " " + std::to_string(s % 60) + "s" : ""));
}

static std::string tries_text(int n)
{
    return (std::to_string(n) + (n == 1 ? " try" : " tries"));
}

static std::string ordinal(int n)
{
    std::string suffix = "th";
    if (n % 100 < 11 || n % 100 > 13)
    {
        if (n % 10 == 1)
            suffix = "st";
        else if (n % 10 == 2)
            suffix = "nd";
        else if (n % 10 == 3)
            suffix = "rd";
    }
    return (std::to_string(n) + suffix);
}

static std::string exam_title(bool student, int exam_number)
{
    return (std::string(student ? "EXAM RANK 0" : "EXAM WEEK 0") + std::to_string(exam_number));
}

// tabs count as one column in display_width, so expand them
static std::string untab(const std::string &s)
{
    std::string out;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == '\t')
            out += "    ";
        else
            out += s[i];
    }
    return (out);
}

static std::string footer_keys(const std::string &keys)
{
    return ("  " + dim(keys));
}

// ==> ModernDisplay

ModernDisplay::ModernDisplay(void) : start_cursor_(1), part_cursor_(0), settings_cursor_(0), settings_drawn_(-1), next_step_("continue")
{
}

// ==> Arrow-key menu: ↑↓/j/k move, enter picks, a key picks directly, esc/q returns `back`
// (ignored when back is empty). Without a terminal: numbered list and a typed answer.
std::string ModernDisplay::pick(const std::string &header, const std::vector<Option> &options, int &cursor, const std::string &footer, const std::string &back)
{
    int count = options.size();
    if (cursor < 0 || cursor >= count)
        cursor = 0;
    int hint_width = 18;
    for (int i = 0; i < count; i++)
        hint_width = std::max(hint_width, ui::display_width(options[i].hint));
    bool raw = term_raw_enter();
    int drawn = -1; // lines of the frame on screen, -1 before the first one
    while (1)
    {
        std::ostringstream frame;
        frame << header << "\n";
        for (int i = 0; i < count; i++)
        {
            const Option &o = options[i];
            if (raw && i == cursor)
                frame << "  " << highlight(" " + sym(Sym::Prompt) + " " + o.key + "   " + ui::pad(o.label, 18) + "  " + ui::pad(o.hint, hint_width) + " ") << "\n";
            else
                frame << "     " << dim(o.key) << "   " << ui::pad(o.label, 18) << "  " << dim(o.hint) << "\n";
        }
        frame << "\n";
        if (!raw)
        {
            std::cout << repaint(-1, frame.str()) << "  choice: " << std::flush;
            return (trim(read_line()));
        }
        frame << footer;
        std::cout << repaint(drawn, frame.str()) << std::flush;
        drawn = count_lines(frame.str());

        char c = 0;
        Key k = read_key(c);
        std::string picked;
        if (k == Key::Up || (k == Key::Char && c == 'k'))
            cursor = (cursor + count - 1) % count;
        else if (k == Key::Down || (k == Key::Char && c == 'j'))
            cursor = (cursor + 1) % count;
        else if (k == Key::Enter)
            picked = options[cursor].key;
        else if ((k == Key::Esc || (k == Key::Char && c == 'q')) && !back.empty())
            picked = back;
        else if (k == Key::Char)
        {
            for (int i = 0; i < count; i++)
                if (options[i].key == std::string(1, c))
                {
                    cursor = i;
                    picked = options[i].key;
                }
        }
        if (!picked.empty())
        {
            term_raw_restore();
            std::cout << clear_screen() << std::flush;
            return (picked);
        }
    }
}

static std::string menu_header(const std::string &where)
{
    std::string line = bold("42 EXAM") + "  " + dim(dot() + "  " + where);
    return (std::string("\n") + ui::box(std::vector<std::string>(1, line)));
}

std::string ModernDisplay::startMenu(bool invalid)
{
    std::vector<Option> o;
    o.push_back({"1", "Piscine part", "exams 01-04"});
    o.push_back({"2", "Student part", "exams 02-06"});
    o.push_back({"3", "Practice", "pick any exercise"});
    o.push_back({"4", "Settings", ""});
    std::string header = menu_header("offline simulator");
    if (invalid)
        header += "  " + paint(Color::Failure, sym(Sym::Cross) + " pick 1-4, or q to quit") + "\n";
    std::string keys = uni("↑↓", "up/down") + " move " + dot() + " enter select " + dot() + " q quit";
    return (pick(header, o, start_cursor_, footer_keys(keys), "q"));
}

std::string ModernDisplay::studMenu(bool invalid)
{
    std::vector<Option> o;
    for (int i = 2; i <= 6; i++)
        o.push_back({std::to_string(i), "Exam rank 0" + std::to_string(i), ""});
    std::string header = menu_header("student part");
    if (invalid)
        header += "  " + paint(Color::Failure, sym(Sym::Cross) + " pick 2-6, or 0 to go back") + "\n";
    std::string keys = uni("↑↓", "up/down") + " move " + dot() + " enter select " + dot() + " esc back";
    return (pick(header, o, part_cursor_, footer_keys(keys), "0"));
}

std::string ModernDisplay::piscineMenu(bool invalid)
{
    std::vector<Option> o;
    for (int i = 1; i <= 4; i++)
        o.push_back({std::to_string(i), "Exam week 0" + std::to_string(i), ""});
    std::string header = menu_header("piscine part");
    if (invalid)
        header += "  " + paint(Color::Failure, sym(Sym::Cross) + " pick 1-4, or 0 to go back") + "\n";
    std::string keys = uni("↑↓", "up/down") + " move " + dot() + " enter select " + dot() + " esc back";
    return (pick(header, o, part_cursor_, footer_keys(keys), "0"));
}

static std::string toggle(bool on)
{
    return (on ? paint(Color::Success, uni("●", "*") + " on") : dim(uni("○", "o") + " off"));
}

static const char *style_label(DisplayStyle style)
{
    if (style == DisplayStyle::Exact)
        return ("Exact examshell");
    if (style == DisplayStyle::Legacy)
        return ("Legacy");
    return ("Modern");
}

// ==> Returns the key of the setting to change ("-3" = display style backwards), "0" to save
std::string ModernDisplay::settingsMenu(const SettingsView &s)
{
    struct Row
    {
        std::string key;
        std::string label;
        std::string value;  // coloured
        std::string plain;  // same without colour, for the highlighted row
    };
    std::string style = uni("‹ ", "< ") + style_label(s.style) + uni(" ›", " >");
    std::vector<Row> rows;
    rows.push_back({"3", "Display style", paint(Color::Current, style), style});
    rows.push_back({"1", "Enable passed exercises", toggle(s.dse), s.dse ? uni("●", "*") + " on" : uni("○", "o") + " off"});
    rows.push_back({"2", "Cheat commands", toggle(s.dcc), s.dcc ? uni("●", "*") + " on" : uni("○", "o") + " off"});
    rows.push_back({"4", "Login animation", toggle(s.anim), s.anim ? uni("●", "*") + " on" : uni("○", "o") + " off"});
    rows.push_back({"5", "Cooldown in Practice mode", toggle(s.pcd), s.pcd ? uni("●", "*") + " on" : uni("○", "o") + " off"});
    int count = rows.size();
    if (settings_cursor_ < 0 || settings_cursor_ >= count)
        settings_cursor_ = 0;

    bool raw = term_raw_enter();
    std::vector<std::string> lines(1, "");
    for (int i = 0; i < count; i++)
    {
        if (!raw)
            lines.push_back(dim(rows[i].key + ".") + " " + ui::pad(rows[i].label, 27) + rows[i].value);
        else if (i == settings_cursor_)
            lines.push_back(highlight(sym(Sym::Prompt) + " " + ui::pad(rows[i].label, 27) + ui::pad(rows[i].plain, 18)));
        else
            lines.push_back("  " + ui::pad(rows[i].label, 27) + rows[i].value);
    }
    lines.push_back("");
    ui::BoxOpts opts;
    opts.title = bold("Settings");
    std::string arrows = uni("↑↓", "up/down") + " move " + dot() + " " + uni("←→", "left/right") + " / space change " + dot() + " esc save";
    if (raw)
        opts.footer = dim(arrows);
    // the exam calls this once per key: the first call clears, the next ones repaint in place
    std::string frame = "\n" + ui::box(lines, opts) + "\n";
    std::cout << repaint(settings_drawn_, frame);
    settings_drawn_ = count_lines(frame);

    if (!raw)
    {
        settings_drawn_ = -1;
        std::cout << "  number to change, enter to save: " << std::flush;
        std::string input = trim(read_line());
        return (input.empty() ? "0" : input);
    }
    std::cout << std::flush;
    char c = 0;
    Key k = read_key(c);
    term_raw_restore();
    const Row &row = rows[settings_cursor_];
    if (k == Key::Up || (k == Key::Char && c == 'k'))
        settings_cursor_ = (settings_cursor_ + count - 1) % count;
    else if (k == Key::Down || (k == Key::Char && c == 'j'))
        settings_cursor_ = (settings_cursor_ + 1) % count;
    else if (k == Key::Left)
        return (row.key == "3" ? "-3" : row.key);
    else if (k == Key::Right || k == Key::Enter || (k == Key::Char && c == ' '))
        return (row.key);
    else if (k == Key::Esc || (k == Key::Char && (c == 'q' || c == '0')))
    {
        settings_drawn_ = -1;
        std::cout << clear_screen() << std::flush;
        return ("0");
    }
    else if (k == Key::Char && c >= '1' && c <= '5')
        return (std::string(1, c));
    return (""); // redraw
}

// ==> Saved exam found at startup: "1" to restore, "2" to erase
std::string ModernDisplay::restorePrompt(const RestoreView &r)
{
    std::vector<std::string> lines;
    lines.push_back(dim("exam      ") + bold(exam_title(r.student, r.exam_number)));
    lines.push_back(dim("exercise  ") + r.ex_name);
    lines.push_back(dim("time left ") + paint(Color::Warning, clock_long(r.end_time - time(NULL))));
    ui::BoxOpts opts;
    opts.title = bold("Saved exam found");
    std::string header = "\n" + ui::box(lines, opts);

    std::vector<Option> o;
    o.push_back({"1", "Restore", "continue the exam"});
    o.push_back({"2", "Erase", "delete it, start over"});
    int cursor = 0;
    std::string keys = uni("↑↓", "up/down") + " move " + dot() + " enter select";
    std::string answer;
    while (answer != "1" && answer != "2")
        answer = pick(header, o, cursor, footer_keys(keys), "");
    return (answer);
}

bool ModernDisplay::confirmRegistration(bool student, int exam_number)
{
    std::cout << std::endl
              << "  " << bold(exam_title(student, exam_number)) << dim("  " + dot() + "  ") << "start this exam? " << dim("[Y/n] ") << std::flush;
    std::string answer = trim(read_line());
    return (answer.empty() || answer == "y" || answer == "Y");
}

void ModernDisplay::showExplanation(void)
{
    std::vector<std::string> lines;
    lines.push_back("");
    lines.push_back(paint(Color::Current, sym(Sym::Current)) + " Work in another terminal: keep this one for the exam shell.");
    lines.push_back(paint(Color::Current, sym(Sym::Current)) + " Your subject is in " + bold("subjects/subject.en.txt") + ".");
    lines.push_back(paint(Color::Current, sym(Sym::Current)) + " Put your files in " + bold("rendu/<exercise>/") + ". No git needed here,");
    lines.push_back("  but the real exam makes you push with git.");
    lines.push_back(paint(Color::Current, sym(Sym::Current)) + " Type " + paint(Color::Success, "grademe") + " to be graded. Pass to reach the next level;");
    lines.push_back("  fail and you retry the same exercise for fewer points.");
    lines.push_back(paint(Color::Current, sym(Sym::Current)) + " Each failed try makes you wait longer before the next grade.");
    lines.push_back("");
    lines.push_back(paint(Color::Warning, uni("⚠", "!") + " Not the real 42 exam and not made by 42."));
    lines.push_back(dim("  A private, offline exam simulator."));
    lines.push_back("");
    ui::BoxOpts opts;
    opts.title = bold("How it works");
    std::cout << clear_screen() << std::endl
              << ui::box(lines, opts) << std::endl
              << footer_keys("enter " + sym(Sym::Prompt) + " continue") << std::flush;
    read_line();
}

void ModernDisplay::connexion(const std::string &username)
{
    play_login(username);
}

void ModernDisplay::showIntro(const std::string &username, bool student, int exam_number, int level, int time_max)
{
    std::cout << "  " << paint(Color::Success, sym(Sym::Check)) << " connected as " << bold(username) << std::endl
              << std::endl;
    std::cout << "  " << bold(exam_title(student, exam_number)) << dim("  " + dot() + "  ") << paint(Color::Mode, "REAL") << dim(" mode  " + dot() + "  ")
              << "level " << level << dim("  " + dot() + "  ") << duration(time_max * 60L) << std::endl;
    std::cout << dim("  Points you earn count whatever happens. You can log out at any time.") << std::endl
              << std::endl
              << footer_keys("enter " + sym(Sym::Prompt) + " start the exam") << std::flush;
    read_line();
    std::cout << std::endl;
}

// ==> Exam status: header box, level list, paths
void ModernDisplay::showStatus(const ExamView &v)
{
    time_t now = time(NULL);
    int grade = v.level_per_ex_save * v.level;

    std::vector<std::string> head;
    head.push_back("Level " + bold(std::to_string(v.level)) + dim("/" + std::to_string(v.level_max)) + "   " + ui::progress(v.level, v.level_max, 28) + "  " + paint(Color::Bold, paint(Color::Success, std::to_string(grade))) + dim("/100"));
    std::string next = paint(Color::Success, "now");
    if (v.cooldown_until > now)
        next = paint(Color::Warning, "in " + wait_time(v.cooldown_until - now));
    head.push_back(ui::pad(paint(Color::Warning, sym(Sym::Clock)) + "  " + bold(clock_long(v.end_time - now)) + " left", 24) + paint(Color::Current, sym(Sym::Cooldown)) + " next grade: " + next);
    ui::BoxOpts opts;
    opts.title = bold(exam_title(v.student, v.exam_number));
    opts.label = v.real_mode ? paint(Color::Mode, "REAL") : paint(Color::Warning, "TEST");
    std::cout << std::endl << ui::box(head, opts) << std::endl;

    // passed levels; an exam saved by an older version doesn't know them
    for (int l = 0; l < v.level; l++)
    {
        std::string name = uni("—", "-"), tries;
        for (size_t i = 0; i < v.passed.size(); i++)
            if (v.passed[i].level == l)
            {
                name = v.passed[i].name;
                tries = tries_text(v.passed[i].tries + 1);
            }
        std::cout << "   " << paint(Color::Success, sym(Sym::Check)) << "  lvl " << ui::pad(std::to_string(l), 3) << " " << (tries.empty() ? name : ui::pad(name, 18) + " " + dim(tries)) << std::endl;
    }
    std::string failed = v.tries ? " " + dot() + " " + std::to_string(v.tries) + " failed" : "";
    std::cout << "   " << paint(Color::Current, sym(Sym::Current)) << "  lvl " << ui::pad(std::to_string(v.level), 3) << " " << ui::pad(bold(v.ex_name), 18) << " " << paint(Color::Current, "current" + failed) << std::endl;
    for (int l = v.level + 1; l < v.level_max; l++)
        std::cout << "   " << dim(sym(Sym::Pending) + "  lvl " + ui::pad(std::to_string(l), 3) + " ???") << std::endl;

    std::cout << std::endl
              << "   " << dim("subject  ") << " subjects/subject.en.txt" << std::endl
              << "   " << dim("your code") << " rendu/" << v.ex_name << "/" << std::endl
              << std::endl
              << "   " << dim("grademe to be graded " + dot() + " help for commands") << std::endl
              << std::endl;
}

void ModernDisplay::showHelp(bool cheats)
{
    std::vector<std::vector<std::string> > rows;
    rows.push_back({paint(Color::Success, "grademe"), "grade your exercise"});
    rows.push_back({paint(Color::Success, "status"), "show the exam status"});
    rows.push_back({paint(Color::Success, "settings"), "open the settings (display style applies right away)"});
    rows.push_back({paint(Color::Success, "help"), "show this help"});
    rows.push_back({paint(Color::Success, "finish"), "leave the exam (progress is lost)"});
    size_t base = rows.size();
    if (cheats)
    {
        rows.push_back({paint(Color::Warning, "gradenow"), "grade right away, with no cooldown"});
        rows.push_back({paint(Color::Warning, "remove_grade_time"), "turn off the cooldown for this exam"});
        rows.push_back({paint(Color::Warning, "force_success"), "mark the current exercise as passed"});
        rows.push_back({paint(Color::Warning, "new_ex"), "draw a new exercise at the same level"});
    }
    // one columns() call so both blocks line up
    std::vector<std::string> lines = ui::columns(rows, 3);
    std::cout << std::endl << "  " << bold("Commands") << std::endl;
    for (size_t i = 0; i < lines.size(); i++)
    {
        if (i == base)
            std::cout << std::endl << "  " << bold("Cheat commands") << dim("  (each one counts in the end screen)") << std::endl;
        std::cout << "    " << lines[i] << std::endl;
    }
    std::cout << std::endl;
}

// readline needs the non-printing parts of the prompt between \001 and \002
static std::string rl(const std::string &escape)
{
    return (escape.empty() ? "" : "\001" + escape + "\002");
}

// ==> Prompt with the time left, recomputed each time it is shown
Action ModernDisplay::readCommand(time_t end_time)
{
    long left = end_time - time(NULL);
    Color c = left < 600 ? Color::Failure : left < 1800 ? Color::Warning : Color::Text;
    std::string end = rl(ui::reset());
    std::string prompt = rl(ui::esc(Color::Warning)) + "examshell" + end + " " + rl(ui::esc(Color::Dim)) + "[" + end + rl(ui::esc(c)) + clock_short(left) + end + rl(ui::esc(Color::Dim)) + "]" + end + " " + rl(ui::esc(Color::Success)) + sym(Sym::Prompt) + end + " ";
    return (read_action(prompt));
}

bool ModernDisplay::confirmFinish(void)
{
    std::cout << "  " << paint(Color::Failure, "Leave the exam?") << " All your progress will be lost." << std::endl
              << "  Type " << bold("yes") << " to confirm: " << std::flush;
    return (trim(read_line()) == "yes");
}

void ModernDisplay::waitEnter(void)
{
    std::cout << footer_keys("enter " + sym(Sym::Prompt) + " " + next_step_) << std::flush;
    next_step_ = "continue";
    read_line();
}

bool ModernDisplay::confirmGrade(const std::string &beta_author)
{
    if (!beta_author.empty())
    {
        std::cout << "  " << paint(Color::Warning, uni("⚠", "!") + " beta exercise") << " by " << paint(Color::Warning, beta_author)
                  << dim(": if you find a bug, fix it in the exercise folder (see the README).") << std::endl;
    }
    std::cout << "  Submit now? A failed try means the same exercise for fewer points. " << dim("[y/N] ") << std::flush;
    std::string answer = trim(read_line());
    return (answer == "y" || answer == "Y");
}

void ModernDisplay::cooldownError(time_t seconds)
{
    std::cout << "  " << paint(Color::Current, sym(Sym::Cooldown)) << " next grade in " << paint(Color::Warning, wait_time(seconds))
              << dim(": take the time to test more.") << std::endl;
}

void ModernDisplay::gradingStarted(void)
{
}

// ==> First line the shared runner writes to .system/grading/progress, "" if none
static std::string grading_progress(void)
{
    std::ifstream file(".system/grading/progress");
    std::string line;
    std::getline(file, line);
    return (line);
}

void ModernDisplay::testerStarted(const std::string &name)
{
    std::cout << std::flush;
    spinner_.start("Grading " + bold(name) + sym(Sym::Ellipsis), grading_progress);
}

void ModernDisplay::testerFinished(void)
{
    spinner_.stop();
}

// ==> Up to `max` lines of block, the first one after the label, the others lined up under it
static void add_block(std::vector<std::string> &lines, const std::string &label, const std::vector<std::string> &block, Color c, size_t max)
{
    std::string head = dim(ui::pad(label, 10));
    std::string indent(10, ' ');
    if (block.empty() || (block.size() == 1 && block[0].empty()))
    {
        lines.push_back(head + dim("(no output)"));
        return;
    }
    for (size_t i = 0; i < block.size() && i < max; i++)
        lines.push_back((i == 0 ? head : indent) + paint(c, untab(block[i])));
    if (block.size() > max)
        lines.push_back(indent + dim(sym(Sym::Ellipsis) + " " + std::to_string(block.size() - max) + " more lines in the trace"));
}

void ModernDisplay::showResult(const GradeResult &r)
{
    std::vector<std::string> lines;
    ui::BoxOpts opts;
    if (r.passed)
    {
        next_step_ = r.level >= r.level_max ? "see your results" : "next exercise";
        std::string tests = r.tests ? std::to_string(r.tests) + "/" + std::to_string(r.tests) + " tests   " : "";
        lines.push_back(bold(r.ex_name) + "   " + tests + dim(ordinal(r.tries) + " try"));
        lines.push_back("Level " + bold(std::to_string(r.level)) + dim("/" + std::to_string(r.level_max)) + "   " + ui::progress(r.level, r.level_max, 28) + "  " + paint(Color::Bold, paint(Color::Success, std::to_string(r.grade))));
        opts.title = paint(Color::Success, sym(Sym::Check) + " PASSED");
        opts.footer = paint(Color::Success, "+" + std::to_string(r.gained) + " pts");
        opts.color = Color::Success;
        std::cout << std::endl << ui::box(lines, opts) << std::endl;
        return;
    }

    opts.title = paint(Color::Failure, sym(Sym::Cross) + " FAILED");
    opts.color = Color::Failure;
    if (r.structured && r.case_no > 0)
    {
        opts.label = "test " + std::to_string(r.case_no) + " of " + std::to_string(r.case_total);
        lines.push_back(dim("$") + " " + r.command);
        if (!r.stdin_label.empty())
            lines.push_back(dim("stdin") + " " + r.stdin_label);
        lines.push_back("");
        if (r.reason != "diff")
            lines.push_back(dim(ui::pad("reason", 10)) + paint(Color::Warning, r.reason));
        add_block(lines, "expected", r.expected, Color::Success, 6);
        add_block(lines, "got", r.got, Color::Failure, 6);
    }
    else if (r.structured)
    {
        // compile error, forbidden function: before any test ran
        lines.push_back(paint(Color::Warning, r.reason));
        if (!r.got.empty())
            lines.push_back("");
        for (size_t i = 0; i < r.got.size() && i < 8; i++)
            lines.push_back(dim(untab(r.got[i])));
        if (r.got.size() > 8)
            lines.push_back(dim(sym(Sym::Ellipsis) + " " + std::to_string(r.got.size() - 8) + " more lines in the trace"));
    }
    else if (!r.raw.empty())
    {
        for (size_t i = 0; i < r.raw.size(); i++)
            lines.push_back(untab(r.raw[i]));
    }
    else
        lines.push_back(dim("The tester left no trace."));

    std::string footer;
    if (!r.trace_rel.empty())
        footer = "trace: " + r.trace_rel;
    time_t now = time(NULL);
    if (r.retry_at > now)
        footer += (footer.empty() ? "" : " " + dot() + " ") + "retry in " + paint(Color::Warning, wait_time(r.retry_at - now));
    opts.footer = dim(footer);
    std::cout << std::endl << ui::box(lines, opts) << std::endl;
}

void ModernDisplay::showEnd(const ExamSummary &s)
{
    std::vector<std::string> lines(1, "");
    lines.push_back(dim(ui::pad("time used", 13)) + duration(s.time_used) + dim(" of " + duration(s.time_total)));
    lines.push_back(dim(ui::pad("cheats", 13)) + (s.cheat_count ? paint(Color::Failure, std::to_string(s.cheat_count)) : paint(Color::Success, "none")));
    lines.push_back("");
    for (size_t i = 0; i < s.rows.size(); i++)
    {
        const SummaryRow &row = s.rows[i];
        std::string unknown = uni("—", "-");
        lines.push_back(paint(Color::Success, sym(Sym::Check)) + " lvl " + ui::pad(std::to_string(row.level), 2) + " " + ui::pad(row.name.empty() ? unknown : row.name, 18) + " " + ui::pad(row.tries < 0 ? unknown : tries_text(row.tries), 9) + dim(ui::pad(row.seconds < 0 ? unknown : duration(row.seconds), 7, ui::Align::Right)));
    }
    lines.push_back("");
    ui::BoxOpts opts;
    Color c = s.cheat_count ? Color::Warning : Color::Success;
    opts.title = paint(Color::Bold, paint(c, exam_title(s.student, s.exam_number) + " " + dot() + " " + std::to_string(s.grade) + "/100"));
    if (s.cheat_count)
        opts.footer = dim("thanks for studying, " + s.username);
    else
        opts.footer = dim("nice work, " + s.username + " " + dot() + " " + uni("❤", "<3"));
    std::cout << std::endl << ui::box(lines, opts) << std::endl;
}

void ModernDisplay::message(Msg m)
{
    std::string cross = paint(Color::Failure, sym(Sym::Cross));
    switch (m)
    {
    case Msg::CheatsDisabled:
        std::cout << "  " << cross << " cheat commands are off: turn them on in " << paint(Color::Success, "settings") << std::endl;
        break;
    case Msg::FinishAborted:
        std::cout << dim("  still in the exam") << std::endl;
        break;
    case Msg::GradeTimeRemoved:
        std::cout << "  " << paint(Color::Current, sym(Sym::Cooldown)) << " grading cooldown removed for this exam" << std::endl;
        break;
    case Msg::UnknownCommand:
        std::cout << "  unknown command, type " << paint(Color::Success, "help") << std::endl;
        break;
    case Msg::OnlyOneExercise:
        std::cout << "  " << cross << " this level has only one exercise" << std::endl;
        break;
    case Msg::NewExercise:
        std::cout << "  " << paint(Color::Current, sym(Sym::Current)) << " new exercise drawn" << std::endl;
        break;
    case Msg::MaxLevel:
        std::cout << "  you reached the last level of this exam" << std::endl;
        break;
    case Msg::NoExercisePath:
        std::cout << "  " << cross << " cannot load the exercise: its folder is missing" << std::endl;
        break;
    case Msg::NoTester:
        std::cout << "  " << cross << " no tester for this exercise yet; " << paint(Color::Warning, "force_success") << " skips it" << std::endl;
        break;
    case Msg::GradeRequestSent:
        break; // the spinner shows it
    case Msg::GradeAborted:
        std::cout << dim("  grading cancelled") << std::endl;
        break;
    case Msg::BackupOpenError:
        std::cout << "  " << cross << " cannot open the saved exam" << std::endl;
        break;
    case Msg::RestoringToken:
        std::cout << dim("  restoring the saved exam" + sym(Sym::Ellipsis)) << std::endl;
        break;
    case Msg::TokenDeleted:
        std::cout << dim("  saved exam erased") << std::endl;
        break;
    case Msg::SettingsSaved:
        std::cout << "  " << paint(Color::Success, sym(Sym::Check)) << " settings saved" << std::endl;
        break;
    }
}

void ModernDisplay::error(const std::string &text)
{
    std::cout << "  " << paint(Color::Failure, sym(Sym::Cross)) << " " << text << std::endl;
}
