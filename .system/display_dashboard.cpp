#include "exam.hpp"
#include "term.hpp"
#include "ui.hpp"
#include "display_parts.hpp"
#include <algorithm>
#include <chrono>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

using ui::Color;
using ui::Sym;
using ui::paint;
using ui::sym;
using namespace modern;

// ==> Dashboard: the whole program on one full screen.
// Rules: the first Dashboard screen goes full screen (term_screen_enter) and raw mode stays on
// until the program quits (dashboard_exit, or sigc/sigd on a signal). The screen is only drawn
// from the key loop (waitKey), the login animation and the grading spinner, under draw_mutex_.
// Only Practice (Modern's screens for now) and another display style leave it before then.

static const int MIN_WIDTH = 80;
static const int MIN_HEIGHT = 24;
static const int LEFT_WIDTH = 31; // left column, its right border shared with the right panels

enum class Ask
{
    None,
    YesNo, // one key
    Line,  // text and enter
    Enter  // enter only
};

// ==> A screen outside the exam panels: menus, exam start, end summary, Settings. Drawn in the
// page frame (title panel, content panel, key bar), or as a box over the exam (overlay).
struct Page
{
    bool on = false;
    bool overlay = false;
    bool end = false; // the end summary: waitEnter waits on it
    std::string where; // title panel: offline simulator · where
    std::string title; // content panel
    std::string label;
    Color color = Color::Box;
    std::vector<std::string> lines;  // above the list
    std::vector<ui::ListRow> rows;   // list, none when empty
    int cursor = 0;
    std::vector<std::string> after;  // below the list
    std::string keys;                // key bar
};

// ==> What the dashboard shows. The exam creates a new display at every level and after
// Settings, so this outlives the DashboardDisplay objects.
struct DashState
{
    ui::Screen screen;
    bool started = false;  // went full screen once: the size check is done
    bool fallback = false; // no terminal or too small at the start: Modern for the whole run

    Page page;
    bool in_exam = false; // the exam panels are under the page (Settings is an overlay then)
    int start_cursor = 1;
    int part_cursor = 0;
    int settings_cursor = 0;

    bool ended = false; // an exam ended and no other started: its summary is printed at quit
    ExamSummary summary;

    ExamView view;
    std::string key; // exam and exercise the panels below belong to

    std::vector<std::string> subject; // lines of subjects/subject.en.txt
    int subject_scroll = 0;
    bool subject_focus = false;

    bool has_result = false;
    GradeResult result;
    bool grading = false;
    std::string grading_name;
    int spin_frame = 0;
    bool skip_wait = false; // waitEnter after a failed grade: the card stays in the panel

    std::string msg; // bottom bar message, coloured
    time_t msg_until = 0;

    Ask ask = Ask::None;
    std::string question;
    std::string input;

    bool help = false;
    bool help_cheats = false;

    bool trace = false;
    std::vector<std::string> trace_lines;
    int trace_scroll = 0;
};
static DashState g;

// ==> Keep the state of the exercise on screen; a new exercise starts with fresh panels
static void set_view(const ExamView &v)
{
    std::string key = std::to_string(v.end_time) + " " + std::to_string(v.level) + " " + v.ex_name;
    g.view = v;
    g.page = Page();
    g.in_exam = true;
    g.ended = false;
    if (key == g.key)
        return;
    g.key = key;
    g.subject.clear();
    std::ifstream file("subjects/subject.en.txt");
    std::string line;
    while (std::getline(file, line))
        g.subject.push_back(untab(line));
    if (!file.is_open())
        g.subject.push_back(dim("subjects/subject.en.txt is missing"));
    g.subject_scroll = 0;
    g.subject_focus = false;
    g.has_result = false;
    g.result = GradeResult();
    g.skip_wait = false;
}

static std::string key_hint(const std::string &key, const std::string &what)
{
    return (paint(Color::Bold, paint(Color::Current, key)) + " " + dim(what));
}

