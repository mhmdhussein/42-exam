#include "exam.hpp"

// ==> Store data of exam in a file
void exam::store_data()
{
    std::ofstream file;

    file.open(".system/exam_token/current_token.txt");
    file << get_start_time() << std::endl;
    file << get_end_time() << std::endl;
    file << get_exam_number() << std::endl;
    file << student << std::endl;
    file << get_lvl() << std::endl;
    file << current_ex->get_assignement() << std::endl;
    file << current_ex->get_name() << std::endl;
    file << level_max << std::endl;
    file << current_ex->time_bef_grade << std::endl;
    file << level_per_ex << std::endl;
    file << level_per_ex_save << std::endl;
    file << using_cheatcode << std::endl;
    // passed levels, one per line: level name failed_tries passed_at (missing in older tokens)
    file << lvl_ex.size() << std::endl;
    for (std::map<int, exercise>::iterator it = lvl_ex.begin(); it != lvl_ex.end(); it++)
        file << it->second.get_lvl() << " " << it->second.get_name() << " " << it->second.get_assignement() << " " << it->second.passed_at << std::endl;

    file.close();
}

// ==> restore an old version of exam
void exam::restore_data(void)
{
    std::ifstream file;
    exam backup;
    std::string name;
    std::string assign;
    std::string time_bef_grade;
    file.open(".system/exam_token/current_token.txt", std::ios::in);
    if (file.is_open())
    {
        file >> backup.start_time;
        file >> backup.end_time;
        file >> backup.exam_number;
        file >> backup.student;
        file >> backup.level;
        file >> assign;
        file >> name;
        file >> backup.level_max;
        file >> time_bef_grade;
        file >> backup.level_per_ex;
        file >> backup.level_per_ex_save;
        file >> backup.using_cheatcode;
        size_t passed_count;
        if (file >> passed_count)
        {
            for (size_t i = 0; i < passed_count; i++)
            {
                int lvl, tries;
                std::string ex_name;
                time_t passed_at;
                if (!(file >> lvl >> ex_name >> tries >> passed_at))
                    break;
                exercise ex(lvl, ex_name, tries, 0);
                ex.passed_at = passed_at;
                backup.lvl_ex.insert(std::pair<int, exercise>(lvl, ex));
            }
        }

        backup.current_ex = new exercise(backup.get_lvl(), name, std::stoi(assign), std::stoi(time_bef_grade));
        backup.backup = 1;
        file.close();
    }
    else
    {
        display->message(Msg::BackupOpenError);
    }

    file.close();

    if (backup.end_time > time(0))
    {
        RestoreView r;
        r.student = backup.student;
        r.exam_number = backup.exam_number;
        r.ex_name = backup.current_ex->get_name();
        r.end_time = backup.end_time;
        std::string answer = display->restorePrompt(r);
        if (answer == "1")
        {
            display->message(Msg::RestoringToken);
            this->start_time = backup.get_start_time();
            this->end_time = backup.get_end_time();
            this->exam_number = backup.get_exam_number();
            this->student = backup.student;
            this->level = backup.get_lvl();
            this->current_ex = backup.current_ex;
            this->backup = backup.backup;
            this->level_max = backup.level_max;
            this->level_per_ex = backup.level_per_ex;
            this->level_per_ex_save = backup.level_per_ex_save;
            this->using_cheatcode = backup.using_cheatcode;
            this->lvl_ex = backup.lvl_ex;
        }
        else
        {
            display->message(Msg::TokenDeleted);
            // delete file and return to menu
            system("rm .system/exam_token/current_token.txt");
            if (!term_screen_active())
                system("clear");
            ask_param();
        }
    }
    else
        ask_param();
}

// ==> Load .settings file into setting_dse bool
void exam::load_settings(void)
{
    std::ifstream file(".system/exam_token/.settings");
    std::string line;
    if (file.is_open())
    {
        bool v;
        bool esd = false;
        int style;
        if (file >> v)
            setting_dse = v;
        if (file >> v)
            setting_dcc = v;
        if (file >> v)
            esd = v;
        if (file >> v)
            setting_anim = v;
        if (file >> v)
            setting_pcd = v;
        // Line 6 is the display style; files without it only know the old esd switch (line 3)
        if (file >> style && style >= 0 && style < DISPLAY_STYLES)
            setting_style = (DisplayStyle)style;
        else
            setting_style = esd ? DisplayStyle::Exact : DisplayStyle::Modern;
        // Line 7 is git submission; older files don't have it (off)
        if (file >> v)
            setting_git = v;
        // Line 8: strict git (no push status)
        if (file >> v)
            setting_git_strict = v;
        file.close();
    }
    set_display();
    if (setting_git && !git_installed())
    {
        setting_git = false;
        display->message(Msg::GitMissing);
    }
}

// ==> Pick the display matching the "Display style" setting
void exam::set_display(void)
{
    delete display;
    display = make_display(setting_style);
}

// ==> Save setting_dse bool into .settings file
void exam::save_settings(void)
{
    std::ofstream file(".system/exam_token/.settings");
    if (file.is_open())
    {
        file << setting_dse << std::endl;
        file << setting_dcc << std::endl;
        file << (setting_style == DisplayStyle::Exact) << std::endl;
        file << setting_anim << std::endl;
        file << setting_pcd << std::endl;
        file << (int)setting_style << std::endl;
        file << setting_git << std::endl;
        file << setting_git_strict << std::endl;
        file.close();
    }
}
