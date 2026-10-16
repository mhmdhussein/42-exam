#include "exam.hpp"
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <algorithm>

// terminal state, global so sigc/sigd can restore it
static struct termios g_saved_termios;
static bool g_raw_active = false;

// ==> Put the terminal back in line mode with a visible cursor
void practice_restore_terminal(void)
{
    if (!g_raw_active)
        return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_saved_termios);
    write(STDOUT_FILENO, "\e[?25h", 6);
    g_raw_active = false;
}

static bool enter_raw_mode(void)
{
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
        return (false);
    if (tcgetattr(STDIN_FILENO, &g_saved_termios) == -1)
        return (false);
    struct termios raw = g_saved_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        return (false);
    g_raw_active = true;
    write(STDOUT_FILENO, "\e[?25l", 6);
    return (true);
}

static std::string to_lower(std::string s)
{
    for (size_t i = 0; i < s.size(); i++)
        s[i] = tolower(s[i]);
    return (s);
}

static bool is_number(const std::string &s)
{
    if (s.empty())
        return (false);
    for (size_t i = 0; i < s.size(); i++)
        if (!isdigit(s[i]))
            return (false);
    return (true);
}

static std::vector<std::string> list_subdirs(const std::string &path)
{
    std::vector<std::string> dirs;
    DIR *dir = opendir(path.c_str());
    struct dirent *entry;
    if (dir == NULL)
        return (dirs);
    while ((entry = readdir(dir)) != NULL)
    {
        std::string folder = entry->d_name;
        if (folder[0] != '.')
            dirs.push_back(folder);
    }
    closedir(dir);
    return (dirs);
}

// ==> Names found in success/success_ex and success/practice_ex
static std::vector<std::string> passed_names(void)
{
    std::vector<std::string> names;
    const char *files[] = {"success/success_ex", "success/practice_ex"};
    for (int f = 0; f < 2; f++)
    {
        std::ifstream file(files[f]);
        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string name;
            if (iss >> name)
                names.push_back(name);
        }
    }
    return (names);
}

static bool entry_less(const practice_entry &a, const practice_entry &b)
{
    if (a.part != b.part)
        return (a.part < b.part);
    if (a.exam != b.exam)
        return (a.exam < b.exam);
    if (a.lvl != b.lvl)
        return (a.lvl < b.lvl);
    return (a.name < b.name);
}

// ==> Walk .subjects/<PART>/exam_0X/<level>/<name>/
static std::vector<practice_entry> practice_index(void)
{
    std::vector<practice_entry> index;
    std::vector<std::string> passed = passed_names();
    const char *parts[] = {"PISCINE_PART", "STUD_PART"};
    for (int p = 0; p < 2; p++)
    {
        std::string part_path = std::string(".subjects/") + parts[p] + "/";
        std::vector<std::string> exams = list_subdirs(part_path);
        for (size_t e = 0; e < exams.size(); e++)
        {
            if (exams[e].compare(0, 5, "exam_") != 0 || !is_number(exams[e].substr(5)))
                continue;
            std::vector<std::string> lvls = list_subdirs(part_path + exams[e] + "/");
            for (size_t l = 0; l < lvls.size(); l++)
            {
                if (!is_number(lvls[l]))
                    continue;
                std::vector<std::string> names = list_subdirs(part_path + exams[e] + "/" + lvls[l] + "/");
                for (size_t n = 0; n < names.size(); n++)
                {
                    practice_entry entry;
                    entry.name = names[n];
                    entry.part = parts[p];
                    entry.exam = atoi(exams[e].substr(5).c_str());
                    entry.lvl = atoi(lvls[l].c_str());
                    entry.passed = std::find(passed.begin(), passed.end(), names[n]) != passed.end();
                    index.push_back(entry);
                }
            }
        }
    }
    std::sort(index.begin(), index.end(), entry_less);
    return (index);
}

// ==> Keep entries matching every word of the query
std::vector<practice_entry> practice_filter(const std::vector<practice_entry> &index, const std::string &query, bool &bad_filter)
{
    std::vector<practice_entry> result;
    std::istringstream iss(to_lower(query));
    std::vector<std::string> words;
    std::string word;
    bad_filter = false;
    while (iss >> word)
        words.push_back(word);

    for (size_t w = 0; w < words.size(); w++)
    {
        size_t colon = words[w].find(':');
        if (colon == std::string::npos)
            continue;
        std::string key = words[w].substr(0, colon);
        std::string value = words[w].substr(colon + 1);
        if (key == "part" ? (value != "stud" && value != "piscine") : (key == "exam" || key == "lvl") ? !is_number(value) : true)
        {
            bad_filter = true;
            return (result);
        }
    }

    for (size_t i = 0; i < index.size(); i++)
    {
        const practice_entry &entry = index[i];
        bool match = true;
        for (size_t w = 0; w < words.size() && match; w++)
        {
            const std::string &word = words[w];
            size_t colon = word.find(':');
            if (word == "passed")
                match = entry.passed;
            else if (word == "-passed")
                match = !entry.passed;
            else if (colon == std::string::npos)
                match = to_lower(entry.name).find(word) != std::string::npos;
            else
            {
                std::string key = word.substr(0, colon);
                std::string value = word.substr(colon + 1);
                if (key == "part")
                    match = (value == "stud") == (entry.part == "STUD_PART");
                else if (key == "exam")
                    match = atoi(value.c_str()) == entry.exam;
                else
                    match = atoi(value.c_str()) == entry.lvl;
            }
        }
        if (match)
            result.push_back(entry);
    }
    return (result);
}

