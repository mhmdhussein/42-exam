# Removed Features

Descriptions of features from the original 42_EXAM (by jcluzet, grademe.fr) that are removed or changed in this fork.
The original code and git history are deleted for good. This file is the only record, written so that any feature
can be rebuilt from its description if it's ever wanted again.

Status values: `planned` → `removed` / `changed` / `kept`.

---

## Online / telemetry

### 1. Usage telemetry — `removed`
- **What it was:** a shell script (`.system/data_sender.sh`) that sent a `curl -X POST` to `https://user.grademe.fr/exam.php`
  with four form fields: `user` (an anonymised ID, see #2), `using` (an event string), `os` (`uname`) and `time`.
  It failed silently when offline.
- **When it fired:** the C++ program called it via `system()` on these events:
  - choosing an exam (`choose_examrank0N` for student ranks, `choose_examweek0N` for piscine)
  - failing an exercise (exercise name, level, attempt number)
  - passing an exercise, normally or via `force_success` (tagged as cheat)
  - finishing an exam, normally or with the number of cheat commands used
  - using `repo_git` or `remove_grade_time`
  - leaving the settings menu (with the setting values)
- **Revive idea:** a local, offline stats log (see CLEANUP Phase 4) could reuse the same event list.

### 2. Anonymous user ID — `removed`
- **What it was:** on first run, generated a random ID and stored it in `.system/unique_id.txt`, then put it in the
  `LOGNAMELOG42EXAM` environment variable, which the telemetry used as the user name. `launch.sh` first set that
  variable to `$LOGNAME`; a setting decided whether the real login or the anonymous ID was sent.
- Only existed for telemetry. Settings menu entry 3 was "Anonymise data sending to LOG" (`setting_an`, third value
  in `.system/exam_token/.settings`), which also showed the name currently being sent.

### 3. VIP system — `changed`
- **What it was:** at startup the program downloaded `https://user.grademe.fr/vip_list` (a list of logins of GitHub
  sponsors and contributors). If the current `$USER` was on it, VIP mode was turned on. VIPs got:
  - **Exact examshell display:** status/info screens styled to match the real 42 examshell, instead of the normal 42_EXAM look.
  - **`gradenow` command:** grade immediately, skipping the cooldown and the fake "server" wait.
  - Extra lines in `help` listing the cheat commands.
- The start menu showed "You are a VIP user" for VIPs and "Made by jcluzet" for everyone else.
- **Change:** the download and the `vip` flag are gone. `gradenow` is now a normal cheat command (needs "Enable cheat
  commands", counts as a cheat). The examshell display (`info_examshell()`, formerly `infovip()`) is setting 3,
  "Exact examshell display".

### 4. Auto-update on launch — `removed`
- **What it was:** `launch.sh` pinged `google.com` behind a spinner ("Checking server availability"). If online, it ran
  `git pull` in the background and printed "You have the last version v2.1"; if offline, it printed "Local launch".

### 5. Readline auto-install — `changed`
- **What it was:** `launch.sh` compiled a tiny test program (`checkreadline.cpp`) to see whether the readline library
  was available. If not, it ran `sudo apt-get install libreadline-dev`, then `sudo yum install readline` as a fallback.
- **Change:** replaced by an error message telling you what to install.

### 11. Build inside `launch.sh` — `changed`
- **What it was:** `launch.sh` compiled all sources with `g++ ... -lreadline` in the background behind a spinner, printing
  the error output (and asking for a GitHub report) on failure. It also prompted for a login and saved it in `.system/.env`
  when `$USER` was unset, and `make grade`/`make gradejustinstall` were variants that skipped the screen clear / version check.
- **Change:** the Makefile builds with `-Wall -Wextra -Werror` into `.system/obj/`; `launch.sh` only checks the platform and
  tools, then runs the binary. `$USER` falls back to `whoami`. `gradejustinstall` is gone; `grade` is an alias of `all`.

## Legal / branding

### 6. Terms (CGV) acceptance — `removed`
- **What it was:** on first launch, showed the terms of use from `.system/CGV.txt` and asked you to accept them.
  Acceptance was remembered by creating `.system/acceptCGV`. You had to type `agree`; anything else exited.

### 7. "Star the repo" screen — `removed`
- **What it was:** at the end of an exam, printed "Love 42_EXAM? Scan this QRCode to star Github repository", printed an
  ASCII QR code (`.system/qrcode`), then asked "Opening Github repository? (y/n)" and opened the GitHub page in the browser.

### 8. `repo_git` command — `removed`
- **What it was:** a prompt command that opened `http://git.grademe.fr` in the browser (and sent a telemetry event).

### 9. Docs, funding, headers — `changed`
- **README:** grademe.fr promotion, cheat-command docs, VIP/sponsor section, a one-command online installer, and a
  GDPR (RGPD) notice about the telemetry (also shown on the exam explanation screen, now removed). Rewritten for the fork.
- **CONTRIBUTING.md:** how to add exercises. Its content (below) is kept in the fork's own docs.
- **`.github/FUNDING.yml`:** GitHub sponsors link.
- **Source headers:** 42-style ASCII headers naming the original authors.
- **`LICENSE` (Apache 2.0):** kept.

## Optional / UX

### 10. Fake examshell login — `kept`
- **What it was:** after choosing an exam, the screen cleared and typed out "examshell" letter by letter, then
  "ExamShell v2.1", `login:` followed by your `$USER`, and `password:` followed by a random number of `*`, imitating
  the real 42 login. Pure cosmetics; kept as setting 4, "Login animation" (on by default).

---

## Exercise format (from CONTRIBUTING.md, for reference)

Each exercise is a folder `.subjects/<STUD_PART|PISCINE_PART>/exam_0X/<level>/<name>/` containing:
- `attachment/subject.en.txt` (plus any other files handed to the student)
- `<name>.c`: the reference solution, assumed to be correct
- `main.c`: only for function exercises
- `tester.sh`: runs `.system/auto_correc_program.sh` (programs) or `.system/auto_correc_main.sh` (functions) once per
  test case with the test arguments; after each call, if `.system/grading/traceback` exists it moves it to the root and
  exits 1; at the end it runs `touch .system/grading/passed`.
- `beta` (optional): contributor name; shows a "beta exercise" warning when grading.

The `auto_correc_*.sh` scripts and per-exercise `tester.sh` copies were later replaced by the shared runner
`.system/run_tests.sh` and per-exercise `cases` files; see [docs/TESTER_GUIDE.md](../docs/TESTER_GUIDE.md).
