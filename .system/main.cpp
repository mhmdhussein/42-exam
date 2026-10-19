#include "exam.hpp"

// ==> Shell prompt
void exam::exam_prompt(void)
{
    while (1)
    {
        Action action = display->readCommand();
        bool cheat = action == Action::RemoveGradeTime || action == Action::NewEx || action == Action::ForceSuccess || action == Action::GradeNow;
        if (cheat && !setting_dcc)
        {
            display->message(Msg::CheatsDisabled);
            continue;
        }
        switch (action)
        {
        case Action::Finish:
            if (display->confirmFinish())
            {
                if (std::ifstream(".system/exam_token/current_token.txt"))
                    remove(".system/exam_token/current_token.txt");
                exit(0);
            }
            display->message(Msg::FinishAborted);
            break;
        case Action::Settings:
            changex = 1;
            settings_menu();
            show_status();
            break;
        case Action::Grade:
            grademe();
            break;
        case Action::Status:
            changex = 1;
            show_status();
            break;
        case Action::NewEx:
            change_ex();
            using_cheatcode++;
            break;
        case Action::ForceSuccess:
            using_cheatcode++;
            success_ex(1);
            break;
        case Action::Help:
            display->showHelp(setting_dcc);
            break;
        case Action::RemoveGradeTime:
            display->message(Msg::GradeTimeRemoved);
            waiting_time = false;
            using_cheatcode++;
            break;
        case Action::GradeNow:
            using_cheatcode++;
            grade_request(1);
            break;
        case Action::Unknown:
            display->message(Msg::UnknownCommand);
            break;
        }
    }
}
// jo je taime

// ==> Fill the status view from the exam state
ExamView exam::make_view(void)
{
    ExamView v;
    v.real_mode = reelmode;
    v.level = level;
    v.level_max = level_max;
    v.level_per_ex = level_per_ex;
    v.level_per_ex_save = level_per_ex_save;
    v.ex_name = current_ex->get_name();
    v.tries = current_ex->get_assignement();
    for (std::map<int, exercise>::iterator it = lvl_ex.begin(); it != lvl_ex.end(); it++)
    {
        PassedLevel p;
        p.level = it->second.get_lvl();
        p.name = it->second.get_name();
        p.tries = it->second.get_assignement();
        v.passed.push_back(p);
    }
    v.animate = current_ex->get_assignement() == 0 && backup == 0 && !changex;
    for (std::map<int, exercise>::iterator it = list_ex_lvl.begin(); it != list_ex_lvl.end(); it++)
        v.candidates.push_back(it->second.get_name());
    v.end_time = end_time;
    return (v);
}

// ==> Status screen; the legacy one marks the exercise as shown so the name roll is not replayed
void exam::show_status(void)
{
    display->showStatus(make_view());
    if (setting_style != DisplayStyle::Exact)
        backup = 1;
}

// ==> Starting a new exercise/exam
bool exam::start_new_ex(void)
{
    load_settings();
    if (!backup)
    {
        list_ex_lvl = list_dir();
        exercise ex = *randomize_exercise(list_ex_lvl, setting_dse);
        current_ex = new exercise(ex);
        prepare_current_ex();
        store_data();
    }
    show_status();
    exam_prompt();
    return (true);
}

int main(void)
{
    signal(SIGINT, sigc);
    signal(SIGQUIT, sigc);
    signal(SIGTERM, sigc);

    if (file_exists("a.out"))
        remove("a.out");

    exam exm;

    if (file_exists(".system/exam_token/current_token.txt"))
        exm.restore_data();
    else
        exm.ask_param();

    exm.start_new_ex();

    return (0);
}
