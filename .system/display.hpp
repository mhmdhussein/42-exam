#pragma once

#include <string>
#include <vector>
#include <ctime>
#include "ui.hpp"

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

enum class DisplayStyle { Modern = 0, Legacy = 1, Exact = 2 };

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

// ==> What the user asked for at the exam prompt
enum class Action
{
    Grade,
    Status,
    Settings,
    Help,
    Finish,
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
    SettingsSaved
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
    virtual Action readCommand(time_t end_time) = 0;
    virtual bool confirmFinish(void) = 0;
    virtual void waitEnter(void) = 0;

    // grading
    virtual bool confirmGrade(const std::string &beta_author) = 0;
    virtual void cooldownError(time_t seconds) = 0;
    virtual void gradingStarted(void) = 0;
    virtual void testerStarted(const std::string &name) = 0; // the tester runs between these two
    virtual void testerFinished(void) = 0;
    virtual void showResult(const GradeResult &r) = 0;
    virtual void showEnd(const ExamSummary &s) = 0;

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
    Action readCommand(time_t end_time);
    bool confirmFinish(void);
    void waitEnter(void);

    bool confirmGrade(const std::string &beta_author);
    void cooldownError(time_t seconds);
    void gradingStarted(void);
    void testerStarted(const std::string &name);
    void testerFinished(void);
    void showResult(const GradeResult &r);
    void showEnd(const ExamSummary &s);

    void message(Msg m);
    void error(const std::string &text);

protected:
    void rollExercise(const ExamView &v);
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
    Action readCommand(time_t end_time);
    bool confirmFinish(void);
    void waitEnter(void);

    bool confirmGrade(const std::string &beta_author);
    void cooldownError(time_t seconds);
    void gradingStarted(void);
    void testerStarted(const std::string &name);
    void testerFinished(void);
    void showResult(const GradeResult &r);
    void showEnd(const ExamSummary &s);

    void message(Msg m);
    void error(const std::string &text);

private:
    struct Option
    {
        std::string key; // returned when picked, also typed to pick it directly
        std::string label;
        std::string hint;
    };
    std::string pick(const std::string &header, const std::vector<Option> &options, int &cursor, const std::string &footer, const std::string &back);

    int start_cursor_;
    int part_cursor_;
    int settings_cursor_;
    int settings_drawn_; // lines of the settings frame on screen, -1 = none yet
    std::string next_step_; // waitEnter wording after a result card
    ui::Spinner spinner_;
};

// shared by the displays
void play_login(const std::string &username);
Action read_action(const std::string &prompt); // readline until a non-blank line

// Display style setting -> Display subclass
Display *make_display(DisplayStyle style);
