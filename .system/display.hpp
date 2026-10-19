#pragma once

#include <string>
#include <vector>
#include <ctime>

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
};

struct GradeResult
{
    bool passed;
    std::string trace_path; // empty when the tester left no traceback
};

struct ExamSummary
{
    int exam_number;
    int cheat_count;
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
    virtual void showIntro(const std::string &username, int exam_number, int level, int time_max) = 0;

    // exam prompt
    virtual void showStatus(const ExamView &v) = 0;
    virtual void showHelp(bool cheats) = 0;
    virtual Action readCommand(void) = 0;
    virtual bool confirmFinish(void) = 0;
    virtual void waitEnter(void) = 0;

    // grading
    virtual bool confirmGrade(const std::string &beta_author) = 0;
    virtual void cooldownError(time_t seconds) = 0;
    virtual void gradingStarted(void) = 0;
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
    void showIntro(const std::string &username, int exam_number, int level, int time_max);

    void showStatus(const ExamView &v);
    void showHelp(bool cheats);
    Action readCommand(void);
    bool confirmFinish(void);
    void waitEnter(void);

    bool confirmGrade(const std::string &beta_author);
    void cooldownError(time_t seconds);
    void gradingStarted(void);
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

// Display style setting -> Display subclass
Display *make_display(DisplayStyle style);