static std::string spinner_frame(int f)
{
    static const char *unicode[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
    static const char *ascii[] = {"|", "/", "-", "\\"};
    return (term_unicode() ? unicode[f % 10] : ascii[f % 4]);
}

// ==> Fixed messages as one bottom bar line; "" for the ones the dashboard has no use for
static std::string msg_text(Msg m)
{
    std::string cross = paint(Color::Failure, sym(Sym::Cross)) + " ";
    switch (m)
    {
    case Msg::CheatsDisabled:
        return (cross + "cheat commands are off: turn them on in settings (,)");
    case Msg::FinishAborted:
        return (dim("still in the exam"));
    case Msg::GradeTimeRemoved:
        return (paint(Color::Current, sym(Sym::Cooldown)) + " grading cooldown removed for this exam");
    case Msg::UnknownCommand:
        return ("unknown command, press " + paint(Color::Current, "?") + " for help");
    case Msg::OnlyOneExercise:
    case Msg::PracticeOnlyOne:
        return (cross + "this level has only one exercise");
    case Msg::NewExercise:
        return (paint(Color::Current, sym(Sym::Current)) + " new exercise drawn");
    case Msg::MaxLevel:
        return ("you reached the last level of this exam");
    case Msg::NoExercisePath:
        return (cross + "cannot load the exercise: its folder is missing");
    case Msg::NoTester:
        return (cross + "no tester for this exercise yet; " + paint(Color::Warning, "force_success") + " skips it");
    case Msg::GradeAborted:
        return (dim("grading cancelled"));
    case Msg::SettingsSaved:
        return (paint(Color::Success, sym(Sym::Check)) + " settings saved");
    case Msg::BackupOpenError:
        return (cross + "cannot open the saved exam");
    case Msg::RestoringToken:
        return (dim("restoring the saved exam" + sym(Sym::Ellipsis)));
    case Msg::TokenDeleted:
        return (dim("saved exam erased"));
    case Msg::PracticeNoExercises:
        return (cross + "no exercise found in .subjects/");
    default:
        return ("");
    }
}

DashboardDisplay::DashboardDisplay(void) : screen_(g.screen), spinning_(false)
{
}

DashboardDisplay::~DashboardDisplay(void)
{
    // the full screen stays on: the next display (a new level, new settings) carries on drawing
    spinning_ = false;
    if (spinner_thread_.joinable())
        spinner_thread_.join();
}

// ==> Normal quit (`q` in the main menu, `exit` in the exam): back to the normal screen, and the
// summary of an exam that just ended stays in the scrollback. After a signal, sigc/sigd have
// already left the full screen, so nothing is printed.
static void dashboard_exit(void)
{
    if (!term_screen_active())
        return;
    term_screen_leave();
    if (!g.ended)
        return;
    ui::BoxOpts opts;
    end_card(g.summary, opts);
    opts.color = Color::Box;
    std::cout << std::endl << ui::box(end_lines(g.summary), opts) << std::flush;
}

// ==> Full screen, or false when the program falls back to Modern: no terminal or smaller than
// 80x24 at the first Dashboard screen, or the full screen failed. Later calls (back from
// Practice) enter it again whatever the size: a small window gets the "bigger" message.
bool DashboardDisplay::enter(void)
{
    if (term_screen_active())
        return (true);
    if (g.fallback)
        return (false);
    bool tty = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
    int w = term_width(), h = term_height();
    bool small = !g.started && (w < MIN_WIDTH || h < MIN_HEIGHT);
    if (!tty || small || !term_screen_enter())
    {
        g.fallback = true;
        std::string why = !tty ? "needs a terminal" : small ? "needs 80x24 (this is " + std::to_string(w) + "x" + std::to_string(h) + ")" : "cannot go full screen";
        std::cout << "  " << paint(Color::Warning, "!") << " Dashboard " << why << dim(": Modern style instead") << std::endl;
        if (tty)
            usleep(1500000); // Modern's next screen clears it
        return (false);
    }
    if (!g.started)
    {
        g.started = true;
        atexit(dashboard_exit); // runs before main()'s atexit(term_screen_leave)
    }
    screen_.invalidate();
    return (true);
}

// ==> Fresh page in the frame; the exam panels are no longer under it
static void new_page(const std::string &where, const std::string &title, const std::string &keys)
{
    g.page = Page();
    g.page.on = true;
    g.page.where = where;
    g.page.title = title;
    g.page.keys = keys;
    g.in_exam = false;
    g.help = false;
    g.trace = false;
}

static std::string arrows(void)
{
    return (uni("↑↓", "up/down"));
}

static std::string list_keys(const std::string &back)
{
    return (key_hint(arrows(), "move") + "   " + key_hint("enter", "select") + (back.empty() ? "" : "   " + back));
}

// ==> Lines of the page, list included
static std::vector<std::string> page_lines(void)
{
    std::vector<std::string> lines = g.page.lines;
    if (!g.page.rows.empty())
    {
        if (!lines.empty())
            lines.push_back("");
        std::vector<std::string> list = ui::list_lines(g.page.rows, g.page.cursor);
        lines.insert(lines.end(), list.begin(), list.end());
    }
    if (!g.page.after.empty())
    {
        lines.push_back("");
        lines.insert(lines.end(), g.page.after.begin(), g.page.after.end());
    }
    return (lines);
}

// ==> The page as a block, centred across the rect and a third of the way down
void DashboardDisplay::drawPage(const ui::Rect &r)
{
    std::vector<std::string> lines = page_lines();
    int w = 0;
    for (size_t i = 0; i < lines.size(); i++)
        w = std::max(w, ui::display_width(lines[i]));
    int x = r.x + std::max((r.w - w) / 2, 0);
    int y = r.y + std::max((r.h - (int)lines.size()) / 3, 0);
    ui::text(screen_, {x, y, r.x + r.w - x, r.y + r.h - y}, lines);
}

// ==> Modern's pick() on the page: the options become its list
std::string DashboardDisplay::listPick(const std::vector<Option> &options, int &cursor, const std::string &back)
{
    int hint_width = 18;
    for (size_t i = 0; i < options.size(); i++)
        hint_width = std::max(hint_width, ui::display_width(options[i].hint));
    g.page.rows.clear();
    for (size_t i = 0; i < options.size(); i++)
    {
        const Option &o = options[i];
        ui::ListRow row;
        row.key = o.key;
        row.normal = dim(o.key) + "   " + ui::pad(o.label, 18) + "  " + dim(o.hint);
        row.plain = o.key + "   " + ui::pad(o.label, 18) + "  " + ui::pad(o.hint, hint_width);
        g.page.rows.push_back(row);
    }
    if (cursor < 0 || cursor >= (int)options.size())
        cursor = 0;
    while (1)
    {
        g.page.cursor = cursor;
        draw();
        char c = 0;
        Key k = waitKey(c);
        int i = ui::list_key(g.page.rows, cursor, k, c, !back.empty());
        if (i == ui::LIST_BACK)
            return (back);
        if (i >= 0)
            return (options[i].key);
    }
}

// ==> Enter goes on
void DashboardDisplay::waitPage(void)
{
    while (1)
    {
        draw();
        char c = 0;
        if (waitKey(c) == Key::Enter)
            return;
    }
}

void DashboardDisplay::setMessage(const std::string &text, bool error)
{
    g.msg = error ? paint(Color::Failure, sym(Sym::Cross)) + " " + text : text;
    g.msg_until = time(NULL) + 4;
}

// ==> One frame: the dashboard, the trace view or the help over it
void DashboardDisplay::draw(void)
{
    std::lock_guard<std::mutex> lock(draw_mutex_);
    drawFrame();
    screen_.flush();
}

void DashboardDisplay::drawGrade(const ui::Rect &r)
{
    ui::Rect in = {r.x + 2, r.y + 1, r.w - 4, r.h - 2};
    if (g.grading)
    {
        std::vector<std::string> lines;
        lines.push_back(paint(Color::Current, spinner_frame(g.spin_frame)) + " Grading " + bold(g.grading_name) + sym(Sym::Ellipsis));
        std::string step = grading_progress();
        if (!step.empty())
            lines.push_back("  " + dim(step));
        ui::panel(screen_, r, "last grade", "", Color::Box);
        ui::text(screen_, in, lines);
        return;
    }
    if (!g.has_result)
    {
        ui::panel(screen_, r, "last grade", "", Color::Box);
        ui::text(screen_, in, std::vector<std::string>(1, dim("no grade yet " + dot() + " press ") + paint(Color::Current, "g")));
        return;
    }
    // the biggest card that fits: fewer expected/got lines when the panel is short
    ui::BoxOpts opts;
    std::vector<std::string> lines;
    for (size_t max_block = 6; max_block > 0; max_block--)
    {
        opts = ui::BoxOpts();
        lines = result_lines(g.result, opts, max_block);
        if ((int)lines.size() + (opts.footer.empty() ? 0 : 1) <= in.h)
            break;
    }
    if (ui::display_width(opts.footer) > 0)
    {
        while ((int)lines.size() < in.h - 1)
            lines.push_back("");
        lines.push_back(opts.footer);
    }
    std::string label = opts.title + (opts.label.empty() ? "" : dim(" " + dot() + " ") + opts.label);
    ui::panel(screen_, r, "last grade", label, Color::Box);
    ui::text(screen_, in, lines);
}

// ==> Bottom bar: prompt, message or key hints on the left, cooldown on the right
static std::string bottom_left(void)
{
    if (g.ask != Ask::None)
    {
        std::string caret = paint(Color::Current, term_unicode() ? "█" : "_");
        if (g.ask == Ask::Line)
            return (paint(Color::Warning, g.question) + " " + g.input + caret);
        return (paint(Color::Warning, g.question));
    }
    if (!g.msg.empty() && time(NULL) < g.msg_until)
        return (g.msg);
    if (g.page.on)
        return (g.page.keys);
    if (g.trace)
        return (key_hint(term_unicode() ? "↑↓" : "up/down", "scroll") + "   " + key_hint("PgUp PgDn", "page") + "   " + key_hint("esc", "back"));
    if (g.help)
        return (dim("any key closes the help"));
    if (g.subject_focus)
        return (key_hint(term_unicode() ? "↑↓" : "up/down", "scroll") + "   " + key_hint("PgUp PgDn", "page") + "   " + key_hint("esc", "back"));
    return (key_hint("g", "grade") + "   " + key_hint("s", "subject") + "   " + key_hint("t", "trace") + "   " + key_hint(",", "settings") + "   " + key_hint("?", "help") + "   " + key_hint("q", "quit"));
}

static std::string bottom_right(const ExamView &v)
{
    time_t now = time(NULL);
    std::string when = paint(Color::Success, "now");
    if (v.cooldown_until > now)
        when = paint(Color::Warning, "in " + wait_time(v.cooldown_until - now));
    return (paint(Color::Current, sym(Sym::Cooldown)) + " grade: " + when);
}

static std::vector<std::string> help_lines(bool cheats)
{
    std::vector<std::vector<std::string> > rows;
    std::string arrows = term_unicode() ? "↑↓" : "up/down";
    rows.push_back({paint(Color::Current, "g"), "grade your exercise"});
    rows.push_back({paint(Color::Current, "s") + dim(" / ") + paint(Color::Current, "tab"), "scroll the subject (" + arrows + " PgUp PgDn j k, esc)"});
    rows.push_back({paint(Color::Current, "t"), "full trace of the last failure"});
    rows.push_back({paint(Color::Current, ","), "settings (the style applies right away)"});
    rows.push_back({paint(Color::Current, "?"), "this help"});
    rows.push_back({paint(Color::Current, "q"), "leave the exam, back to the main menu"});
    rows.push_back({paint(Color::Current, ":"), "type a command: grademe, status, finish, exit..."});
    size_t base = rows.size();
    if (cheats)
    {
        rows.push_back({paint(Color::Warning, "G"), "gradenow: grade with no cooldown"});
        rows.push_back({paint(Color::Warning, "r"), "remove_grade_time: no cooldown for this exam"});
        rows.push_back({paint(Color::Warning, "f"), "force_success: pass the current exercise"});
        rows.push_back({paint(Color::Warning, "n"), "new_ex: new exercise at the same level"});
    }
    std::vector<std::string> cols = ui::columns(rows, 3);
    std::vector<std::string> lines;
    for (size_t i = 0; i < cols.size(); i++)
    {
        if (i == base)
        {
            lines.push_back("");
            lines.push_back(bold("Cheat keys") + dim("  (each one counts in the end screen)"));
        }
        lines.push_back(cols[i]);
    }
    return (lines);
}

void DashboardDisplay::drawFrame(void)
{
    int W = term_width(), H = term_height();
    if (screen_.width() != W || screen_.height() != H)
        screen_.resize(W, H);
    screen_.clear();
    const ExamView &v = g.view;
    time_t now = time(NULL);

    if (W < MIN_WIDTH || H < MIN_HEIGHT)
    {
        std::string a = paint(Color::Warning, "make the window bigger") + dim(" (min 80x24)");
        std::string b = dim("now " + std::to_string(W) + "x" + std::to_string(H));
        screen_.put(std::max((W - ui::display_width(a)) / 2, 0), H / 2 - 1, a);
        screen_.put(std::max((W - ui::display_width(b)) / 2, 0), H / 2, b);
        return;
    }

    if (g.page.on && !g.page.overlay)
    {
        char clock_text[16];
        strftime(clock_text, sizeof(clock_text), "%H:%M", localtime(&now));
        ui::FrameOpts o;
        o.title = bold("42 EXAM");
        o.label = dim("v" EXAMSHELL_VERSION);
        o.line = dim("offline simulator " + dot() + " ") + g.page.where;
        o.line_right = dim(clock_text);
        o.panel_title = g.page.title;
        o.panel_label = g.page.label;
        o.color = g.page.color;
        drawPage(ui::frame(screen_, o));
        ui::key_bar(screen_, bottom_left(), "");
        return;
    }

    ui::Rect body = {0, 0, W, H - 1};
    if (g.trace)
    {
        ui::Rect in = {2, 1, W - 4, H - 3};
        int n = g.trace_lines.size();
        g.trace_scroll = std::max(std::min(g.trace_scroll, n - in.h), 0);
        std::string where = n > in.h ? std::to_string(g.trace_scroll + 1) + "-" + std::to_string(std::min(g.trace_scroll + in.h, n)) + " of " + std::to_string(n) : "";
        ui::panel(screen_, body, bold("trace") + dim(" " + dot() + " " + g.result.trace_rel), dim(where), Color::Box);
        ui::text(screen_, in, g.trace_lines, g.trace_scroll);
    }
    else
    {
        ui::Rect left, right, head, levels, subject, grade;
        ui::split_h(body, LEFT_WIDTH, left, right);
        right.x -= 1; // shared border column
        right.w += 1;
        ui::split_v(left, 4, head, levels);
        levels.y -= 1; // shared border row
        levels.h += 1;
        int grade_h = std::max(9, body.h * 2 / 5);
        ui::split_v(right, -grade_h, subject, grade);
        subject.h += 1;

        // header: exam, mode, clock, progress
        long left_s = v.end_time - now;
        Color clock = left_s < 600 ? Color::Failure : left_s < 1800 ? Color::Warning : Color::Text;
        std::string mode = v.real_mode ? paint(Color::Mode, "REAL") : paint(Color::Warning, "TEST");
        int in_w = head.w - 4;
        std::string clock_text = paint(clock, sym(Sym::Clock)) + " " + paint(Color::Bold, paint(clock, clock_long(left_s)));
        std::string score = std::to_string(v.level_per_ex_save * v.level);
        std::string points = paint(Color::Bold, paint(Color::Success, score)) + dim("/100");
        int bar_w = std::max(in_w - 9, 4);
        std::vector<std::string> head_lines;
        head_lines.push_back(ui::pad(clock_text, in_w - 4) + mode);
        head_lines.push_back(ui::progress(v.level, v.level_max, bar_w) + ui::pad(points, in_w - bar_w, ui::Align::Right));
        ui::panel(screen_, head, bold(exam_title(v.student, v.exam_number)), "", Color::Box);
        ui::text(screen_, {head.x + 2, head.y + 1, in_w, 2}, head_lines);

        // levels: passed, current, the ones to come
        std::vector<std::string> level_lines;
        for (int l = 0; l < v.level_max; l++)
        {
            std::string mark, name, right_text;
            if (l < v.level)
            {
                mark = paint(Color::Success, sym(Sym::Check));
                name = uni("—", "-");
                for (size_t i = 0; i < v.passed.size(); i++)
                    if (v.passed[i].level == l)
                    {
                        name = v.passed[i].name;
                        right_text = dim(tries_text(v.passed[i].tries + 1));
                    }
            }
            else if (l == v.level)
            {
                mark = paint(Color::Current, sym(Sym::Current));
                name = bold(v.ex_name);
                if (v.tries)
                    right_text = paint(Color::Failure, std::to_string(v.tries) + " failed");
            }
            else
            {
                mark = dim(sym(Sym::Pending));
                name = dim("???");
            }
            std::string num = l == v.level ? paint(Color::Current, std::to_string(l)) : dim(std::to_string(l));
            int name_w = in_w - 4 - (right_text.empty() ? 0 : ui::display_width(right_text) + 1);
            level_lines.push_back(mark + " " + num + " " + ui::pad(name, std::max(name_w, 1)) + (right_text.empty() ? "" : " " + right_text));
        }
        ui::panel(screen_, levels, "levels", "", Color::Box);
        ui::text(screen_, {levels.x + 2, levels.y + 1, in_w, levels.h - 2}, level_lines);

        // subject, word-wrapped and scrollable
        ui::Rect sub_in = {subject.x + 2, subject.y + 1, subject.w - 4, subject.h - 2};
        std::vector<std::string> wrapped;
        for (size_t i = 0; i < g.subject.size(); i++)
        {
            std::vector<std::string> part = ui::wrap(g.subject[i], sub_in.w);
            wrapped.insert(wrapped.end(), part.begin(), part.end());
        }
        int n = wrapped.size();
        g.subject_scroll = std::max(std::min(g.subject_scroll, n - sub_in.h), 0);
        std::string where = n > sub_in.h ? std::to_string(g.subject_scroll + 1) + "-" + std::to_string(std::min(g.subject_scroll + sub_in.h, n)) + "/" + std::to_string(n) : "";
        Color sub_color = g.subject_focus ? Color::Current : Color::Box;
        std::string sub_title = bold(v.ex_name) + dim(" " + dot() + " subject");
        ui::panel(screen_, subject, sub_title, dim(where), sub_color);
        ui::text(screen_, sub_in, wrapped, g.subject_scroll);

        drawGrade(grade);

        // joints where the panels meet
        std::string jc = ui::esc(Color::Box), end = ui::reset();
        screen_.put(right.x, 0, jc + ui::border(ui::Border::TeeDown) + end);
        screen_.put(right.x, body.h - 1, jc + ui::border(ui::Border::TeeUp) + end);
        screen_.put(0, levels.y, jc + ui::border(ui::Border::TeeRight) + end);
        screen_.put(right.x, levels.y, jc + ui::border(ui::Border::TeeLeft) + end);
        screen_.put(right.x, grade.y, jc + ui::border(ui::Border::TeeRight) + end);
        screen_.put(W - 1, grade.y, jc + ui::border(ui::Border::TeeLeft) + end);
        if (g.subject_focus) // the focused panel's edges stay coloured
        {
            std::string fc = ui::esc(Color::Current);
            screen_.put(right.x, 0, fc + ui::border(ui::Border::TeeDown) + end);
            screen_.put(right.x, grade.y, fc + ui::border(ui::Border::TeeRight) + end);
            screen_.put(W - 1, grade.y, fc + ui::border(ui::Border::TeeLeft) + end);
            screen_.put(right.x, levels.y, fc + ui::border(ui::Border::TeeLeft) + end);
        }

        if (g.help)
        {
            std::vector<std::string> lines = help_lines(g.help_cheats);
            int w = 4, h = lines.size() + 4;
            for (size_t i = 0; i < lines.size(); i++)
                w = std::max(w, ui::display_width(lines[i]) + 6);
            w = std::min(w, W - 4);
            h = std::min(h, body.h - 2);
            ui::Rect box = {(W - w) / 2, std::max((body.h - h) / 2, 1), w, h};
            screen_.fill(box);
            ui::panel(screen_, box, bold("Keys"), "", Color::Current);
            ui::text(screen_, {box.x + 3, box.y + 2, box.w - 6, box.h - 4}, lines);
        }

        if (g.page.on) // Settings over the exam
        {
            std::vector<std::string> lines = page_lines();
            int w = 4, h = lines.size() + 4;
            for (size_t i = 0; i < lines.size(); i++)
                w = std::max(w, ui::display_width(lines[i]) + 6);
            w = std::min(w, W - 4);
            h = std::min(h, body.h - 2);
            ui::Rect box = {(W - w) / 2, std::max((body.h - h) / 2, 1), w, h};
            screen_.fill(box);
            ui::panel(screen_, box, g.page.title, g.page.label, Color::Current);
            drawPage({box.x + 3, box.y + 2, box.w - 6, box.h - 4});
        }
    }

    ui::key_bar(screen_, bottom_left(), bottom_right(v));
}

// ==> Next key. Meanwhile the clock ticks (redrawn at each new second, only the changed cells
// are sent) and a resize redraws everything; drawing never happens in the signal handler.
Key DashboardDisplay::waitKey(char &c)
{
    while (1)
    {
        if (term_resized())
            draw();
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int ms = 1000 - tv.tv_usec / 1000 + 5; // wake up just after the next second
        Key k = term_read_key_timeout(c, ms);
        if (k == Key::None)
        {
            draw();
            continue;
        }
        if (k == Key::Eof || (k == Key::Other && c == 4))
        {
            term_screen_leave();
            sigd();
        }
        g.msg_until = 0; // a message stays until the next key
        return (k);
    }
}

// ==> Inline prompt in the bottom bar; the key (YesNo), the text (Line), "" when cancelled
std::string DashboardDisplay::ask(const std::string &question, bool line)
{
    g.ask = line ? Ask::Line : Ask::YesNo;
    g.question = question;
    g.input.clear();
    std::string answer;
    while (1)
    {
        draw();
        char c = 0;
        Key k = waitKey(c);
        if (k == Key::Esc)
            break;
        if (!line && k == Key::Char)
        {
            answer = std::string(1, c);
            break;
        }
        if (!line && k == Key::Enter)
            break;
        if (line && k == Key::Enter)
        {
            answer = g.input;
            break;
        }
        if (line && k == Key::Backspace && !g.input.empty())
            g.input.erase(g.input.size() - 1);
        else if (line && k == Key::Char)
            g.input += c;
    }
    g.ask = Ask::None;
    g.input.clear();
    draw();
    return (answer);
}

void DashboardDisplay::scrollSubject(int delta)
{
    g.subject_scroll = std::max(g.subject_scroll + delta, 0); // drawFrame clamps the end
}

// ==> Full trace of the last failure, scrollable; esc/q/t returns
void DashboardDisplay::showTrace(void)
{
    if (!g.has_result || g.result.passed || g.result.trace_rel.empty())
    {
        setMessage("no failed grade with a trace yet", true);
        return;
    }
    g.trace_lines.clear();
    std::ifstream file(g.result.trace_rel);
    std::string line;
    while (std::getline(file, line))
        g.trace_lines.push_back(untab(line));
    if (g.trace_lines.empty())
        g.trace_lines.push_back(dim("the trace is empty"));
    g.trace = true;
    g.trace_scroll = 0;
    int page = std::max(term_height() - 4, 1);
    while (1)
    {
        draw();
        char c = 0;
        Key k = waitKey(c);
        if (k == Key::Esc || (k == Key::Char && (c == 'q' || c == 't')))
            break;
        if (k == Key::Up || (k == Key::Char && c == 'k'))
            g.trace_scroll = std::max(g.trace_scroll - 1, 0);
        else if (k == Key::Down || (k == Key::Char && c == 'j'))
            g.trace_scroll++;
        else if (k == Key::PageUp)
            g.trace_scroll = std::max(g.trace_scroll - page, 0);
        else if (k == Key::PageDown || (k == Key::Char && c == ' '))
            g.trace_scroll += page;
    }
    g.trace = false;
}

// ==> DashboardDisplay methods. Each one shows the Modern screen when the dashboard is not on.

std::string DashboardDisplay::startMenu(bool invalid)
{
    if (!enter())
        return (ModernDisplay::startMenu(invalid));
    new_page("main menu", bold("Main menu"), list_keys(key_hint("q", "quit")));
    std::vector<Option> o;
    o.push_back({"1", "Piscine part", "exams 01-04"});
    o.push_back({"2", "Student part", "exams 02-06"});
    o.push_back({"3", "Practice", "pick any exercise"});
    o.push_back({"4", "Settings", ""});
    if (invalid)
        setMessage("pick 1-4, or q to quit", true);
    return (listPick(o, g.start_cursor, "q"));
}

std::string DashboardDisplay::studMenu(bool invalid)
{
    if (!enter())
        return (ModernDisplay::studMenu(invalid));
    new_page("student part", bold("Student part"), list_keys(key_hint("esc", "back")));
    std::vector<Option> o;
    for (int i = 2; i <= 6; i++)
        o.push_back({std::to_string(i), "Exam rank 0" + std::to_string(i), exam_hint(true, i)});
    if (invalid)
        setMessage("pick 2-6, or esc to go back", true);
    return (listPick(o, g.part_cursor, "0"));
}

std::string DashboardDisplay::piscineMenu(bool invalid)
{
    if (!enter())
        return (ModernDisplay::piscineMenu(invalid));
    new_page("piscine part", bold("Piscine part"), list_keys(key_hint("esc", "back")));
    std::vector<Option> o;
    for (int i = 1; i <= 4; i++)
        o.push_back({std::to_string(i), "Exam week 0" + std::to_string(i), exam_hint(false, i)});
    if (invalid)
        setMessage("pick 1-4, or esc to go back", true);
    return (listPick(o, g.part_cursor, "0"));
}

// ==> Settings as a page from the main menu, as a box over the exam during one. The exam calls
// it again after each change; the style applies when the menu is left (the exam then picks
// the display again).
std::string DashboardDisplay::settingsMenu(const SettingsView &s)
{
    if (!enter())
        return (ModernDisplay::settingsMenu(s));
    struct Row
    {
        std::string key;
        std::string label;
        std::string value; // coloured
        std::string plain; // same without colour, for the highlighted row
    };
    std::string style = uni("‹ ", "< ") + style_label(s.style) + uni(" ›", " >");
    std::vector<Row> rows;
    rows.push_back({"3", "Display style", paint(Color::Current, style), style});
    rows.push_back({"1", "Enable passed exercises", toggle(s.dse), s.dse ? uni("●", "*") + " on" : uni("○", "o") + " off"});
    rows.push_back({"2", "Cheat commands", toggle(s.dcc), s.dcc ? uni("●", "*") + " on" : uni("○", "o") + " off"});
    rows.push_back({"4", "Login animation", toggle(s.anim), s.anim ? uni("●", "*") + " on" : uni("○", "o") + " off"});
    rows.push_back({"5", "Cooldown in Practice mode", toggle(s.pcd), s.pcd ? uni("●", "*") + " on" : uni("○", "o") + " off"});
    std::string keys = key_hint(arrows(), "move") + "   " + key_hint(uni("←→", "left/right") + " space", "change") + "   " + key_hint("esc", "save");
    if (g.in_exam)
    {
        g.page = Page();
        g.page.on = true;
        g.page.overlay = true;
        g.page.title = bold("Settings");
        g.page.keys = keys;
        g.help = false;
    }
    else if (!g.page.on || g.page.where != "settings")
        new_page("settings", bold("Settings"), keys);
    g.page.rows.clear();
    for (size_t i = 0; i < rows.size(); i++)
        g.page.rows.push_back({rows[i].key, ui::pad(rows[i].label, 27) + rows[i].value, ui::pad(rows[i].label, 27) + ui::pad(rows[i].plain, 18)});
    g.page.after.assign(1, dim("the display style applies when you leave the settings"));
    int count = rows.size();
    int &cursor = g.settings_cursor;
    if (cursor < 0 || cursor >= count)
        cursor = 0;
    while (1)
    {
        g.page.cursor = cursor;
        draw();
        char c = 0;
        Key k = waitKey(c);
        const Row &row = rows[cursor];
        if (k == Key::Up || (k == Key::Char && c == 'k'))
            cursor = (cursor + count - 1) % count;
        else if (k == Key::Down || (k == Key::Char && c == 'j'))
            cursor = (cursor + 1) % count;
        else if (k == Key::Left)
            return (row.key == "3" ? "-3" : row.key);
        else if (k == Key::Right || k == Key::Enter || (k == Key::Char && c == ' '))
            return (row.key);
        else if (k == Key::Esc || (k == Key::Char && (c == 'q' || c == '0' || c == ',')))
        {
            if (g.page.overlay)
                g.page = Page(); // the "saved" message shows over the exam
            return ("0");
        }
        else if (k == Key::Char && c >= '1' && c <= '5')
            return (std::string(1, c));
    }
}

// ==> Saved exam found at startup: "1" to restore, "2" to erase
std::string DashboardDisplay::restorePrompt(const RestoreView &r)
{
    if (!enter())
        return (ModernDisplay::restorePrompt(r));
    new_page("saved exam", bold("Saved exam found"), list_keys(""));
    g.page.lines.push_back(dim("exam      ") + bold(exam_title(r.student, r.exam_number)));
    g.page.lines.push_back(dim("exercise  ") + r.ex_name);
    g.page.lines.push_back(dim("time left ") + paint(Color::Warning, clock_long(r.end_time - time(NULL))));
    std::vector<Option> o;
    o.push_back({"1", "Restore", "continue the exam"});
    o.push_back({"2", "Erase", "delete it, start over"});
    int cursor = 0;
    return (listPick(o, cursor, ""));
}

bool DashboardDisplay::confirmRegistration(bool student, int exam_number)
{
    if (!enter())
        return (ModernDisplay::confirmRegistration(student, exam_number));
    new_page(student ? "student part" : "piscine part", bold(exam_title(student, exam_number)), list_keys(key_hint("esc", "back")));
    g.page.lines.push_back(exam_hint(student, exam_number));
    g.page.lines.push_back(dim("The clock starts once you are logged in."));
    std::vector<Option> o;
    o.push_back({"y", "Start", "begin this exam"});
    o.push_back({"n", "Back", "pick another exam"});
    int cursor = 0;
    return (listPick(o, cursor, "n") == "y");
}

void DashboardDisplay::showExplanation(void)
{
    if (!enter())
        return (ModernDisplay::showExplanation());
    new_page("how it works", bold("How it works"), key_hint("enter", "continue"));
    std::string b = paint(Color::Current, sym(Sym::Current)) + " ";
    std::vector<std::string> &l = g.page.lines;
    l.push_back(b + "Work in another terminal: keep this one for the exam shell.");
    l.push_back(b + "Your subject is in " + bold("subjects/subject.en.txt") + ".");
    l.push_back(b + "Put your files in " + bold("rendu/<exercise>/") + ". No git needed here,");
    l.push_back("  but the real exam makes you push with git.");
    l.push_back(b + "Press " + paint(Color::Current, "g") + " to be graded. Pass to reach the next level;");
    l.push_back("  fail and you retry the same exercise for fewer points.");
    l.push_back(b + "Each failed try makes you wait longer before the next grade.");
    l.push_back("");
    l.push_back(paint(Color::Warning, "! Not the real 42 exam and not made by 42."));
    l.push_back(dim("  A private, offline exam simulator."));
    waitPage();
}

// ==> Login animation on the page: the login and a masked password type in, a spinner
// connects, then a check. Keys typed meanwhile are dropped.
void DashboardDisplay::connexion(const std::string &username)
{
    if (!enter())
        return (ModernDisplay::connexion(username));
    new_page("login", paint(Color::Warning, "examshell"), dim("logging in" + sym(Sym::Ellipsis)));
    g.page.label = dim("v" EXAMSHELL_VERSION);
    std::string caret = paint(Color::Current, uni("█", "_"));
    auto frame = [&](const std::string &login, const std::string &password, const std::string &status, int us) {
        g.page.lines.clear();
        g.page.lines.push_back(dim(ui::pad("login", 10)) + ui::pad(bold(login), 24));
        g.page.lines.push_back(dim(ui::pad("password", 10)) + password);
        g.page.after.assign(1, status);
        draw(); // a resize is picked up by the size check of drawFrame
        usleep(us);
    };
    std::string login;
    frame("", "", "", 500000);
    for (size_t i = 0; i < username.size(); i++)
    {
        login += username[i];
        frame(login + caret, "", "", 90000);
    }
    srand(time(NULL));
    int password_size = rand() % 10 + 4;
    std::string password;
    for (int i = 0; i < password_size; i++)
    {
        password += uni("•", "*");
        frame(login, dim(password) + caret, "", 70000);
    }
    for (int f = 0; f < 14; f++)
        frame(login, dim(password), paint(Color::Current, spinner_frame(f)) + dim(" connecting" + sym(Sym::Ellipsis)), 90000);
    frame(login, dim(password), paint(Color::Success, sym(Sym::Check) + " connected"), 400000);
    tcflush(STDIN_FILENO, TCIFLUSH);
}

void DashboardDisplay::showIntro(const std::string &username, bool student, int exam_number, int level, int time_max)
{
    if (!enter())
        return (ModernDisplay::showIntro(username, student, exam_number, level, time_max));
    new_page("exam", bold("Welcome, " + username), key_hint("enter", "start the exam"));
    std::string sep = dim("  " + dot() + "  ");
    g.page.lines.push_back(bold(exam_title(student, exam_number)) + sep + paint(Color::Mode, "REAL") + dim(" mode") + sep + "level " + std::to_string(level) + sep + duration(time_max * 60L));
    g.page.lines.push_back("");
    g.page.lines.push_back(dim("Points you earn count whatever happens. You can log out at any time."));
    g.page.lines.push_back(dim("The clock starts when you press enter."));
    waitPage();
}

void DashboardDisplay::showStatus(const ExamView &v)
{
    set_view(v);
    if (!enter())
        return (ModernDisplay::showStatus(v));
    draw();
}

void DashboardDisplay::showHelp(bool cheats)
{
    if (!term_screen_active())
        return (ModernDisplay::showHelp(cheats));
    g.help = true;
    g.help_cheats = cheats;
    draw();
    char c = 0;
    waitKey(c);
    g.help = false;
    draw();
}

Action DashboardDisplay::readCommand(const ExamView &v)
{
    set_view(v);
    if (!enter())
        return (ModernDisplay::readCommand(v));
    while (1)
    {
        draw();
        char c = 0;
        Key k = waitKey(c);
        int page = std::max(term_height() * 3 / 5 - 3, 1);
        if (k == Key::Up || (g.subject_focus && k == Key::Char && c == 'k'))
            scrollSubject(-1);
        else if (k == Key::Down || (g.subject_focus && k == Key::Char && c == 'j'))
            scrollSubject(1);
        else if (k == Key::PageUp)
            scrollSubject(-page);
        else if (k == Key::PageDown || (g.subject_focus && k == Key::Char && c == ' '))
            scrollSubject(page);
        else if (k == Key::Esc)
            g.subject_focus = false;
        else if (k == Key::Tab || (k == Key::Char && c == 's'))
            g.subject_focus = !g.subject_focus;
        else if (k != Key::Char)
            continue;
        else if (c == 'g')
            return (Action::Grade);
        else if (c == 't')
            showTrace();
        else if (c == ',')
            return (Action::Settings);
        else if (c == '?')
            return (Action::Help);
        else if (c == 'q')
            return (Action::Finish);
        else if (c == ':')
        {
            std::string command = ask(":", true);
            size_t a = command.find_first_not_of(" \t"), b = command.find_last_not_of(" \t");
            if (a != std::string::npos)
                return (parse_action(command.substr(a, b - a + 1)));
        }
        else if (c == 'n')
            return (Action::NewEx);
        else if (c == 'f')
            return (Action::ForceSuccess);
        else if (c == 'r')
            return (Action::RemoveGradeTime);
        else if (c == 'G')
            return (Action::GradeNow);
    }
}

// ==> Typed "yes" leaves the exam; the main menu (or the quit) comes next, still full screen
bool DashboardDisplay::confirmFinish(void)
{
    if (!term_screen_active())
        return (ModernDisplay::confirmFinish());
    return (ask("Leave the exam? All progress is lost. Type yes:", true) == "yes");
}

// ==> End summary: enter goes back to the main menu. After a failed grade the card simply
// stays in the panel; after a pass, enter goes on.
void DashboardDisplay::waitEnter(void)
{
    if (!term_screen_active())
        return (ModernDisplay::waitEnter());
    if (g.page.on && g.page.end)
    {
        waitPage();
        g.page.end = false;
        return;
    }
    if (g.skip_wait || !g.has_result || !g.result.passed)
    {
        g.skip_wait = false;
        return;
    }
    bool last = g.result.level >= g.result.level_max;
    g.ask = Ask::Enter;
    g.question = sym(Sym::Check) + " passed " + dot() + " enter " + sym(Sym::Prompt) + (last ? " see your results" : " next exercise");
    while (1)
    {
        draw();
        char c = 0;
        if (waitKey(c) == Key::Enter)
            break;
    }
    g.ask = Ask::None;
}

bool DashboardDisplay::confirmGrade(const std::string &beta_author)
{
    if (!term_screen_active())
        return (ModernDisplay::confirmGrade(beta_author));
    std::string question = "Submit " + g.view.ex_name + "? A failed try means fewer points. y/n";
    if (!beta_author.empty())
        question = "! beta exercise by " + beta_author + " " + dot() + " " + question;
    std::string answer = ask(question, false);
    return (answer == "y" || answer == "Y");
}

void DashboardDisplay::cooldownError(time_t seconds)
{
    if (!term_screen_active())
        return (ModernDisplay::cooldownError(seconds));
    setMessage(paint(Color::Current, sym(Sym::Cooldown)) + " next grade in " + paint(Color::Warning, wait_time(seconds)) + dim(": take the time to test more"), false);
    draw();
}

void DashboardDisplay::gradingStarted(void)
{
}

// ==> The tester runs between testerStarted and testerFinished: its output goes to /dev/null
// and a spinner thread draws the grade panel
void DashboardDisplay::testerStarted(const std::string &name)
{
    if (!term_screen_active())
        return (ModernDisplay::testerStarted(name));
    g.grading = true;
    g.grading_name = name;
    draw();
    term_quiet_begin();
    spinning_ = true;
    spinner_thread_ = std::thread(&DashboardDisplay::spin, this);
}

void DashboardDisplay::spin(void)
{
    while (spinning_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(90));
        std::lock_guard<std::mutex> lock(draw_mutex_);
        g.spin_frame++;
        drawFrame();
        screen_.flush();
    }
}

