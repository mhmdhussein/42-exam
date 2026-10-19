#include "exam.hpp"
#include <algorithm>

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


// ==> Position of a filtered entry inside the full index
int practice_position(const std::vector<practice_entry> &index, const practice_entry &entry)
{
    for (size_t i = 0; i < index.size(); i++)
        if (index[i].name == entry.name && index[i].part == entry.part && index[i].exam == entry.exam && index[i].lvl == entry.lvl)
            return (i);
    return (-1);
}

// ==> Saved exam check: true to go on with practice, false to go back
bool exam::practice_guard(void)
{
    if (!file_exists(".system/exam_token/current_token.txt"))
        return (true);

    std::ifstream file(".system/exam_token/current_token.txt");
    std::string skip, saved_exam, saved_student, saved_end, saved_name;
    file >> skip >> saved_end >> saved_exam >> saved_student >> skip >> skip >> saved_name;
    file.close();

    RestoreView saved;
    saved.student = saved_student == "1";
    saved.exam_number = atoi(saved_exam.c_str());
    saved.ex_name = saved_name;
    saved.end_time = atol(saved_end.c_str());
    if (!display->practiceGuard(saved))
        return (false);
    remove(".system/exam_token/current_token.txt");
    display->message(Msg::PracticeTokenDeleted);
    return (true);
}

PracticeView exam::practice_view(const practice_entry &entry)
{
    PracticeView v;
    v.entry = entry;
    v.path = get_path();
    v.tries = current_ex->get_assignement();
    v.cooldown_until = setting_pcd ? current_ex->time_bef_grade : 0;
    return (v);
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
    display->practiceStart(practice_view(entry));
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
        display->message(Msg::PracticeOnlyOne);
        return (cur);
    }
    srand(time(NULL));
    return (candidates[rand() % candidates.size()]);
}

// ==> Practice prompt, returns the next exercise to start or -1 to leave practice
int exam::practice_prompt(std::vector<practice_entry> &index, int cur, std::string &query)
{
    while (1)
    {
        PracticeAction action = display->practiceCommand(practice_view(index[cur]));
        if (action == PracticeAction::Exit)
            return (-1);
        else if (action == PracticeAction::Help)
            display->practiceHelp();
        else if (action == PracticeAction::Status)
            display->practiceStatus(practice_view(index[cur]), false);
        else if (action == PracticeAction::Pick)
        {
            int next = display->practicePicker(index, query);
            if (next >= 0)
                return (next);
            display->practiceStatus(practice_view(index[cur]), true);
        }
        else if (action == PracticeAction::Random)
        {
            int next = practice_random(index, cur);
            if (next != cur)
                return (next);
        }
        else if (action == PracticeAction::Grade)
        {
            if (setting_pcd && current_ex->time_bef_grade > time(NULL))
            {
                display->practiceCooldown(current_ex->time_bef_grade - time(NULL));
                continue;
            }
            int result = run_tester();
            if (result == 0)
            {
                current_ex->up_assignement();
                if (setting_pcd)
                {
                    current_ex->set_time_bef_grade(time(NULL) + current_ex->grade_time() * 60);
                    if (current_ex->time_bef_grade > time(NULL))
                        last_result.retry_at = current_ex->time_bef_grade;
                }
                display->showResult(last_result);
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
                std::string choice = display->practicePassed(last_result);
                if (choice == "0")
                    return (-1);
                if (choice == "1")
                    return (cur);
                if (choice == "3")
                    return (practice_random(index, cur));
                int next = display->practicePicker(index, query);
                return (next >= 0 ? next : cur);
            }
        }
        else
            display->message(Msg::UnknownCommand);
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
        display->message(Msg::PracticeNoExercises);
        return;
    }
    std::string query = "";
    current_ex = NULL;
    int cur = display->practicePicker(index, query);
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