static std::string entry_row(const practice_entry &entry)
{
    std::ostringstream row;
    row << std::left << std::setw(24) << entry.name << " "
        << std::setw(7) << (entry.part == "STUD_PART" ? "STUD" : "PISCINE")
        << " exam_0" << entry.exam << "  lvl " << entry.lvl;
    if (entry.passed)
        row << "  " << LIME << "✓" << RESET;
    return (row.str());
}

// ==> Position of a filtered entry inside the full index
static int index_of(const std::vector<practice_entry> &index, const practice_entry &entry)
{
    for (size_t i = 0; i < index.size(); i++)
        if (index[i].name == entry.name && index[i].part == entry.part && index[i].exam == entry.exam && index[i].lvl == entry.lvl)
            return (i);
    return (-1);
}

// ==> Line-mode picker for pipes / non-terminals
int exam::practice_picker_lines(std::vector<practice_entry> &index, std::string &query)
{
    while (1)
    {
        bool bad_filter;
        std::vector<practice_entry> found = practice_filter(index, query, bad_filter);
        std::cout << std::endl;
        for (size_t i = 0; i < found.size(); i++)
            std::cout << std::right << std::setw(4) << i + 1 << "  " << entry_row(found[i]) << std::endl;
        std::cout << "  " << found.size() << " / " << index.size() << " exercises";
        if (bad_filter)
            std::cout << RED << "   unknown filter" << RESET;
        std::cout << std::endl;
        std::cout << "Number to select, new query to filter, empty to go back: ";
        std::string input;
        if (!std::getline(std::cin, input))
            sigd();
        if (input.empty())
            return (-1);
        if (is_number(input) && atoi(input.c_str()) >= 1 && atoi(input.c_str()) <= (int)found.size())
            return (index_of(index, found[atoi(input.c_str()) - 1]));
        query = input;
    }
}

// ==> Live search bar, returns the index of the selected exercise or -1 on Esc
int exam::practice_picker(std::vector<practice_entry> &index, std::string &query)
{
    if (!enter_raw_mode())
        return (practice_picker_lines(index, query));

    int selected = 0;
    int offset = 0;
    while (1)
    {
        bool bad_filter;
        std::vector<practice_entry> found = practice_filter(index, query, bad_filter);
        if (selected >= (int)found.size())
            selected = found.size() ? found.size() - 1 : 0;

        struct winsize ws;
        int height = 24;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0)
            height = ws.ws_row;
        int rows = std::max(height - 3, 1);
        if (selected < offset)
            offset = selected;
        if (selected >= offset + rows)
            offset = selected - rows + 1;

        std::string frame = "\e[H\e[J";
        frame += std::string(WHITE) + BOLD + "Search: " + RESET + query + "█\r\n";
        for (int i = offset; i < (int)found.size() && i < offset + rows; i++)
        {
            if (i == selected)
                frame += std::string(YELLOW) + BOLD + "> " + RESET + entry_row(found[i]) + "\r\n";
            else
                frame += "  " + entry_row(found[i]) + "\r\n";
        }
        std::ostringstream footer;
        footer << "  " << found.size() << " / " << index.size() << " exercises";
        if (bad_filter)
            footer << RED << "   unknown filter" << RESET;
        footer << "        ↑↓ move · Enter select · Esc back";
        frame += footer.str();
        write(STDOUT_FILENO, frame.c_str(), frame.size());

        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0)
        {
            practice_restore_terminal();
            sigd();
        }
        if (c == '\e')
        {
            // a lone Esc has nothing following it within a short delay
            struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
            char seq[2];
            if (poll(&pfd, 1, 50) <= 0 || read(STDIN_FILENO, &seq[0], 1) <= 0)
            {
                practice_restore_terminal();
                return (-1);
            }
            if (poll(&pfd, 1, 50) > 0 && read(STDIN_FILENO, &seq[1], 1) > 0 && seq[0] == '[')
            {
                if (seq[1] == 'A' && selected > 0)
                    selected--;
                else if (seq[1] == 'B' && selected + 1 < (int)found.size())
                    selected++;
            }
        }
        else if (c == 16 && selected > 0)
            selected--;
        else if (c == 14 && selected + 1 < (int)found.size())
            selected++;
        else if (c == '\n' || c == '\r')
        {
            if (found.empty())
                continue;
            practice_restore_terminal();
            std::cout << std::endl;
            return (index_of(index, found[selected]));
        }
        else if ((c == 127 || c == 8) && !query.empty())
        {
            query.erase(query.size() - 1);
            selected = 0;
        }
        else if (c >= 32 && c < 127)
        {
            query += c;
            selected = 0;
        }
    }
}

