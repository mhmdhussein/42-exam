# Cleanup & Fork Plan

Goal: turn this offline copy of 42_EXAM (grademe.fr is gone) into a clean, private,
self-maintained exam simulator. Not for distribution.

The fork starts as a **brand-new git repository**: no old commits, history, remotes or tags are carried over,
and removed code is deleted outright, with no archive. [REMOVED_FEATURES.md](REMOVED_FEATURES.md) is the only record:
a written description of each removed feature, enough to rebuild it if ever wanted.

---

## Phase 0 — Clean slate

No need to keep any previous session state or history.

- [ ] Discard all local session leftovers:
  - `subjects/`, `rendu/`, `success/`, `traceback`, `test`
  - `.system/grading/`, `.system/exam_token/.settings`, `.system/exam_token/current_token.txt`
  - `.system/unique_id.txt`, `.system/acceptCGV`, `.system/.devmake.err`, `.system/a.out`
- [ ] Restore the broken `union` exercise: `git checkout -- .subjects` and delete the stray
      `.subjects/STUD_PART/exam_02/1/union/attachement/`.
- [ ] Add a `.gitignore` covering all runtime files listed above so they never pollute the repo again.
- [ ] Start a fresh repository:
      `rm -rf .git` → `git init` → `git add .` → first commit ("Initial import").
      No remote, no old commits, no tags. Old upstream history is gone for good.
- [ ] Commit per phase on `main` (or short branches if preferred).

## Phase 1 — Remove owner / online features

All of these are documented in REMOVED_FEATURES.md before deletion.

| Item | Location | Action |
|---|---|---|
| Telemetry (`data_sender.sh` POSTs to user.grademe.fr) | `data_sender.sh`; calls in `exam.cpp`, `grade_request.cpp`, `main.cpp`, `menu.cpp` | Delete script + all calls |
| VIP list download (`curl user.grademe.fr/vip_list`) | `exam.cpp` constructor | Delete; drop `vip` flag |
| VIP-only UI (`infovip`, VIP help lines) | `menu.cpp`, `main.cpp`, `grade_request.cpp` | Delete; keep the *exact examshell display* idea if wanted |
| `gradenow` (VIP instant grading) | `main.cpp` | Keep, ungated (or a setting) |
| Unique ID / `LOGNAMELOG42EXAM` | `exam.cpp: generate_unique_id`, `menu.cpp`, `launch.sh` | Delete (telemetry only) |
| CGV acceptance | `main.cpp: CGVAcceptation`, `CGV.txt` | Delete |
| Auto-update (`ping google.com` + `git pull`) | `launch.sh` | Delete |
| `repo_git` command, GitHub open, QR code | `main.cpp`, `grade_request.cpp: end_exam`, `.system/qrcode` | Delete |
| Branding / funding | `README.md`, `CONTRIBUTING.md`, `.github/FUNDING.yml`, 42 headers | Rewrite as own; keep `LICENSE` (Apache 2.0) |
| Fake login animation (`connexion()`) | `menu.cpp` | Optional — keep as a setting or remove |

## Phase 2 — Fixes

**Platform / build**
- [ ] Target Linux / WSL only (scripts rely on `/proc`, `ps -p`, `gcc`).
- [ ] Replace readline `sudo apt-get`/`yum` auto-install with a clear error message.
- [ ] Real Makefile build (`-Wall -Wextra -Werror`) instead of compiling inside `launch.sh`; fix warnings.

**Grader**
- [ ] Compile student code with `-Wall -Wextra -Werror` like the real exam.
- [ ] Timeout / infinite-loop detection for every rank, shorter than 20 s.
- [ ] Optional leak & crash checks (valgrind or `-fsanitize=address`).
- [ ] Compare stderr and exit codes, not only stdout.
- [ ] Quote paths properly in `system()` strings.

**Behaviour**
- [ ] Make fake "server" wait and grading cooldown configurable / disableable.
- [ ] Review: `rostring` disabling `srand`, empty copy constructor, `level_per_ex > 100` end condition.

## Phase 3 — Update exam content

Current content: Rank 02 (levels 0–3 + `need_correction`), Rank 03 (1 level), Rank 04 (1),
Rank 05 (3), Rank 06 (1), Piscine exams 01–04.

- [ ] Gather the current official structure per rank (exercise lists, levels, question count, time limits).
- [ ] Restructure `.subjects/` levels; add/remove exercises (subject, reference solution, `main.c`, `tester.sh`).
- [ ] Update `set_max_lvl` / `set_max_time` in `exam.cpp`.
- [x] Resolve `exam_02/need_correction` (verify or delete). Deleted: 5 of its 8 exercises were older copies of graded
      exercises in `exam_02/3`; the other 3 (ft_list_foreach, ft_list_remove_if, sort_int_tab) had no tester.
- [x] Add `make check-subjects`: run every reference solution through its own `tester.sh` to catch broken exercises.
      Also requires an empty submission to fail. Open: `exam_06/mini_serv` has no reference solution; 49 exercises
      (Piscine 03 levels 2–3, Piscine 04) have no tester.

## Phase 4 — Own improvements (optional)

- Practice mode: pick a specific exercise.
- Stats / history (pass/fail per exercise, time spent).
- Refactor the monolithic `exam` class and `system()`-driven logic.

---

**Order:** 0 → 1 → Phase 2 platform/build → 3 → rest of 2 → 4.
