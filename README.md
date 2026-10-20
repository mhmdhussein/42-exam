# 42 Exam Simulator

A private, offline simulator for the 42 exams (Exam Ranks 02–06 and the Piscine exams).
Exercises are drawn at random, graded locally against a reference solution, and the wait
before you can grade again grows with each failed attempt, like the real examshell.

> Not made by 42. Not identical to the real exams.

## Requirements

Linux or WSL with `gcc`, `g++`, `clang`, `make` and the readline headers
(`libreadline-dev` on Debian/Ubuntu, `readline-devel` on Fedora). `valgrind` is optional: without
it, exercises that check for leaks skip that check.

### Windows (WSL)

The simulator doesn't run on Windows itself; use it inside WSL.

1. Install WSL with Ubuntu, from PowerShell as administrator, then restart and open **Ubuntu**:

   ```powershell
   wsl --install -d Ubuntu
   ```

2. Install the tools, in Ubuntu:

   ```sh
   sudo apt update
   sudo apt install gcc g++ clang make libreadline-dev valgrind git
   ```

3. Clone the repo **in your Linux home folder**, not under `/mnt/c/...`:

   ```sh
   cd ~
   git clone https://github.com/mhmdhussein/42-exam.git 42-EXAM
   cd 42-EXAM
   make
   ```

   Files under `/mnt/c` are on the Windows drive: every file access goes through a slow bridge, so
   compiling and grading take much longer and testers that wait on timing (like `mini_serv`) can fail
   for no reason. File permissions there are also emulated. `make` and `make check-subjects` print a
   warning when they run from `/mnt/`.

4. To edit with VS Code, install the **WSL** extension and run `code .` from the repo folder in
   Ubuntu. Your Windows files stay reachable from Ubuntu at `/mnt/c/Users/<you>/`.

**`\r` errors** (`$'\r': command not found`, `bad interpreter`): the repo was checked out with Windows
line endings, usually by cloning with Windows git. `.gitattributes` forces LF, so re-checking out the
files fixes it:

```sh
git rm --cached -r . && git reset --hard
```

This throws away uncommitted changes. Or clone again inside WSL as in step 3.

## Usage

```sh
make        # build and start
make build  # build only
make re     # rebuild from scratch
make fclean # remove build output
make check-subjects # check every exercise: the reference must pass its tester, an empty submission and every wrong/ solution must fail
make reset  # delete session files: rendu/, subjects/, success/, traces/ and the saved exam (settings are kept)
```

The version shown on the login screen comes from the `VERSION` file; `make` rebuilds when it changes.

Choose Piscine or Student part, pick an exam and confirm. Your subject goes in
`subjects/subject.en.txt`. Write your solution in `rendu/<exercise>/`, then type `grademe` at the
`examshell>` prompt. If grading fails, the trace is saved in `traces/`.

In the default **Modern** style, the menus take ↑/↓ (or `j`/`k`) and Enter, Esc or `q` to go back
(typing an option's number still works, and without a terminal you type the number as before). The
prompt shows the time left (`examshell [2:41] ❯`), yellow under 30 minutes and red under 10.
`status` shows your level, points, time left and when you can grade again. Grading shows a spinner,
then a card: on failure, the first failing test with its expected and actual output; on success, your
new level and points. When the exam ends you get a summary with tries and time spent per level.
Set `NO_COLOR=1` for no colours, or `EXAMSHELL_ASCII=1` for plain ASCII boxes and symbols.

### Commands

| Command | |
|---|---|
| `help` | List commands |
| `status` | Show exam status |
| `grademe` | Grade the current exercise |
| `settings` | Open the settings menu |
| `finish` | Leave the exam and go back to the main menu (progress is lost) |
| `exit` / `quit` | Quit the simulator (progress is lost) |

Cheat commands, available only when "Enable cheat commands" is on. Each one counts as a cheat in the end screen:

| Command | |
|---|---|
| `gradenow` | Grade right away, with no cooldown |
| `remove_grade_time` | Turn off the cooldown for this exam |
| `force_success` | Mark the current exercise as passed |
| `new_ex` | Draw a new exercise at the same level |

### Settings

Saved in `.system/exam_token/.settings`.

1. Enable exercises you already passed
2. Enable cheat commands
3. Display style: **Modern** (default: boxes, arrow-key menus, result cards), **Legacy** (the original look) or **Exact examshell** (the original look, with status screens worded like the real 42 examshell). Press 3 to cycle (in Modern: ←/→ on the row); it applies as soon as you leave the menu, also in the middle of an exam. Settings files from older versions load as Exact examshell if that switch was on, otherwise Modern
4. Login animation: show the fake examshell login when an exam starts
5. Cooldown in Practice mode (default off): after a failed `grademe` in Practice, wait like in an exam; the wait grows with each failure and resets when you switch exercise

## Practice mode

Choose **3 Practice** in the main menu to train on any exercise you pick, with no timer, no levels
and no grade. If a saved exam exists, you are asked first whether to clear it or cancel and go back.

The picker is a live search bar: type to filter, ↑/↓ (or Ctrl+P/Ctrl+N) to move, Enter to select,
Esc to go back. Words are space-separated and must all match:

| Word | Matches |
|---|---|
| `str` | name contains `str` (case-insensitive) |
| `part:stud` / `part:piscine` | Student or Piscine part |
| `exam:02` | exam number |
| `lvl:1` | level |
| `passed` / `-passed` | passed / not passed yet |

When the input is not a terminal, the picker falls back to a numbered list.

At the `practice>` prompt: `grademe`, `pick` (search again), `random` (another exercise of the same
exam and level), `status`, `help` and `exit` (back to the menu). Switching exercise keeps `rendu/`.
After a pass, choose `1` retry, `2` pick another, `3` random from the same level or `0` exit to the menu
(in the Modern style this is an arrow-key menu under the pass card, and the prompt shows the exercise:
`practice [rev_print] ❯`).
Passes are appended to `success/practice_ex` (`<name> <PART>/exam_0X/<level> <timestamp>`);
`success/success_ex` is not touched, so practice does not hide exercises from real exams.

## Exercise format

Each exercise is a folder `.subjects/<STUD_PART|PISCINE_PART>/exam_0X/<level>/<name>/` with:

- `attachment/subject.en.txt`, plus any other files given to the student
- `<name>.c`: the reference solution, assumed correct
- `main.c`: only for function exercises
- `cases`: the test cases, one per line in shell-quoted form, graded by the shared runner
  `.system/run_tests.sh`. It compiles the reference and `rendu/<name>/` with `-Wall -Wextra -Werror`,
  runs both on every case and compares the output.
- `allowed` (optional): the subject's allowed functions; any other function the student calls fails.
- `config` (optional): per-exercise settings (timeout, `BUFFER_SIZE` values, exit code / stderr / leak checks).
- `inputs/` (optional): files fed on stdin by `stdin:` cases.
- `wrong/`: known-wrong solutions, each named after its bug. `make check-subjects` reports `LEAKY` if the
  tester lets one pass.
- `tester.sh`: only for exercises the runner can't handle (servers, several processes...). It writes the
  same trace and the `.system/grading/passed` marker.
- `beta` (optional): name of the contributor. Grading then shows a "beta exercise" warning.

The rules every tester must follow are in [docs/TESTER_GUIDE.md](docs/TESTER_GUIDE.md).

## License

Apache 2.0, see [LICENSE](LICENSE). Derived from [jcluzet/42_EXAM](https://github.com/jcluzet/42_EXAM).
Features removed from the original are documented in [plans/REMOVED_FEATURES.md](plans/REMOVED_FEATURES.md).
