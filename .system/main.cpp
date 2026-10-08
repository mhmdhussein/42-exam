#include "exam.hpp"

// ==> Shell prompt
void exam::exam_prompt(void)
{
    std::string input;

    while (1)
    {
        char *line = readline("\e[93mexamshell\e[0m> ");
        if (line == NULL)
            sigd();
        input = line;
        while (input.back() == ' ')
            input.pop_back();
        while (input.front() == ' ')
            input.erase(input.begin());
        if (input.empty())
            continue;
        add_history(input.c_str());
        if ((input == "remove_grade_time" || input == "new_ex" || input == "force_success" || input == "gradenow") && !setting_dcc)
            std::cout << " ❌ Cheat commands are currently disabled, use " << LIME << BOLD << "settings" << RESET << " command." << std::endl;
        else if (input == "finish" || input == "exit" || input == "quit")
        {
            std::cout << "Are you sure you want to" << RED << " exit" << RESET << " the exam?" << std::endl;
            std::cout << "All your progress will be " << RED << "lost" << RESET << "." << std::endl;
            std::cout << "Type " << LIME << BOLD << "yes" << RESET << " to confirm." << std::endl;
            // read input
            std::string input;
            std::getline(std::cin, input);
            if (input == "yes")
            {
                if (std::ifstream(".system/exam_token/current_token.txt"))
                    remove(".system/exam_token/current_token.txt");
                exit(0);
            }
            else
                std::cout << " ** Abort ** " << std::endl;
        }
        else if (input == "settings")
        {
            changex = 1;
            settings_menu();
            if (setting_esd)
                info_examshell();
            else
                info();
        }
        else if (input == "grademe")
        {
            grademe();
        }
        else if (input == "status")
        {
            changex = 1;
            if (setting_esd)
                info_examshell();
            else
                info();
        }
        else if (input == "new_ex")
        {
            change_ex();
            using_cheatcode++;
        }
        else if (input == "force_success")
        {
            using_cheatcode++;
            success_ex(1);
        }
        else if (input == "help")
        {
            exam_help();
        }
        else if (input == "remove_grade_time")
        {
            std::cout << "Time between grading is now removed for this exam" << std::endl;
            waiting_time = false;
            using_cheatcode++;
        }
        else if (input == "gradenow")
        {
            using_cheatcode++;
            grade_request(1);
        }
        else if (input == "")
            std::cout << REMOVE_LINE;
        else if (input != "")
            std::cout << "           **Unknown command**     type " << LIME << "help" << RESET << " for more help" << std::endl;
        // info();
    }
}
// jo je taime

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
    if (setting_esd)
        info_examshell();
    else
        info();
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