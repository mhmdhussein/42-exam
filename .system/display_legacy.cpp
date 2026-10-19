#include "exam.hpp"

// ==> First menu display
std::string LegacyDisplay::startMenu(bool invalid)
{
    std::string choice;
    system("clear");
    std::cout << WHITE << BOLD << "         42EXAM " << std::endl;
    std::cout << "     Offline simulator" << RESET << std::endl
              << std::endl
              << std::endl;
    std::cout << LIME << BOLD << "            1" << RESET << std::endl;
    std::cout << WHITE << BOLD << "    |  Piscine PART  |" << RESET << BOLD << std::endl
              << "     \\ ------------ /" << std::endl
              << std::endl;
    std::cout << LIME << BOLD << "            2" << RESET << std::endl;
    std::cout << WHITE << BOLD << "    |  Student PART  |" << RESET << BOLD << std::endl
              << "     \\ ------------ /" << std::endl
              << std::endl << std::endl;

    std::cout << LIME << BOLD << "            3" << RESET << std::endl;
    std::cout << WHITE << BOLD << "    |" << YELLOW << BOLD << "    Practice    " << WHITE << BOLD << "|" << RESET << BOLD << std::endl
              << "     \\ ------------ /" << std::endl
              << std::endl;

    std::cout << LIME << BOLD << "            4" << RESET << std::endl;
    std::cout << WHITE << BOLD << "    |" << RESET << BOLD << "  SETTINGS PART " << WHITE << BOLD << "|" << RESET << BOLD << std::endl
              << "     \\ ------------ /" << std::endl
              << std::endl
              << std::endl;

    if (invalid)
        std::cout << BOLD << RED;
    else
        std::cout << WHITE << BOLD;
    std::cout << "    Enter your choice:" << RESET << std::endl
              << "            ";
    if (!std::getline(std::cin, choice))
        sigd();
    return (choice);
}

// ==> Display the menu for the student part
std::string LegacyDisplay::studMenu(bool invalid)
{
    std::string choice;
    system("clear");
    std::cout << WHITE << BOLD << "         42EXAM " << std::endl
              << RED << "   BACK" << RESET << WHITE << BOLD << " to menu with " << RED << "0" << RESET << std::endl;
    std::cout << LIME << BOLD << "            " << RESET << std::endl
              << std::endl
              << std::endl;
    std::cout << WHITE << BOLD << "    |  Piscine PART  |" << RESET << BOLD << std::endl
              << "     \\ ------------ /" << std::endl
              << std::endl
              << LIME << "            •           " << std::endl;
    std::cout << WHITE << BOLD << "    |  Student PART  |" << std::endl;
    std::cout << std::endl
              << LIME << "            2" << RESET << WHITE << BOLD << std::endl
              << "       EXAM RANK 02" << std::endl;
    std::cout << std::endl
              << LIME << "            3" << RESET << WHITE << BOLD << std::endl
              << "       EXAM RANK 03" << std::endl;
    std::cout << std::endl
              << LIME << "            4" << RESET << WHITE << BOLD << std::endl
              << "       EXAM RANK 04" << std::endl;
    std::cout << std::endl
              << LIME << "            5" << RESET << WHITE << BOLD << std::endl
              << "       EXAM RANK 05" << std::endl;
    std::cout << std::endl
              << LIME << "            6" << RESET << WHITE << BOLD << std::endl
              << "       EXAM RANK 06";
    std::cout << RESET << BOLD << std::endl
              << "     \\ ------------ /" << std::endl;
    if (invalid)
        std::cout << BOLD << RED;
    std::cout << std::endl
              << std::endl;
    std::cout << "    Enter your choice:" << RESET << std::endl
              << "            ";
    if (!std::getline(std::cin, choice))
        sigd();
    return (choice);
}

