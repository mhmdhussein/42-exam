#pragma once

#include <string>
#include <vector>
#include <ctime>
#include "ui.hpp"
#include "term.hpp"

#define BOLD "\e[1m"
#define RESET "\e[0m"
#define CYAN "\e[36m"
#define UNDERLINE "\e[4m"
#define WHITE "\e[97m"
#define LIME "\e[92m"
#define RED "\e[91m"
#define MAGENTA "\e[95m"
#define YELLOW "\e[93m"
#define REMOVE_LINE "\e[1A\e[K"

// set by the Makefile from the VERSION file
#ifndef EXAMSHELL_VERSION
#define EXAMSHELL_VERSION "dev"
#endif

// ==> Plain data the exam fills and the display reads; the display never changes exam state

struct PassedLevel
{
    int level;
    std::string name;
    int tries; // failed tries before the success
};

struct ExamView
{
    bool student;
    int exam_number;
    bool real_mode;
    int level;
    int level_max;
    int level_per_ex;
    int level_per_ex_save;
    std::string ex_name;
    int tries;                           // failed tries on the current exercise
    std::vector<PassedLevel> passed;
    bool animate;                        // roll the exercise name before showing it
    std::vector<std::string> candidates; // exercises of the current level, for the roll
    time_t end_time;
    time_t cooldown_until; // next grademe allowed at, 0 when the cooldown is off
};

struct GradeResult
{
    bool passed;
    std::string ex_name;
    std::string trace_path; // empty when the tester left no traceback
    std::string trace_rel;  // same file, relative to the repo root
    time_t retry_at;        // next grademe allowed at, 0 = now
    int tests;              // cases passed by the shared runner, 0 = unknown
    int tries;              // tries including this one
    int level;              // level after this result
    int level_max;
    int grade;  // grade after this result
    int gained; // points this result added

    // first failed case, read from the trace (docs/TESTER_GUIDE.md format)
    bool structured;
    int case_no; // 0 = the failure happened before any case (compile error, forbidden function)
    int case_total;
    std::string reason;
    std::string command;
    std::string stdin_label;
    std::vector<std::string> expected;
    std::vector<std::string> got;
    std::vector<std::string> raw; // first lines of a trace in another format

    GradeResult(void) : passed(false), retry_at(0), tests(0), tries(0), level(0), level_max(0), grade(0), gained(0), structured(false), case_no(0), case_total(0) {}
};

struct SummaryRow
{
    int level;
    std::string name; // empty when unknown (exam saved by an older version)
    int tries;        // -1 = unknown
    long seconds;     // time spent on the level, -1 = unknown
};

struct ExamSummary
{
    bool student;
    int exam_number;
    int grade;
    long time_used;
    long time_total;
    int cheat_count;
    std::vector<SummaryRow> rows;
    std::string username;
};

enum class DisplayStyle { Modern = 0, Legacy = 1, Exact = 2, Dashboard = 3 };
const int DISPLAY_STYLES = 4;

struct SettingsView
{
    bool dse;
    bool dcc;
    DisplayStyle style;
    bool anim;
    bool pcd;
};

struct RestoreView
{
    bool student;
    int exam_number;
    std::string ex_name;
    time_t end_time;
};

// ==> One exercise of the Practice index
struct practice_entry
{
    std::string name;
    std::string part;
    int exam;
    int lvl;
    bool passed;
};
std::vector<practice_entry> practice_filter(const std::vector<practice_entry> &index, const std::string &query, bool &bad_filter);

// ==> The exercise being practised
struct PracticeView
{
    practice_entry entry;
    std::string path;      // .subjects/... folder of the exercise
    int tries;             // failed tries
    time_t cooldown_until; // next grademe allowed at, 0 when the Practice cooldown is off
};

// ==> What the user asked for at the practice prompt
enum class PracticeAction
{
    Grade,
    Pick,
    Random,
    Status,
    Help,
    Exit, // back to the main menu
    Quit, // leave the simulator (the Dashboard q key)
    Unknown
};

// ==> What the user asked for at the exam prompt
enum class Action
{
    Grade,
    Status,
    Settings,
    Help,
    Finish, // leave the exam, back to the main menu
    Quit,   // leave the simulator
    NewEx,
    ForceSuccess,
    RemoveGradeTime,
    GradeNow,
    Unknown
};

// ==> Fixed one-line messages
enum class Msg
{
    CheatsDisabled,
    FinishAborted,
    GradeTimeRemoved,
    UnknownCommand,
    OnlyOneExercise,
    NewExercise,
    MaxLevel,
    NoExercisePath,
    NoTester,
    GradeRequestSent,
    GradeAborted,
    BackupOpenError,
    RestoringToken,
    TokenDeleted,
    SettingsSaved,
    PracticeTokenDeleted,
    PracticeNoExercises,
    PracticeOnlyOne
};

class Display
{
public:
    virtual ~Display(void) {}

