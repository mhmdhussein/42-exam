#include "exam.hpp"

// ==> Store all possible exercises in a map
std::map<int, exercise> exam::list_dir(void)
{
    struct dirent *entry;
    int i = 0;
    std::map<int, exercise> list;
    std::string path;
    if (student)
        path = ".subjects/STUD_PART/exam_0" + std::to_string(exam_number) + "/" + std::to_string(level) + "/";
    else
        path = ".subjects/PISCINE_PART/exam_0" + std::to_string(exam_number) + "/" + std::to_string(level) + "/";

    DIR *dir = opendir(path.c_str());
    std::string folder;
    if (dir == NULL)
    {
        display->error("Error: can't open directory" + get_path());
        sleep(100);
        return list;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        folder = entry->d_name;
        if (folder != "." && folder != ".." && folder != ".DS_Store")
        {
            list.insert(std::pair<int, exercise>(i, exercise(get_lvl(), folder)));
            i++;
        }
    }
    closedir(dir);
    return (list);
}

// ==> Get path of actual exercise
std::string exam::get_path(void)
{
    std::string path_exam;

    if (student)
        path_exam = ".subjects/STUD_PART/exam_0" + std::to_string(exam_number) + "/" + std::to_string(level) + "/" + current_ex->get_name() + "/";
    else
        path_exam = ".subjects/PISCINE_PART/exam_0" + std::to_string(exam_number) + "/" + std::to_string(level) + "/" + current_ex->get_name() + "/";
    return (path_exam);
}

// ==> Set max level for an exam
void exam::set_max_lvl(void)
{
    if (student)
    {
        if (exam_number == 2)
            level_max = 4;
        else if (exam_number == 3)
            level_max = 2;
        else if (exam_number == 4)
            level_max = 2;
        else if (exam_number == 5)
            level_max = 2;
        else if (exam_number == 6)
            level_max = 1;
    }
    else if (exam_number == 3 || exam_number == 4)
        level_max = 4;
    else
        level_max = 8;
}

// ==> Set max hrs for exam (3 or 4)
void exam::set_max_time(void)
{
    if (student)
        time_max = 180;
    else
    {
        if (exam_number == 4)
            time_max = 480;
        else
            time_max = 240;
    }
}

// ==> First menu asking examrank number
void exam::ask_param(void)
{
    int select = 0;
    reset_folder();
    while (1)
    {
        exam_number = 0;
        while (exam_number == 0)
        {
            if (exam_number == 0)
                select = stud_or_swim();
            if (select == 1)
            {
                student = false;
                exam_number = piscine_menu();
            }
            else if (select == 2)
            {
                student = true;
                exam_number = stud_menu();
            }
        }
        if (display->confirmRegistration(student, exam_number))
            break;
    }

    set_max_lvl();
    level_per_ex = ((double)level + 1) / (double)level_max * 100;
    level_per_ex_save = level_per_ex;

    display->showExplanation();
    // =============================

    // CONNEXION ANIMATION
    if (setting_anim)
        display->connexion(username);
    set_max_time();
    display->showIntro(username, exam_number, level, time_max);
    // ====================

    // TIME SETUP
    start_time = time(0);
    end_time = start_time + (60 * time_max);
    struct tm temp;
    memset(&temp, '\0', sizeof(struct tm));
    localtime_r(&end_time, &temp);
    // ============

}

// CONSTRUCTOR/OPERATOR/GETTER/SETTER
exam::exam(void) : student(false), level_max(0), display(NULL), using_cheatcode(0), exam_grade(0), level(0), failures(0), backup(false)
{
    reelmode = true;
    waiting_time = true;
    username = getenv("USER") ? getenv("USER") : "student";
    setting_dse = false;
    setting_dcc = false;
    setting_style = DisplayStyle::Modern;
    setting_anim = true;
    setting_pcd = false;
    load_settings();
    changex = 0;
}

exam &exam::operator=(exam const &src)
{
    this->exam_grade = src.exam_grade;
    this->level = src.level;
    this->level_max = src.level_max;
    this->failures = src.failures;
    this->student = src.student;
    this->reelmode = src.reelmode;
    this->waiting_time = src.waiting_time;
    this->level_per_ex = src.level_per_ex;
    this->level_per_ex_save = src.level_per_ex_save;
    this->exam_number = src.exam_number;
    this->start_time = src.start_time;
    this->end_time = src.end_time;
    this->time_max = src.time_max;
    this->current_ex = src.current_ex;
    return (*this);
}

exam::exam(exam const &) : display(NULL) {}
exam::~exam(void)
{
    delete display;
}

void exam::up_lvl(void)
{
    level++;
}

time_t exam::get_end_time(void)
{
    return (end_time);
}

time_t exam::get_start_time(void)
{
    return (start_time);
}

int exam::get_lvl(void)
{
    return (level);
}

int exam::get_exam_number()
{
    return (exam_number);
}