// ==> Display the menu for the piscine part
std::string LegacyDisplay::piscineMenu(bool invalid)
{
    std::string choice;
    system("clear");
    std::cout << WHITE << BOLD << "         42EXAM " << std::endl
              << RED << "   BACK" << RESET << WHITE << BOLD << " to menu with " << RED << "0" << RESET << std::endl;
    std::cout << LIME << BOLD << "            " << RESET << std::endl
              << std::endl
              << LIME << "            •           " << std::endl
              << WHITE << BOLD << "    |  Piscine PART  |" << std::endl;
    std::cout << std::endl
              << LIME << "            1" << RESET << WHITE << BOLD << std::endl
              << "       EXAM WEEK 01" << std::endl;
    std::cout << std::endl
              << LIME << "            2" << RESET << WHITE << BOLD << std::endl
              << "       EXAM WEEK 02" << std::endl;
    std::cout << std::endl
              << LIME << "            3" << RESET << WHITE << BOLD << std::endl
              << "       EXAM WEEK 03" << std::endl;
    std::cout << std::endl
              << LIME << "            4" << RESET << WHITE << BOLD << std::endl
              << "       EXAM WEEK 04" << std::endl;
    std::cout << RESET << BOLD << std::endl
              << "     \\ ------------ /" << std::endl
              << std::endl
              << std::endl;
    std::cout << WHITE << BOLD << "    |  Student PART  |" << RESET << BOLD << std::endl
              << "     \\ ------------ /" << std::endl
              << std::endl;
    if (invalid)
        std::cout << BOLD << RED;
    std::cout << std::endl;
    std::cout << "    Enter your choice:" << RESET << std::endl
              << "            ";
    if (!std::getline(std::cin, choice))
        sigd();
    return (choice);
}

static void print_toggle(bool on)
{
    if (on)
        std::cout << LIME << BOLD << " ON" << RESET << std::endl;
    else
        std::cout << RED << BOLD << " OFF" << RESET << std::endl;
}

static const char *style_name(DisplayStyle style)
{
    if (style == DisplayStyle::Exact)
        return ("Exact examshell");
    if (style == DisplayStyle::Legacy)
        return ("Legacy");
    return ("Modern");
}

// ==> Setting MENU, returns the key typed (read word by word)
std::string LegacyDisplay::settingsMenu(const SettingsView &s)
{
    std::string input;
    system("clear");

    std::cout << WHITE << BOLD << "     === SETTINGS MENU ===" << std::endl << RED << "          BACK" << RESET << WHITE << BOLD << " with " << RED << "0" << RESET << std::endl << std::endl;

    std::cout << LIME << "1." << WHITE << BOLD << " Enable exercises you already passed";
    print_toggle(s.dse);
    std::cout << LIME << "2." << WHITE << BOLD << " Enable cheat commands";
    print_toggle(s.dcc);
    std::cout << LIME << "3." << WHITE << BOLD << " Display style " << LIME << BOLD << style_name(s.style) << RESET << std::endl;
    std::cout << LIME << "4." << WHITE << BOLD << " Login animation";
    print_toggle(s.anim);
    std::cout << LIME << "5." << WHITE << BOLD << " Cooldown in Practice mode";
    print_toggle(s.pcd);
    if (!(std::cin >> input))
        sigd();
    return (input);
}