void DashboardDisplay::testerFinished(void)
{
    if (!spinning_ && !spinner_thread_.joinable())
        return (ModernDisplay::testerFinished());
    spinning_ = false;
    spinner_thread_.join();
    term_quiet_end();
    g.grading = false;
    screen_.invalidate(); // in case something reached the terminal anyway
}

void DashboardDisplay::showResult(const GradeResult &r)
{
    if (!term_screen_active())
        return (ModernDisplay::showResult(r));
    g.has_result = true;
    g.result = r;
    g.grading = false;
    g.skip_wait = !r.passed;
    draw();
}

// ==> Summary page; the exam then calls waitEnter and goes back to the main menu. It is
// printed again on the normal screen if the program quits before another exam starts.
void DashboardDisplay::showEnd(const ExamSummary &s)
{
    g.ended = true;
    g.summary = s;
    if (!enter())
        return (ModernDisplay::showEnd(s));
    ui::BoxOpts opts;
    end_card(s, opts);
    new_page("results", opts.title, key_hint("enter", "main menu"));
    g.page.color = opts.color;
    g.page.end = true;
    g.page.lines = end_lines(s);
    g.page.after.assign(1, opts.footer);
    draw();
}

void DashboardDisplay::message(Msg m)
{
    if (!term_screen_active())
        return (ModernDisplay::message(m));
    std::string text = msg_text(m);
    if (text.empty())
        return;
    setMessage(text, false);
    draw();
}

