#pragma once

#include "exercise.hpp"
#include "display.hpp"
#include <iostream>
#include <map>

// include for readline
#include <readline/readline.h>
#include <readline/history.h>

#include <signal.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <sstream>
#include <random>
#include <readline/readline.h>
#include <readline/history.h>

std::string current_path(void);
std::string remaining_time(time_t end_time);
std::string lastupdate(time_t end_time);
std::string remaining_time(time_t end_time);
std::string current_path(void);
exercise *randomize_exercise(std::map<int, exercise> list, bool remove_success);
exam restore_data(void);
bool file_exists(std::string path);

void reset_folder(void);

// ==> Thrown by the `finish` command, caught in main() to go back to the main menu
struct exam_finished
{
};
void sigc(int sig);
void sigd(void);

int stud_or_swim(void);
int piscine_menu(void);
int stud_menu(void);

class exam
{
public:
    exam(void);
    exam(exam const &src);
    ~exam(void);
    void ask_param(void);
    void fail_ex(void);
    void success_ex(bool force);
    int get_exam_number(void);
    int get_lvl(void);
    void store_data();
    void up_lvl(void);
    std::string get_path(void);
    time_t get_end_time(void);
    time_t get_start_time(void);
    int change_ex(void);
    void exam_prompt(void);
    void show_status(void);
    ExamView make_view(void);
    bool prepare_current_ex(void);
    void copy_ex_files(void);
    int run_tester(void);
    GradeResult last_result; // filled by run_tester
    void practice_mode(void);
    bool clean_all(void);
    void restore_data(void);
    bool start_new_ex(void);
    void reset_session(void);
    exam &operator=(exam const &src);
    std::map<int, exercise> list_dir();
    std::map<int, exercise> list_ex_lvl;
    std::map<int, exercise> lvl_ex;
    exercise *current_ex;
    bool student;
    bool waiting_time;
    int level_max;
    bool changex;
    Display *display;

private:
    void set_max_time(void);
    void grademe(void);
    void settings_menu(void);
    void grade_request(bool i);
    void end_exam(void);
    int grade_at(int passed);
    void set_max_lvl(void);
    int stud_menu(void);
    void load_settings(void);
    void save_settings(void);
    void set_display(void);
    int piscine_menu(void);
    int stud_or_swim(void);
    bool practice_guard(void);
    void practice_start(practice_entry &entry);
    PracticeView practice_view(const practice_entry &entry);
    int practice_prompt(std::vector<practice_entry> &index, int cur, std::string &query);
    int practice_random(std::vector<practice_entry> &index, int cur);

    bool setting_dse;
    bool setting_dcc;
    DisplayStyle setting_style;
    bool setting_anim;
    bool setting_pcd;
    std::string username;

    time_t start_time;
    time_t end_time;

    bool reelmode;
    int level_per_ex_save;
    int time_max;
    int exam_number;
    int using_cheatcode;
    int exam_grade;
    int level_per_ex;
    int level;
    int failures;
    bool backup;
};