// ==> Saved exam found at startup: "1" to restore, "2" to erase
std::string LegacyDisplay::restorePrompt(const RestoreView &r)
{
    system("clear");
    std::cout << std::endl
              << RED << BOLD << "       !!" << BOLD << WHITE << " BACKUP " << RED << BOLD << "!!" << std::endl
              << std::endl;
    if (r.student)
        std::cout << WHITE << BOLD << "        EXAMRANK " << LIME << "0" << r.exam_number << WHITE << std::endl;
    else
        std::cout << WHITE << BOLD << "        EXAMWEEK " << LIME << "0" << r.exam_number << WHITE << std::endl;
    std::cout << "    CURRENT EX: " << LIME << r.ex_name << RESET << std::endl;
    std::cout << RED << "    " << remaining_time(r.end_time) << std::endl;
    std::cout << RED << "           left" << std::endl
              << std::endl;

    std::cout << WHITE << BOLD << "            1" << RESET << std::endl;
    std::cout << WHITE << BOLD << "    |" << LIME << BOLD << "  RESTORE EXAM  " << WHITE << BOLD << "|" << RESET << BOLD << std::endl
              << "     \\ ------------ /" << std::endl
              << std::endl;
    std::cout << WHITE << BOLD << "            2" << RESET << std::endl;
    std::cout << WHITE << BOLD << "     | " << RED << BOLD << " ERASE EXAM" << WHITE << BOLD << "  |" << RESET << BOLD << std::endl
              << "      \\ ---------- /" << std::endl
              << std::endl
              << std::endl;
    std::cout << "            ";

    std::string answer;
    std::cin >> answer;
    while (answer != "1" && answer != "2")
    {
        std::cout << REMOVE_LINE << "            ";
        std::cin >> answer;
    }
    return (answer);
}

// ==> Exam chosen in the menus, true when confirmed
bool LegacyDisplay::confirmRegistration(bool student, int exam_number)
{
    std::cout << REMOVE_LINE << REMOVE_LINE << REMOVE_LINE << std::endl;
    if (student)
        std::cout << LIME << BOLD << "       EXAM RANK 0" << exam_number << RESET << std::endl;
    else
        std::cout << LIME << BOLD << "       EXAM WEEK 0" << exam_number << RESET << std::endl;
    std::cout << "   Confirm" << BOLD << WHITE << " Registration" << RESET << "?" << std::endl
              << "          (y/n)" << std::endl
              << "            ";
    std::string confirm;
    if (!std::getline(std::cin, confirm))
        sigd();
    return (confirm == "y" || confirm == "Y");
}

void LegacyDisplay::showExplanation(void)
{
    std::string enter;
    system("clear");

    std::cout << std::endl
              << "\x1B[32m        EXPLANATION : \e[97m\e[1m" << std::endl
              << std::endl;
    std::cout << "     ⚠️  You have to work from a new window to keep this one \x1B[32mavailable\e[97m\e[1m\n"
              << std::endl;
    std::cout << "     📝 A random subject named \x1B[32msubject.en.txt\e[97m\e[1m will be generated" << std::endl;
    std::cout << "         > You must write your file (example.c) in the assign folder (see subject)," << std::endl;
    std::cout << "           this folder must be in folder: \x1B[32mrendu\e[97m\e[1m" << std::endl
              << std::endl;
    std::cout << "     🎓 Once completed, you can push/correct your project with : \x1B[32mgrademe\e[97m\e[1m" << std::endl;
    std::cout << "         If your level is validated, you move on to the next level 🎉" << std::endl;
    std::cout << "         If not, you have to start again ❌" << std::endl
              << std::endl;
    std::cout << "     ⌛️ Warning: The more you try to get the same project corrected, \n     the longer you will have to wait to get it \x1B[32mcorrected\e[97m\e[1m.\n\n"
              << std::endl;
    std::cout << "     📌 Nice reminder : Here you don't need to use GIT.\n         Remember that during the exam you will have to use it to push your project !\n\n"
              << std::endl;

    std::cout << RED << "     ‼️  DICLAIMER" << WHITE << std::endl;
    std::cout << "         This program is " << RED << "not" << WHITE << " the real 42 exam and is " << RED << "not" << WHITE << " made by 42." << std::endl;
    std::cout << "         It is a private, offline exam simulator." << std::endl << std::endl;
    std::cout << RESET << "     (Press enter to continue...)" << std::endl
              << "      ";
    if (!std::getline(std::cin, enter))
        sigd();
}