    // menus: draw, read the answer, return it
    virtual std::string startMenu(bool invalid) = 0;
    virtual std::string studMenu(bool invalid) = 0;
    virtual std::string piscineMenu(bool invalid) = 0;
    virtual std::string settingsMenu(const SettingsView &s) = 0;
    virtual std::string restorePrompt(const RestoreView &r) = 0;
    virtual bool confirmRegistration(bool student, int exam_number) = 0;

    // exam start
    virtual void showExplanation(void) = 0;
    virtual void connexion(const std::string &username) = 0;
    virtual void showIntro(const std::string &username, bool student, int exam_number, int level, int time_max) = 0;

    // exam prompt
    virtual void showStatus(const ExamView &v) = 0;
    virtual void showHelp(bool cheats) = 0;
    virtual Action readCommand(const ExamView &v) = 0; // v: current state, for a live clock
    virtual bool confirmFinish(bool quit) = 0; // quit: leave the simulator, not only the exam
    virtual void waitEnter(void) = 0;

    // grading
    virtual bool confirmGrade(const std::string &beta_author) = 0;
    virtual void cooldownError(time_t seconds) = 0;
    virtual void gradingStarted(void) = 0;
    virtual void testerStarted(const std::string &name) = 0; // the tester runs between these two
    virtual void testerFinished(void) = 0;
    virtual void showResult(const GradeResult &r) = 0;
    virtual void showEnd(const ExamSummary &s) = 0;

    // practice mode
    virtual bool practiceGuard(const RestoreView &saved) = 0; // saved exam found: true to clear it, false to go back
    virtual int practicePicker(const std::vector<practice_entry> &index, std::string &query) = 0; // position in index, -1 = back
    virtual void practiceStart(const PracticeView &v) = 0;
    virtual void practiceStatus(const PracticeView &v, bool clear) = 0;
    virtual void practiceHelp(void) = 0;
    virtual PracticeAction practiceCommand(const PracticeView &v) = 0;
    virtual void practiceCooldown(time_t seconds) = 0;
    virtual std::string practicePassed(const GradeResult &r) = 0; // "1" retry, "2" pick, "3" random, "0" leave

    virtual void message(Msg m) = 0;
    virtual void error(const std::string &text) = 0;
};

// ==> Default look
class LegacyDisplay : public Display
{
public:
    std::string startMenu(bool invalid);
    std::string studMenu(bool invalid);
    std::string piscineMenu(bool invalid);
    std::string settingsMenu(const SettingsView &s);
    std::string restorePrompt(const RestoreView &r);
    bool confirmRegistration(bool student, int exam_number);

    void showExplanation(void);
    void connexion(const std::string &username);
    void showIntro(const std::string &username, bool student, int exam_number, int level, int time_max);

    void showStatus(const ExamView &v);
    void showHelp(bool cheats);
    Action readCommand(const ExamView &v);
    bool confirmFinish(bool quit);
    void waitEnter(void);

    bool confirmGrade(const std::string &beta_author);
    void cooldownError(time_t seconds);
    void gradingStarted(void);
    void testerStarted(const std::string &name);
    void testerFinished(void);
    void showResult(const GradeResult &r);
    void showEnd(const ExamSummary &s);

    bool practiceGuard(const RestoreView &saved);
    int practicePicker(const std::vector<practice_entry> &index, std::string &query);
    void practiceStart(const PracticeView &v);
    void practiceStatus(const PracticeView &v, bool clear);
    void practiceHelp(void);
    PracticeAction practiceCommand(const PracticeView &v);
    void practiceCooldown(time_t seconds);
    std::string practicePassed(const GradeResult &r);

    void message(Msg m);
    void error(const std::string &text);

protected:
    void rollExercise(const ExamView &v);
    int practicePickerLines(const std::vector<practice_entry> &index, std::string &query);
};

// ==> "Exact examshell display" setting: the real examshell wording for the status
class ExamshellDisplay : public LegacyDisplay
{
public:
    void showStatus(const ExamView &v);
};

// ==> Default look: boxes, arrow-key menus, result cards (built on ui.hpp)
class ModernDisplay : public Display
{
public:
    ModernDisplay(void);

    std::string startMenu(bool invalid);
    std::string studMenu(bool invalid);
    std::string piscineMenu(bool invalid);
    std::string settingsMenu(const SettingsView &s);
    std::string restorePrompt(const RestoreView &r);
    bool confirmRegistration(bool student, int exam_number);

    void showExplanation(void);
    void connexion(const std::string &username);
    void showIntro(const std::string &username, bool student, int exam_number, int level, int time_max);

    void showStatus(const ExamView &v);
    void showHelp(bool cheats);
    Action readCommand(const ExamView &v);
    bool confirmFinish(bool quit);
    void waitEnter(void);

    bool confirmGrade(const std::string &beta_author);
    void cooldownError(time_t seconds);
    void gradingStarted(void);
    void testerStarted(const std::string &name);
    void testerFinished(void);
    void showResult(const GradeResult &r);
    void showEnd(const ExamSummary &s);