// ==> Saved exam check: true to go on with practice, false to go back
bool exam::practice_guard(void)
{
    if (!file_exists(".system/exam_token/current_token.txt"))
        return (true);

    std::ifstream file(".system/exam_token/current_token.txt");
    std::string skip, saved_exam, saved_student, saved_name;
    file >> skip >> skip >> saved_exam >> saved_student >> skip >> skip >> saved_name;
    file.close();

    std::string answer = "";
    while (answer != "1" && answer != "0")
    {
        system("clear");
        std::cout << std::endl
                  << RED << BOLD << "       !!" << BOLD << WHITE << " SAVED EXAM " << RED << BOLD << "!!" << RESET << std::endl
                  << std::endl;
        std::cout << WHITE << BOLD << "        " << (saved_student == "1" ? "EXAMRANK " : "EXAMWEEK ") << LIME << "0" << saved_exam << RESET << std::endl;
        std::cout << WHITE << BOLD << "    CURRENT EX: " << LIME << saved_name << RESET << std::endl
                  << std::endl;
        std::cout << "    Practice will clear the files of this exam session." << std::endl
                  << std::endl;
        std::cout << WHITE << BOLD << "            1" << RESET << std::endl;
        std::cout << WHITE << BOLD << "    |" << RED << BOLD << "  CLEAR  EXAM   " << WHITE << BOLD << "|" << RESET << BOLD << std::endl
                  << "     \\ ------------ /" << std::endl
                  << std::endl;
        std::cout << WHITE << BOLD << "            0" << RESET << std::endl;
        std::cout << WHITE << BOLD << "    |" << LIME << BOLD << "  CANCEL, BACK  " << WHITE << BOLD << "|" << RESET << BOLD << std::endl
                  << "     \\ ------------ /" << RESET << std::endl
                  << std::endl
                  << "            ";
        if (!std::getline(std::cin, answer))
            sigd();
    }
    if (answer == "0")
        return (false);
    remove(".system/exam_token/current_token.txt");
    std::cout << "   Exam token deleted" << std::endl;
    return (true);
}

// ==> Copy the subject and tester of an exercise, keeping rendu/
void exam::practice_start(practice_entry &entry)
{
    student = entry.part == "STUD_PART";
    exam_number = entry.exam;
    level = entry.lvl;
    delete current_ex;
    current_ex = new exercise(level, entry.name);
    system("rm -rf subjects .system/grading");
    copy_ex_files();

    system("clear");
    std::cout << "==================================================================" << std::endl;
    std::cout << YELLOW << BOLD << "PRACTICE" << RESET << " (no timer, no grade)" << std::endl
              << std::endl;
    practice_status(entry);
    std::cout << "==================================================================" << std::endl;
    std::cout << "Use the \"" << LIME << "grademe" << RESET << "\" command to be graded, or \"" << LIME << "help" << RESET << "\" to get some help." << std::endl;
}

void exam::practice_status(practice_entry &entry)
{
    std::cout << "Assignment: " << LIME << entry.name << RESET << " (" << get_path() << ")" << std::endl;
    std::cout << "Tries: " << YELLOW << current_ex->get_assignement() << RESET << std::endl;
    if (setting_pcd && current_ex->time_bef_grade > time(NULL))
        std::cout << "Cooldown: " << YELLOW << current_ex->time_bef_grade - time(NULL) << " seconds" << RESET << std::endl;
    std::cout << "Subject location:  " << LIME << current_path() << "/subjects/subject.en.txt" << RESET << std::endl;
    std::cout << "Exercise location: " << RED << current_path() << "/rendu/" << entry.name << "/" << RESET << std::endl;
}

// ==> Another random exercise of the same exam and level, or cur if there is none
int exam::practice_random(std::vector<practice_entry> &index, int cur)
{
    std::vector<int> candidates;
    for (size_t i = 0; i < index.size(); i++)
        if ((int)i != cur && index[i].part == index[cur].part && index[i].exam == index[cur].exam && index[i].lvl == index[cur].lvl)
            candidates.push_back(i);
    if (candidates.empty())
    {
        std::cout << "There is only one exercise in this level." << std::endl;
        return (cur);
    }
    srand(time(NULL));
    return (candidates[rand() % candidates.size()]);
}