// ==> display connexion animation
void LegacyDisplay::connexion(const std::string &username)
{
    system("clear");
    std::cout << RESET;
    std::string examsystem = "examshell";
    for (size_t i = 0; i < examsystem.length(); i++)
    {
        std::cout << examsystem[i];
        usleep(70000);
        fflush(stdout);
    }
    std::cout << std::endl;
    usleep(600000);
    system("clear");
    std::cout << BOLD << UNDERLINE << "ExamShell v2.1" << RESET << std::endl
              << std::endl;
    std::cout << BOLD << "login:" << RESET;
    fflush(stdout);
    usleep(600000);
    for (size_t i = 0; i < username.size(); i++)
    {
        usleep(100000);
        std::cout << username[i];
        fflush(stdout);
    }

    usleep(400000);
    std::cout << std::endl
              << BOLD << "password:" << RESET;
    fflush(stdout);
    srand(time(NULL));
    int password_size = rand() % 10 + 4;
    for (int i = 0; i < password_size; i++)
    {
        usleep(150000);
        std::cout << "*";
        fflush(stdout);
    }
    std::cout << std::endl
              << std::endl
              << std::endl;
}

void LegacyDisplay::showIntro(const std::string &username, int exam_number, int level, int time_max)
{
    std::string enter;
    std::cout << "You're connected " << LIME << username << RESET << "!" << std::endl;
    std::cout << "You can log out at any time. If this program tells you you earned points,\nthen they will be counted whatever happens.\n"
              << std::endl;
    std::cout << BOLD << WHITE << "You are about to start the project " << LIME << BOLD << "ExamRank0" << exam_number << BOLD << WHITE << ", in " << MAGENTA << "REAL" << BOLD << WHITE << " mode, at level " << YELLOW << level << BOLD << WHITE << "." << RESET << std::endl;
    std::cout << WHITE << BOLD << "You would have " << LIME << BOLD << (time_max / 60) << "hrs " << BOLD << WHITE << "to complete this project." << RESET << std::endl
              << "Press a key to start exam 🏁" << std::endl;
    if (!std::getline(std::cin, enter))
        sigd();
}

// ==> Animation of exercise name
void LegacyDisplay::rollExercise(const ExamView &v)
{
    int pot_pts;
    if (v.level == 0)
        pot_pts = 0;
    else
        pot_pts = v.level_per_ex_save;
    std::vector<std::string>::const_iterator it = v.candidates.begin();
    std::cout << "    " << YELLOW << v.tries << RESET << ": " << LIME << *it << RESET << " for " << pot_pts << " potential points (" << CYAN << "Current" << RESET << ")" << std::endl;
    double time = 100000;
    for (int i = 0; i < 20; i++)
    {
        std::cout << REMOVE_LINE;
        std::cout << "    " << YELLOW << v.tries << RESET << ": " << LIME << *it << RESET << " for " << pot_pts << " potential points (" << CYAN << "Current" << RESET << ")" << std::endl;
        it++;
        if (it == v.candidates.end())
            it = v.candidates.begin();
        if (v.candidates.size() == 1)
            break;
        usleep(time);
        time += 10000;
    }
    std::cout << REMOVE_LINE;
    std::cout << "    " << YELLOW << v.tries << RESET << ": " << LIME << v.ex_name << RESET << " for " << pot_pts << " potential points (" << CYAN << "Current" << RESET << ")" << std::endl;
    if (v.level_per_ex != 100)
    {
        for (int i = 0; i < v.level_per_ex_save && v.level == 0; i++)
        {
            std::cout << REMOVE_LINE;
            std::cout << "    " << YELLOW << v.tries << RESET << ": " << LIME << v.ex_name << RESET << " for " << i << " potential points (" << CYAN << "Current" << RESET << ")" << std::endl;
            usleep(40000);
        }
    }
    std::cout << REMOVE_LINE;
    std::cout << "    " << YELLOW << v.tries << RESET << ": " << LIME << v.ex_name << RESET << " for " << (int)v.level_per_ex_save << " potential points (" << CYAN << "Current" << RESET << ")" << std::endl;
}

