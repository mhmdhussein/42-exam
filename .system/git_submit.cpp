#include "exam.hpp"

// Git submission mode: rendu/ is a git repository pushing to the bare
// .system/remote.git, and grademe grades only what was pushed there.

bool git_installed(void)
{
    return (system("command -v git > /dev/null 2>&1") == 0);
}

// ==> Output of a shell command, trailing newline removed
static std::string capture(const std::string &cmd)
{
    std::string out;
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return (out);
    char buf[256];
    while (fgets(buf, sizeof(buf), pipe))
        out += buf;
    pclose(pipe);
    while (!out.empty() && out[out.size() - 1] == '\n')
        out.erase(out.size() - 1);
    return (out);
}

// ==> Fresh bare remote and rendu/ repository; the same branch name on both sides
// so nothing depends on the user's init.defaultBranch. The identity is local only.
void exam::git_init(void)
{
    system("rm -rf .system/remote.git");
    system("git init -q --bare -b master .system/remote.git > /dev/null 2>&1");
    system("cp .system/pre-receive .system/remote.git/hooks/pre-receive && chmod +x .system/remote.git/hooks/pre-receive");
    system("mkdir -p rendu && git init -q -b master rendu > /dev/null 2>&1");
    system("git -C rendu remote add origin ../.system/remote.git");
    // a plain `git push` works, as on the exam machines
    system("git -C rendu config branch.master.remote origin && git -C rendu config branch.master.merge refs/heads/master");
    // under WSL, files on the Windows drive all look executable: ignore modes, so the
    // push status agrees with a git run from Windows
    system("git -C rendu config core.filemode false");
    // same for line endings: Windows git commits LF but leaves CRLF files on disk, which
    // WSL git would see as changed; normalise to LF on both sides
    system("git -C rendu config core.autocrlf input");
    std::string cmd = "git -C rendu config user.name '" + username + "' && git -C rendu config user.email '" + username + "@student.42.fr'";
    system(cmd.c_str());
}

// ==> Export what was pushed to .system/grading/rendu/; an empty remote leaves it empty
void exam::git_snapshot(void)
{
    system("rm -rf .system/grading/rendu && mkdir -p .system/grading/rendu");
    system("git --git-dir=.system/remote.git archive HEAD 2> /dev/null | tar -x -C .system/grading/rendu 2> /dev/null");
}

// ==> "everything pushed", "2 files not committed, 1 commit not pushed", ...
std::string exam::git_push_status(void)
{
    if (!file_exists("rendu/.git"))
        return ("no git repository in rendu/");
    int changed = 0;
    std::istringstream lines(capture("git -C rendu status --porcelain 2> /dev/null"));
    std::string line;
    while (std::getline(lines, line))
        changed++;
    // commits not on any remote-tracking branch, which a push updates
    int unpushed = atoi(capture("git -C rendu rev-list --count HEAD --not --remotes 2> /dev/null").c_str());
    std::string status;
    if (changed)
        status = std::to_string(changed) + (changed == 1 ? " file" : " files") + " not committed";
    if (unpushed)
        status += (status.empty() ? "" : ", ") + std::to_string(unpushed) + (unpushed == 1 ? " commit" : " commits") + " not pushed";
    if (!status.empty())
        return (status);
    if (system("git --git-dir=.system/remote.git rev-parse -q --verify HEAD > /dev/null 2>&1") != 0)
        return ("nothing pushed yet");
    return ("everything pushed");
}