// ==> Practice prompt, returns the next exercise to start or -1 to leave practice
int exam::practice_prompt(std::vector<practice_entry> &index, int cur, std::string &query)
{
    std::string input;

    while (1)
    {
        char *line = readline("\e[93mpractice\e[0m> ");
        if (line == NULL)
            sigd();
        input = line;
        free(line);
        while (!input.empty() && input.back() == ' ')
            input.pop_back();
        while (!input.empty() && input.front() == ' ')
            input.erase(input.begin());
        if (input.empty())
            continue;
        add_history(input.c_str());
        if (input == "exit" || input == "quit")
            return (-1);
        else if (input == "help")
        {
            std::cout << "Commands:" << std::endl;
            std::cout << LIME << "    grademe:" << RESET << " grade your exercise" << std::endl;
            std::cout << LIME << "    pick:" << RESET << " search another exercise" << std::endl;
            std::cout << LIME << "    random:" << RESET << " random exercise from the same exam and level" << std::endl;
            std::cout << LIME << "    status:" << RESET << " display the current exercise" << std::endl;
            std::cout << LIME << "    exit:" << RESET << " back to the main menu" << std::endl;
        }
        else if (input == "status")
            practice_status(index[cur]);
        else if (input == "pick")
        {
            int next = practice_picker(index, query);
            if (next >= 0)
                return (next);
            system("clear");
            practice_status(index[cur]);
        }
        else if (input == "random")
        {
            int next = practice_random(index, cur);
            if (next != cur)
                return (next);
        }
        else if (input == "grademe")
        {
            if (setting_pcd && current_ex->time_bef_grade > time(NULL))
            {
                std::cout << RED << "ERROR: " << RESET << "You must wait at least " << YELLOW << BOLD;
                if ((((current_ex->time_bef_grade - time(NULL))) / 60) >= 1)
                    std::cout << (current_ex->time_bef_grade - time(NULL)) / 60 << " minutes" << RESET << " and " << YELLOW << BOLD << (current_ex->time_bef_grade - time(NULL)) % 60 << " seconds" << RESET;
                else
                    std::cout << (current_ex->time_bef_grade - time(NULL)) << " seconds" << RESET;
                std::cout << " until next grading request (setting 5)." << std::endl;
                continue;
            }
            int result = run_tester();
            if (result == 0)
            {
                current_ex->up_assignement();
                if (setting_pcd)
                    current_ex->set_time_bef_grade(time(NULL) + current_ex->grade_time() * 60);
            }
            else if (result == 1)
            {
                system("mkdir -p success");
                std::ofstream file("success/practice_ex", std::ios::app);
                file << index[cur].name << " " << index[cur].part << "/exam_0" << index[cur].exam << "/" << index[cur].lvl << " " << time(NULL) << std::endl;
                file.close();
                for (size_t i = 0; i < index.size(); i++)
                    if (index[i].name == index[cur].name)
                        index[i].passed = true;
                std::cout << std::endl
                          << LIME << ">>>>>>>>>> SUCCESS <<<<<<<<<<" << RESET << std::endl
                          << std::endl;
                std::string choice = "";
                while (1)
                {
                    std::cout << "    " << LIME << "1" << RESET << "  retry the same exercise" << std::endl;
                    std::cout << "    " << LIME << "2" << RESET << "  pick another" << std::endl;
                    std::cout << "    " << LIME << "3" << RESET << "  random from the same level" << std::endl;
                    std::cout << "    " << LIME << "0" << RESET << "  exit to the main menu" << std::endl;
                    std::cout << "Your choice: ";
                    if (!std::getline(std::cin, choice))
                        sigd();
                    if (choice == "0")
                        return (-1);
                    if (choice == "1")
                        return (cur);
                    if (choice == "3")
                        return (practice_random(index, cur));
                    if (choice == "2")
                    {
                        int next = practice_picker(index, query);
                        return (next >= 0 ? next : cur);
                    }
                }
            }
        }
        else
            std::cout << "           **Unknown command**     type " << LIME << "help" << RESET << " for more help" << std::endl;
    }
}

// ==> Entry point from the main menu
void exam::practice_mode(void)
{
    if (!practice_guard())
        return;
    load_settings();
    std::vector<practice_entry> index = practice_index();
    if (index.empty())
    {
        std::cout << "Error: no exercise found in .subjects/" << std::endl;
        return;
    }
    std::string query = "";
    current_ex = NULL;
    int cur = practice_picker(index, query);
    while (cur >= 0)
    {
        practice_start(index[cur]);
        cur = practice_prompt(index, cur, query);
    }
    delete current_ex;
    current_ex = NULL;
    level = 0;
    system("rm -rf subjects .system/grading");
}