// ==> display of exam status
void LegacyDisplay::showStatus(const ExamView &v)
{
    std::cout << "==================================================================" << std::endl << std::endl;
    if (v.real_mode)
        std::cout << "Mode: " << MAGENTA << "REAL" << RESET << " | ";
    else
        std::cout << "Mode: " << YELLOW << "TEST" << RESET << " (Your grade will not be counted)" << std::endl;
    std::cout << "Current Grade: " << LIME << v.level_per_ex_save * v.level << RESET << " / 100" << std::endl;
    std::cout << std::endl;

    for (size_t l = 0; l < v.passed.size(); l++)
    {
        const PassedLevel &p = v.passed[l];
        std::cout << "  Level " << LIME << p.level << RESET << ": " << std::endl;
        for (int i = 0; i < p.tries; i++)
        {
            std::cout << "    " << YELLOW << i << RESET << ": " << LIME << p.name << RESET << " for " << (int)v.level_per_ex_save  << " potential points (" << RED << "Failure" << RESET << ")" << std::endl;
        }
        std::cout << "    " << YELLOW << p.tries << RESET << ": " << LIME << p.name << RESET << " for " << (int)v.level_per_ex_save << " potential points (" << LIME << "Success" << RESET << ")" << std::endl;
    }

    std::cout << "  Level " << LIME << v.level << RESET << ": " << std::endl;
    if (v.animate)
        rollExercise(v);
    else
    {
        for (int i = 0; i < v.tries; i++)
        {
            std::cout << "    " << YELLOW << i << RESET << ": " << LIME << v.ex_name << RESET << " for " << (int)v.level_per_ex_save << " potential points (" << RED << "Failure" << RESET << ")" << std::endl;
        }
        std::cout << "    " << YELLOW << v.tries << RESET << ": " << LIME << v.ex_name << RESET << " for " << (int)v.level_per_ex_save << " potential points (" << CYAN << "Current" << RESET << ")" << std::endl;
    }
    std::cout << std::endl
              << "Assignment: " << LIME << v.ex_name << RESET << " for " << LIME << BOLD << (double)(((double)v.level + 1) / (double)v.level_max * 100) << RESET << "xp" <<RESET << ", ";
    std::cout << "try: " << YELLOW << v.tries << RESET << std::endl << std::endl;
    std::cout << "Subject location:  " << LIME << current_path() << "/subjects/subject.en.txt" << RESET << std::endl;
    std::cout << "Exercise location: " << RED << current_path() << "/rendu/" << v.ex_name << "/" << RESET << std::endl;
    std::cout << "Here you " << RED << BOLD << "don't need" << RESET <<" to use git." << std::endl
              << std::endl;
    std::cout << "End date: " << LIME << std::put_time(std::localtime(&v.end_time), "%d/%m/%Y %H:%M:%S") << RESET << std::endl;
    std::cout << "Left time: " << LIME << remaining_time(v.end_time) << RESET << std::endl;
    std::cout << std::endl
              << "==================================================================" << std::endl;
    std::cout << "Use the \"" << LIME << "grademe" << RESET << "\" command to be graded, or \"" << LIME << "help" << RESET << "\" to get some help." << std::endl;
}

// ==> Help section
void LegacyDisplay::showHelp(bool cheats)
{
    std::cout << "Commands:" << std::endl;
    std::cout << LIME << "    help:" << RESET << " display this help" << std::endl;
    std::cout << LIME << "    settings: " << RESET << "display settings menu (display style applies right away)" << std::endl;
    std::cout << LIME << "    status:" << RESET << " display information about the exam" << std::endl;
    std::cout << LIME << "    finish:" << RESET << " exit the exam" << std::endl;
    std::cout << LIME << "    grademe:" << RESET << " grade your exercise" << std::endl;
    if (cheats)
    {
        std::cout << BOLD << LIME << "Cheat commands:" << RESET << std::endl;
        std::cout << LIME << "    force_success:" << RESET << " force a ex to success" << std::endl;
        std::cout << LIME << "    remove_grade_time:" << RESET << " remove grade time limit between two grademe" << std::endl;
        std::cout << LIME << "    gradenow:" << RESET << " instant grade exercise" << std::endl;
        std::cout << LIME << "    new_ex:" << RESET << " generate a new exercise for the same level" << std::endl;
    }
}

