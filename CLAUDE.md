# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

An offline simulator for the 42 exams (Exam Ranks 02–06 and Piscine exams), derived from jcluzet/42_EXAM. A C++ "examshell" binary draws random exercises, and bash testers grade the student's submission against a reference solution. Runs on Linux/WSL only (`launch.sh` refuses anything else; on Windows, build and run `make`/`check-subjects` inside WSL); needs `gcc`, `g++`, `clang`, `make`, and readline headers.

## Commands

```sh
make                 # build .system/a.out and start it via .system/launch.sh
make build           # build only (c++ -std=c++11 -Wall -Wextra -Werror, links -lreadline)
make re              # rebuild from scratch
make check-subjects  # validate every exercise (see below)
make reset           # delete session state: rendu/ subjects/ success/ traces/ .system/grading, saved token
```

Check a single exercise or subset: `bash .system/check_subjects.sh <substring-of-path>` (e.g. `exam_02/first_word`). Env vars `JOBS` (parallelism) and `TIMEOUT` (seconds per tester, default 180). Each result is `OK`, `BROKEN` (reference fails its own tester), `WEAK` (empty submission passes), or `NO_TESTER`.

## Architecture

- **C++ examshell** (`.system/*.cpp`, headers `exam.hpp`/`exercise.hpp`): menus (`menu.cpp`), the `exam` class with prompt loop, levels, cooldown and cheat commands (`exam.cpp`), exercise preparation — copies the subject into `subjects/` and tester files into `.system/grading/` (`exercise.cpp`), grading (`grade_request.cpp` runs `bash .system/grading/tester.sh` and checks for `.system/grading/passed`, saving traces to `traces/` and passing work to `success/`), and save/restore of exam state and settings under `.system/exam_token/` (`data_persistence.cpp`). It shells out heavily via `system()`; all paths are relative to the repo root, so the binary must run from there.
- **Exercises** live at `.subjects/<STUD_PART|PISCINE_PART>/exam_0X/<level>/<name>/` with `attachment/subject.en.txt` (+ any given files), reference `<name>.c`, `main.c` for function exercises, `tester.sh`, and optional `beta` (contributor name → "beta exercise" warning). `check_subjects.sh` finds exercises at exactly depth 4 under `.subjects`.
- **Tester protocol**: `tester.sh` runs from the repo root and calls `.system/auto_correc_program.sh <file> <name> [args...]` (programs) or `.system/auto_correc_main.sh` (functions) once per test case. These compile both reference and `rendu/<name>/<file>`, diff `cat -e` output, and write `.system/grading/traceback` on mismatch/timeout. After each call the tester must move `traceback` to the root and `exit 1` if it exists; at the end it does `touch .system/grading/passed`. Follow the existing tester.sh pattern when adding exercises, and run `check-subjects` on them.

`plans/REMOVED_FEATURES.md` documents features dropped from the upstream project.
