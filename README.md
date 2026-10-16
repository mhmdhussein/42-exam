# 42 Exam Simulator

A private, offline simulator for the 42 exams (Exam Ranks 02–06 and the Piscine exams).
Exercises are drawn at random, graded locally against a reference solution, and the wait
before you can grade again grows with each failed attempt, like the real examshell.

> Not made by 42. Not identical to the real exams.

## Requirements

Linux or WSL with `gcc`, `g++`, `clang`, `make` and the readline headers
(`libreadline-dev` on Debian/Ubuntu, `readline-devel` on Fedora).

## Usage

```sh
make        # build and start
make build  # build only
make re     # rebuild from scratch
make fclean # remove build output
make check-subjects # check every exercise: the reference must pass its tester, an empty submission must fail
make reset  # delete session files: rendu/, subjects/, success/, traces/ and the saved exam (settings are kept)
```

Choose Piscine or Student part, pick an exam and confirm. Your subject goes in
`subjects/subject.en.txt`. Write your solution in `rendu/<exercise>/`, then type `grademe` at the
`examshell>` prompt. If grading fails, the trace is saved in `traces/`.

### Commands

| Command | |
|---|---|
| `help` | List commands |
| `status` | Show exam status |
| `grademe` | Grade the current exercise |
| `settings` | Open the settings menu |
| `finish` | Quit the exam (progress is lost) |

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
3. Exact examshell display: use status screens worded like the real 42 examshell
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
After a pass, choose `1` retry, `2` pick another, `3` random from the same level or `0` exit to the menu.
Passes are appended to `success/practice_ex` (`<name> <PART>/exam_0X/<level> <timestamp>`);
`success/success_ex` is not touched, so practice does not hide exercises from real exams.

## Exercise format

Each exercise is a folder `.subjects/<STUD_PART|PISCINE_PART>/exam_0X/<level>/<name>/` with:

- `attachment/subject.en.txt`, plus any other files given to the student
- `<name>.c`: the reference solution, assumed correct
- `main.c`: only for function exercises
- `tester.sh`: calls `.system/auto_correc_program.sh` (programs) or `.system/auto_correc_main.sh`
  (functions) once per test case. After each call, if `.system/grading/traceback` exists, it moves the
  file to the root and exits 1. At the end it runs `touch .system/grading/passed`.
  Exercises that need stdin, several files, C++ or a custom test `main` (STUD exams 03–05) have
  self-contained testers that build and compare reference and student output themselves, with the
  same `traceback` / `passed` contract.
- `beta` (optional): name of the contributor. Grading then shows a "beta exercise" warning.

The rules every tester must follow are in [docs/TESTER_GUIDE.md](docs/TESTER_GUIDE.md).

## License

Apache 2.0, see [LICENSE](LICENSE). Derived from [jcluzet/42_EXAM](https://github.com/jcluzet/42_EXAM).
Features removed from the original are documented in [plans/REMOVED_FEATURES.md](plans/REMOVED_FEATURES.md).