// ==> Shell prompt: read lines until one is not blank
Action LegacyDisplay::readCommand(void)
{
    std::string input;

    while (1)
    {
        char *line = readline("\e[93mexamshell\e[0m> ");
        if (line == NULL)
            sigd();
        input = line;
        free(line);
        while (input.back() == ' ')
            input.pop_back();
        while (input.front() == ' ')
            input.erase(input.begin());
        if (!input.empty())
            break;
    }
    add_history(input.c_str());
    if (input == "grademe")
        return (Action::Grade);
    if (input == "status")
        return (Action::Status);
    if (input == "settings")
        return (Action::Settings);
    if (input == "help")
        return (Action::Help);
    if (input == "finish" || input == "exit" || input == "quit")
        return (Action::Finish);
    if (input == "new_ex")
        return (Action::NewEx);
    if (input == "force_success")
        return (Action::ForceSuccess);
    if (input == "remove_grade_time")
        return (Action::RemoveGradeTime);
    if (input == "gradenow")
        return (Action::GradeNow);
    return (Action::Unknown);
}

bool LegacyDisplay::confirmFinish(void)
{
    std::cout << "Are you sure you want to" << RED << " exit" << RESET << " the exam?" << std::endl;
    std::cout << "All your progress will be " << RED << "lost" << RESET << "." << std::endl;
    std::cout << "Type " << LIME << BOLD << "yes" << RESET << " to confirm." << std::endl;
    std::string input;
    std::getline(std::cin, input);
    return (input == "yes");
}

void LegacyDisplay::waitEnter(void)
{
    std::cout << "(Press enter to continue...)" << std::endl;
    std::string input;
    if (!std::getline(std::cin, input))
        sigd();
}

// ==> Warning before grading, true when confirmed; beta_author is empty for non-beta exercises
bool LegacyDisplay::confirmGrade(const std::string &beta_author)
{
    if (!beta_author.empty())
    {
        std::cout << std::endl;
        std::cout << YELLOW << " ⚠️  Warning: " << RESET << "This exercise is a contribution by ";
        std::cout << YELLOW << beta_author << RESET;
        std::cout << ", it's still in " << YELLOW << "beta testing" << RESET << "." << std::endl;
        std::cout << " See the README for the exercise format." << std::endl;
        std::cout << " If you find any " << RED << "bug" << RESET << ", fix it in the exercise folder." << std::endl;
    }

    std::cout << std::endl
              << "Before continuing, please make " << RED << "ABSOLUTELY SURE" << RESET << " that you are in the right directory," << std::endl;
    std::cout << "that you didn't forget anything, etc..." << std::endl;
    std::cout << "If your assignment is wrong, you will have the same assignment" << std::endl;
    std::cout << std::endl
              << " but with less potential points to earn !" << std::endl;
    std::cout << RED << "Are you sure?" << RESET << " [y/N] ";
    std::string input;
    if (!std::getline(std::cin, input))
        sigd();
    return (input == "y" || input == "Y");
}

void LegacyDisplay::cooldownError(time_t seconds)
{
    std::cout << RED << "ERROR: " << RESET << "You must wait at least " << YELLOW << BOLD;
    if (seconds / 60 >= 1)
        std::cout << seconds / 60 << " minutes" << RESET << " and " << YELLOW << BOLD << seconds % 60 << " seconds" << RESET;
    else
        std::cout << seconds << " seconds" << RESET;
    std::cout << " until next grading request, so take your time to make more tests and be sure you will succeed next try!" << std::endl;
}

