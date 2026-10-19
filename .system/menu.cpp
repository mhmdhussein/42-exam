#include "exam.hpp"

// ==> First menu
int exam::stud_or_swim(void)
{
    std::string choice = "";
    while (choice != "1" && choice != "2")
    {
        choice = display->startMenu(choice == "-1");
        if (choice == "3")
            practice_mode();
        else if (choice == "4")
            settings_menu();
        else if (choice == "q")
        {
            system("clear");
            exit(0);
        }
        else if (choice != "1" && choice != "2" )
            choice = "-1";
    }
    return (atoi(choice.c_str()));
}

// ==> Setting MENU
void exam::settings_menu(void)
{
    load_settings();
    std::string input = "";
    while (input != "0")
    {
        SettingsView s;
        s.dse = setting_dse;
        s.dcc = setting_dcc;
        s.style = setting_style;
        s.anim = setting_anim;
        s.pcd = setting_pcd;
        input = display->settingsMenu(s);
        if (input == "1")
            setting_dse = !setting_dse;
        if (input == "2")
            setting_dcc = !setting_dcc;
        if (input == "3")
            setting_style = (DisplayStyle)(((int)setting_style + 1) % 3);
        if (input == "-3")
            setting_style = (DisplayStyle)(((int)setting_style + 2) % 3);
        if (input == "4")
            setting_anim = !setting_anim;
        if (input == "5")
            setting_pcd = !setting_pcd;
    }

    display->message(Msg::SettingsSaved);
    save_settings();
    set_display();
}

// ==> Menu for the student part
int exam::stud_menu(void)
{
    std::string choice = "-2";
    while (choice == "-1" || choice == "-2")
    {
        choice = display->studMenu(choice == "-1");
        if (choice != "2" && choice != "3" && choice != "4" && choice != "5" && choice != "6" && choice != "0")
            choice = "-1";
    }
    return (atoi(choice.c_str()));
}

// ==> Menu for the piscine part
int exam::piscine_menu(void)
{
    std::string choice = "-2";
    while (choice == "-1" || choice == "-2")
    {
        choice = display->piscineMenu(choice == "-1");
        if (choice != "1" && choice != "2" && choice != "3" && choice != "4" && choice != "0")
            choice = "-1";
    }
    return (atoi(choice.c_str()));
}
