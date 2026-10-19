#include "exam.hpp"

void exam::fail_ex()
{
    current_ex->up_assignement();
    current_ex->set_time_bef_grade(time(NULL) + current_ex->grade_time() * 60);
    store_data();
}

// ==> Grade after `passed` levels
int exam::grade_at(int passed)
{
    if (passed >= level_max)
        return (100);
    return (passed * level_per_ex_save);
}

void exam::success_ex(bool force)
{
    // insert current_ex in lvl_ex
    current_ex->passed_at = time(NULL);
    lvl_ex.insert(std::pair<int, exercise>(current_ex->get_lvl(), *current_ex));
    // insert the success exercise into file .system/exam_token/success_ex
    if (!force)
    {
        std::ofstream file;
        file.open("success/success_ex", std::ios::app);
        file << current_ex->get_name() << std::endl;
    }
    GradeResult result;
    if (!force)
        result = last_result;
    result.passed = true;
    result.ex_name = current_ex->get_name();
    result.tries = current_ex->get_assignement() + 1;
    result.level = level + 1;
    result.level_max = level_max;
    result.grade = grade_at(level + 1);
    result.gained = result.grade - grade_at(level);
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
    summary.student = student;
    summary.exam_number = exam_number;
    summary.grade = grade_at(level);
    summary.time_used = time(NULL) - start_time;
    summary.time_total = end_time - start_time;
    summary.cheat_count = using_cheatcode;
    summary.username = username;
    // time on a level runs from the previous level's pass (or the exam start) to its own
    time_t since = start_time;
    for (int l = 0; l < level_max && l < level; l++)
    {
        SummaryRow row;
        row.level = l;
        row.tries = -1;
        row.seconds = -1;
        std::map<int, exercise>::iterator it = lvl_ex.find(l);
        if (it != lvl_ex.end())
        {
            row.name = it->second.get_name();
            row.tries = it->second.get_assignement() + 1;
            if (it->second.passed_at && since)
                row.seconds = it->second.passed_at - since;
            since = it->second.passed_at;
        }
        else
            since = 0;
        summary.rows.push_back(row);
    }
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

// ==> Lines after lines[i] up to the next one starting with `until`; i ends on that line
static std::vector<std::string> section(const std::vector<std::string> &lines, size_t &i, const std::string &until)
{
    std::vector<std::string> out;
    for (i++; i < lines.size() && lines[i].compare(0, until.size(), until) != 0; i++)
        out.push_back(lines[i]);
    return (out);
}

// ==> Fill the failure fields of r from a trace in the docs/TESTER_GUIDE.md format;
// the first lines are always kept in r.raw for traces in another format
static void parse_trace(const std::string &path, GradeResult &r)
{
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    for (size_t i = 0; i < lines.size() && i < 15; i++)
        r.raw.push_back(lines[i]);
    if (lines.empty())
        return;

    const std::string head_case = "=== CASE ", head_build = "=== FAILED: ", tail = " ===";
    const std::string &first = lines[0];
    if (first.compare(0, head_case.size(), head_case) == 0)
    {
        // === CASE 3/8 FAILED: diff ===
        size_t slash = first.find('/'), colon = first.find(" FAILED: ");
        if (slash == std::string::npos || colon == std::string::npos || first.size() < colon + 9 + tail.size())
            return;
        r.case_no = atoi(first.c_str() + head_case.size());
        r.case_total = atoi(first.c_str() + slash + 1);
        r.reason = first.substr(colon + 9, first.size() - colon - 9 - tail.size());
        for (size_t i = 1; i < lines.size(); i++)
        {
            if (lines[i].compare(0, 9, "COMMAND: ") == 0)
                r.command = lines[i].substr(9);
            else if (lines[i].compare(0, 9, "STDIN:   ") == 0)
                r.stdin_label = lines[i].substr(9);
            else if (lines[i] == "EXPECTED (cat -e):")
            {
                r.expected = section(lines, i, "GOT (cat -e):");
                r.got = section(lines, i, "=== END CASE");
            }
        }
        if (r.stdin_label == "(none)")
            r.stdin_label.clear();
        r.structured = r.case_no > 0;
    }
    else if (first.compare(0, head_build.size(), head_build) == 0 && first.size() >= head_build.size() + tail.size())
    {
        // === FAILED: compile error ===
        r.reason = first.substr(head_build.size(), first.size() - head_build.size() - tail.size());
        for (size_t i = 1; i < lines.size(); i++)
            if (lines[i] == "GOT:")
                r.got = section(lines, i, "=== END ===");
        r.structured = true;
    }
}

// ==> Run the exercise's custom tester.sh, or the shared runner on its cases file:
// 1 on pass, 0 on fail (trace moved to traces/), -1 if there is no tester.
// Fills last_result; the caller shows it.
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
    remove(".system/grading/progress");
    remove(".system/grading/result");
    display->testerStarted(current_ex->get_name());
    system(cmd_system_call.c_str());
    display->testerFinished();

    last_result = GradeResult();
    last_result.ex_name = current_ex->get_name();
    last_result.tries = current_ex->get_assignement() + 1;
    if (file_exists(".system/grading/passed"))
    {
        last_result.passed = true;
        // the shared runner leaves "tests=N"
        std::ifstream file(".system/grading/result");
        std::string line;
        while (std::getline(file, line))
            if (line.compare(0, 6, "tests=") == 0)
                last_result.tests = atoi(line.c_str() + 6);
        return (1);
    }

    // if there is a traceback file, create a folder traces and copy the file to it with the good name
    if (file_exists("traceback"))
    {
        system("mkdir -p traces");
        std::string trace_name = std::to_string(level) + "-" + std::to_string(current_ex->get_assignement()) + "_" + current_ex->get_name() + ".trace";
        std::string cmd_system_call = "mv traceback traces/" + trace_name;
        system(cmd_system_call.c_str());
        last_result.trace_path = current_path() + "/traces/" + trace_name;
        last_result.trace_rel = "traces/" + trace_name;
        parse_trace(last_result.trace_rel, last_result);
    }
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
        if (waiting_time && current_ex->time_bef_grade > time(NULL))
            last_result.retry_at = current_ex->time_bef_grade;
        display->showResult(last_result);
        display->waitEnter();
        show_status();
    }
}