// ==> Fake wait for the grading server
void LegacyDisplay::gradingStarted(void)
{
    std::cout << std::endl
              << "We will now wait for the job to complete." << std::endl
              << "Please be " << LIME << "patient" << RESET << ", this " << LIME << "CAN" << RESET " take several minutes..." << std::endl;
    std::cout << "(10 seconds is fast, 30 seconds is expected, 3 minutes is a maximum)" << std::endl;
    srand(time(NULL));
    int waiting_seconds = rand() % 5 + 1;
    for (int i = 0; i < waiting_seconds; i++)
    {
        srand(time(NULL));
        double random = rand() % 6500000 + 250000;
        std::cout << "waiting..." << std::endl;
        usleep(random);
    }
}

void LegacyDisplay::showResult(const GradeResult &r)
{
    if (r.passed)
    {
        std::cout << std::endl
                  << LIME << ">>>>>>>>>> SUCCESS <<<<<<<<<<" << RESET << std::endl
                  << std::endl;
        return;
    }
    std::cout << RED << ">>>>>>>>>> FAILURE <<<<<<<<<<" << RESET << std::endl;
    sleep(1);
    std::cout << "You have failed the assignment." << std::endl;
    if (!r.trace_path.empty())
        std::cout << "Trace saved to " << LIME << r.trace_path << RESET << std::endl
                  << std::endl;
    else
        std::cout << "No traceback found." << std::endl
                  << std::endl;
}

void LegacyDisplay::showEnd(const ExamSummary &s)
{
    if (s.cheat_count == 0)
        std::cout << WHITE << BOLD << "🥳 Congratulation! You have finished the Exam Rank 0" << s.exam_number << " !" << std::endl;
    else
        std::cout << WHITE << BOLD << "🙁 You have finished the Exam Rank 0" << s.exam_number << ", " << RED << BOLD << "after using " << s.cheat_count << " cheat command..." << WHITE << BOLD << std::endl;
    std::cout << WHITE << BOLD << "Thanks for studying with us " << LIME << BOLD << s.username << WHITE << BOLD << " ❤️" << std::endl;
}

void LegacyDisplay::message(Msg m)
{
    switch (m)
    {
    case Msg::CheatsDisabled:
        std::cout << " ❌ Cheat commands are currently disabled, use " << LIME << BOLD << "settings" << RESET << " command." << std::endl;
        break;
    case Msg::FinishAborted:
        std::cout << " ** Abort ** " << std::endl;
        break;
    case Msg::GradeTimeRemoved:
        std::cout << "Time between grading is now removed for this exam" << std::endl;
        break;
    case Msg::UnknownCommand:
        std::cout << "           **Unknown command**     type " << LIME << "help" << RESET << " for more help" << std::endl;
        break;
    case Msg::OnlyOneExercise:
        std::cout << "You can't change exercise, there is only one exercise in this level" << std::endl;
        break;
    case Msg::NewExercise:
        std::cout << "  > You have generated a new exercise" << std::endl;
        break;
    case Msg::MaxLevel:
        std::cout << "You have reached the maximum level of this exam." << std::endl;
        break;
    case Msg::NoExercisePath:
        std::cout << "Error: Cannot load exercise, unable to find valid path" << std::endl;
        break;
    case Msg::NoTester:
        std::cout << "Error: Unable to find grading script for this exercise, it's coming soon. You can use \"force_success\" to pass this ex." << std::endl;
        break;
    case Msg::GradeRequestSent:
        std::cout << "Ok, making grading request to server now." << std::endl;
        break;
    case Msg::GradeAborted:
        std::cout << " Abort" << std::endl;
        break;
    case Msg::BackupOpenError:
        std::cout << "Error: can't open backup" << std::endl;
        break;
    case Msg::RestoringToken:
        std::cout << std::endl
                  << "   Restoring exam token..." << RESET << std::endl
                  << std::endl;
        break;
    case Msg::TokenDeleted:
        std::cout << "   Exam token deteled" << std::endl;
        break;
    case Msg::SettingsSaved:
        std::cout << REMOVE_LINE << RESET << WHITE << BOLD << "Save settings..." << std::endl;
        break;
    }
}

void LegacyDisplay::error(const std::string &text)
{
    std::cout << text << std::endl;
}