void DashboardDisplay::error(const std::string &text)
{
    if (!term_screen_active())
        return (ModernDisplay::error(text));
    setMessage(text, true);
    draw();
}

// ==> Practice: Modern's screens on the normal screen for now. The full screen is left here and
// entered again by the next Dashboard screen (the main menu when Practice ends).
bool DashboardDisplay::practiceGuard(const RestoreView &saved)
{
    term_screen_leave();
    return (ModernDisplay::practiceGuard(saved));
}

int DashboardDisplay::practicePicker(const std::vector<practice_entry> &index, std::string &query)
{
    term_screen_leave();
    return (ModernDisplay::practicePicker(index, query));
}

void DashboardDisplay::practiceStart(const PracticeView &v)
{
    term_screen_leave();
    ModernDisplay::practiceStart(v);
}

void DashboardDisplay::practiceStatus(const PracticeView &v, bool clear)
{
    term_screen_leave();
    ModernDisplay::practiceStatus(v, clear);
}

void DashboardDisplay::practiceHelp(void)
{
    term_screen_leave();
    ModernDisplay::practiceHelp();
}

PracticeAction DashboardDisplay::practiceCommand(const PracticeView &v)
{
    term_screen_leave();
    return (ModernDisplay::practiceCommand(v));
}

void DashboardDisplay::practiceCooldown(time_t seconds)
{
    term_screen_leave();
    ModernDisplay::practiceCooldown(seconds);
}

std::string DashboardDisplay::practicePassed(const GradeResult &r)
{
    term_screen_leave();
    return (ModernDisplay::practicePassed(r));
}