    bool practiceGuard(const RestoreView &saved);
    int practicePicker(const std::vector<practice_entry> &index, std::string &query);
    void practiceStart(const PracticeView &v);
    void practiceStatus(const PracticeView &v, bool clear);
    void practiceHelp(void);
    PracticeAction practiceCommand(const PracticeView &v);
    void practiceCooldown(time_t seconds);
    std::string practicePassed(const GradeResult &r);

    void message(Msg m);
    void error(const std::string &text);

protected:
    struct Option
    {
        std::string key; // returned when picked, also typed to pick it directly
        std::string label;
        std::string hint;
    };

private:
    std::string pick(const std::string &header, const std::vector<Option> &options, int &cursor, const std::string &footer, const std::string &back);

    int start_cursor_;
    int part_cursor_;
    int settings_cursor_;
    int settings_drawn_; // lines of the settings frame on screen, -1 = none yet
    std::string next_step_; // waitEnter wording after a result card
    ui::Spinner spinner_;
};

// ==> "Dashboard" style: the whole program on one full screen (display_dashboard.cpp): menus, exam
// start and the Practice picker as pages of one frame; the exam and Practice as panels with
// single-key controls (and a live clock in the exam).
class DashboardDisplay : public ModernDisplay
{
public:
    DashboardDisplay(void);
    ~DashboardDisplay(void);

    std::string startMenu(bool invalid);
    std::string studMenu(bool invalid);
    std::string piscineMenu(bool invalid);
    std::string settingsMenu(const SettingsView &s);
    std::string restorePrompt(const RestoreView &r);
    bool confirmRegistration(bool student, int exam_number);

    void showExplanation(void);
    void connexion(const std::string &username);
    void showIntro(const std::string &username, bool student, int exam_number, int level, int time_max);

    void showStatus(const ExamView &v);
    void showHelp(bool cheats);
    Action readCommand(const ExamView &v);
    bool confirmFinish(bool quit);
    void waitEnter(void);

    bool confirmGrade(const std::string &beta_author);
    void cooldownError(time_t seconds);
    void gradingStarted(void);
    void testerStarted(const std::string &name);
    void testerFinished(void);
    void showResult(const GradeResult &r);
    void showEnd(const ExamSummary &s);

    // Practice, in the same frame
    bool practiceGuard(const RestoreView &saved);
    int practicePicker(const std::vector<practice_entry> &index, std::string &query);
    void practiceStart(const PracticeView &v);
    void practiceStatus(const PracticeView &v, bool clear);
    void practiceHelp(void);
    PracticeAction practiceCommand(const PracticeView &v);
    void practiceCooldown(time_t seconds);
    std::string practicePassed(const GradeResult &r);

    void message(Msg m);
    void error(const std::string &text);

private:
    bool enter(void); // full screen on; false = the program falls back to Modern
    void draw(void);  // whole frame into screen_, then flush
    void drawFrame(void);
    void drawPage(const ui::Rect &r); // lines and list of the current page
    void drawGrade(const ui::Rect &r);
    void drawExamSide(const ui::Rect &head, const ui::Rect &levels);     // left column: exam and levels
    void drawPracticeSide(const ui::Rect &head, const ui::Rect &levels); // left column: Practice exercise
    std::string listPick(const std::vector<Option> &options, int &cursor, const std::string &back); // Modern's pick() as a page list
    void waitPage(void); // enter goes on
    bool panelKey(Key k, char c); // subject scrolling and trace keys of the exam and Practice screens
    std::string commandLine(void); // `:` prompt in the bottom bar, trimmed
    void setMessage(const std::string &text, bool error);
    Key waitKey(char &c); // next key; redraws on clock ticks and resizes meanwhile
    std::string ask(const std::string &question, bool line); // inline prompt in the bottom bar
    void scrollSubject(int delta);
    void showTrace(void);
    void spin(void);

    ui::Screen &screen_; // shared by every DashboardDisplay: a new one only sends what changed
    std::mutex draw_mutex_; // screen_ is drawn by the main thread and the grading spinner
    std::thread spinner_thread_;
    std::atomic<bool> spinning_;
};

// exam facts, also shown in the exam pickers (exam.cpp)
int exam_levels(bool student, int exam_number);
int exam_minutes(bool student, int exam_number);

// shared by the displays
std::string read_command(const std::string &prompt); // readline until a non-blank line, trimmed
Action parse_action(const std::string &command); // typed command -> Action
Action read_action(const std::string &prompt);
PracticeAction parse_practice_action(const std::string &command); // typed command -> PracticeAction
PracticeAction read_practice_action(const std::string &prompt);
int practice_position(const std::vector<practice_entry> &index, const practice_entry &entry); // -1 if absent

// Display style setting -> Display subclass
Display *make_display(DisplayStyle style);
