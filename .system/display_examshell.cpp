#include "exam.hpp"

void ExamshellDisplay::showStatus(const ExamView &v)
{
    int current_pts = (int)(((double)v.level + 1) / (double)v.level_max * 100);

    std::cout << "==================================================================" << std::endl;
    std::cout << "You are currently at level " << LIME << v.level << RESET << std::endl;
    if (v.real_mode)
        std::cout << "You are running in " << MAGENTA << "REAL" << RESET << " mode (Your grade will be counted)" << std::endl;
    else
        std::cout << "You are running in " << YELLOW << "TEST" << RESET << " mode (Your grade will not be counted)" << std::endl;
    std::cout << "Your current grade is " << LIME << v.level_per_ex_save * v.level << RESET << "/100" << std::endl;
    std::cout << "Assignments: " << std::endl;

    for (size_t l = 0; l < v.passed.size(); l++)
    {
        const PassedLevel &p = v.passed[l];
        std::cout << "  Level " << LIME << p.level << RESET << ": " << std::endl;
        for (int i = 0; i < p.tries; i++)
        {
            std::cout << "    " << YELLOW << i << RESET << ": " << LIME << p.name << RESET << " for " << (int)v.level_per_ex_save * (p.level + 1) << " potential points (" << RED << "Failure" << RESET << ")" << std::endl;
        }
        std::cout << "    " << YELLOW << p.tries << RESET << ": " << LIME << p.name << RESET << " for " << (int)v.level_per_ex_save * (p.level + 1) << " potential points (" << LIME << "Success" << RESET << ")" << std::endl;
    }

    std::cout << "  Level " << LIME << v.level << RESET << ": " << std::endl;
    if (v.animate)
        rollExercise(v);
    else
    {
        for (int i = 0; i < v.tries; i++)
        {
            std::cout << "    " << YELLOW << i << RESET << ": " << LIME << v.ex_name << RESET << " for " << current_pts << " potential points (" << RED << "Failure" << RESET << ")" << std::endl;
        }
        std::cout << "    " << YELLOW << v.tries << RESET << ": " << LIME << v.ex_name << RESET << " for " << current_pts << " potential points (" << CYAN << "Current" << RESET << ")" << std::endl;
    }

    std::cout << std::endl
              << "Your current assignment is " << LIME << v.ex_name << RESET << " for " << LIME << current_pts << RESET << " potential points" << std::endl;
    std::cout << "It is assignment " << YELLOW << v.tries << RESET << " for level " << LIME << v.level << RESET << std::endl;
    std::cout << "The subject is located at: " << LIME << current_path() << "/subjects/subject.en.txt" << RESET << std::endl;
    std::cout << "You must turn in your files in a subdirectory with the" << std::endl;
    std::cout << "same name as the assignment (" << RED << current_path() << "/rendu/" << v.ex_name << RESET << "/)." << std::endl;
    std::cout << "Here you don't need to use git." << std::endl
              << std::endl;
    std::cout << "The end date for this exam is: " << LIME << std::put_time(std::localtime(&v.end_time), "%d/%m/%Y %H:%M:%S") << RESET << std::endl;
    std::cout << "You have " << LIME << remaining_time(v.end_time) << RESET << " remaining" << std::endl;
    std::cout << std::endl
              << "=============================================================================" << std::endl;
    std::cout << "You can work on your assignment. When you are sure you're done with it," << std::endl;
    std::cout << "use the \"" << LIME << "grademe" << RESET << "\" command to be graded, or \"" << LIME << "help" << RESET << "\" to get some help." << std::endl;
}

Display *make_display(bool exact_examshell)
{
    if (exact_examshell)
        return (new ExamshellDisplay());
    return (new LegacyDisplay());
}
