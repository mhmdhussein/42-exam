#include "exam.hpp"

void exam::fail_ex()
{
    current_ex->up_assignement();
    current_ex->set_time_bef_grade(time(NULL) + current_ex->grade_time() * 60);
    store_data();
}

void exam::success_ex(bool force)
{
    // insert current_ex in lvl_ex
    lvl_ex.insert(std::pair<int, exercise>(current_ex->get_lvl(), *current_ex));
    // insert the success exercise into file .system/exam_token/success_ex
    if (!force)
    {
        std::ofstream file;
        file.open("success/success_ex", std::ios::app);
        file << current_ex->get_name() << std::endl;
    }
    GradeResult result;
    result.passed = true;
    display->showResult(result);
    up_lvl();
    display->waitEnter();
    level_per_ex += level_per_ex_save;
    changex = 0;
    backup = 0;
    if (!force)
    {
        if (file_exists("rendu/"))
        {
            if (!file_exists("success"))
                system("mkdir success");
            system("cp -r rendu/* success/ 2> /dev/null");
        }
    }
    if (level_per_ex > 100)
        end_exam();
    start_new_ex();
}

void exam::end_exam()
{
    remove(".system/exam_token/current_token.txt");
    ExamSummary summary;
    summary.exam_number = exam_number;
    summary.cheat_count = using_cheatcode;
    summary.username = username;
    display->showEnd(summary);
    exit(0);
}

// ==> GradeMe function call by entering `grademe` into prompt
void exam::grademe(void)
{
    std::string beta_author;
    if (file_exists(".system/grading/beta"))
    {
        std::ifstream file;
        file.open(".system/grading/beta");
        std::getline(file, beta_author);
    }

    if (display->confirmGrade(beta_author))
    {
        if (current_ex->time_bef_grade > time(NULL) && waiting_time)
        {
            display->cooldownError(current_ex->time_bef_grade - time(NULL));
            return;
        }
        display->message(Msg::GradeRequestSent);
        grade_request(0);
    }
    else
    {
        display->message(Msg::GradeAborted);
    }
}

// ==> Run the exercise's custom tester.sh, or the shared runner on its cases file:
// 1 on pass, 0 on fail (trace moved to traces/), -1 if there is no tester
int exam::run_tester(void)
{
    std::string cmd_system_call;
    if (file_exists(".system/grading/tester.sh"))
        cmd_system_call = "bash .system/grading/tester.sh";
    else if (file_exists(".system/grading/cases"))
        cmd_system_call = "bash .system/run_tests.sh " + current_ex->get_name();
    else
    {
        display->message(Msg::NoTester);
        return (-1);
    }

    remove(".system/grading/passed");
    system(cmd_system_call.c_str());

    if (file_exists(".system/grading/passed"))
        return (1);

    // if there is a traceback file, create a folder traces and copy the file to it with the good name
    GradeResult result;
    result.passed = false;
    if (file_exists("traceback"))
    {
        system("mkdir -p traces");
        std::string trace_name = std::to_string(level) + "-" + std::to_string(current_ex->get_assignement()) + "_" + current_ex->get_name() + ".trace";
        std::string cmd_system_call = "mv traceback traces/" + trace_name;
        system(cmd_system_call.c_str());
        result.trace_path = current_path() + "/traces/" + trace_name;
    }
    display->showResult(result);
    return (0);
}

// ==> Function that call the bash grade system
void exam::grade_request(bool i)
{
    if (!i)
        display->gradingStarted();

    int result = run_tester();
    if (result == -1)
        return;
    if (result)
    {
        success_ex(0);
    }
    else
    {
        fail_ex();
        display->waitEnter();
        show_status();
    }
}
