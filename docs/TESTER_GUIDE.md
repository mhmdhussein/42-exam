# Tester guide

Rules for every exercise tester in this repository. Read this before you add or change a `tester.sh`,
a `cases` file or a `wrong/` solution.

The real 42 exam grader is not public. The testers here started as jcluzet's work, not 42's. These rules
are our own, built on the same idea as the exam: run the student's code and the reference solution on
the same inputs and compare the results. Each rule is tagged:

- **[exam]**: matches known real-exam behaviour (from the subjects or the exam's documented rules).
- **[our choice]**: our decision. The real grader may be more or less strict.

Some parts depend on work that is still planned in the **Reliable testers** milestone: the shared
runner and the `cases` / `allowed` / `config` files (#5), and the `wrong/` solutions (#6). These parts are
marked *(planned)*. Until they land, hand-written testers must follow the same rules.

## 1. Compiling

| Language | Command |
|----------|---------|
| C | `gcc -Wall -Wextra -Werror` |
| C++ (STUD `exam_05`) | `c++ -Wall -Wextra -Werror -std=c++98` |

- The flags are **[exam]**. Every subject asks for them.
- The reference and the student are compiled with the same flags. Function exercises add the
  exercise's `main.c`. For C++, the reference and the student are built in separate folders
  (`build_ref` / `build_stud`) so their headers don't mix.
- A compile error, including a warning turned into an error, is a **failure**. The trace shows the
  full compiler output. **[exam]**
- Extra flags for an exercise (for example `-D BUFFER_SIZE=42`) go in `config` *(planned)*. **[our choice]**

## 2. Allowed functions

Each subject has an `Allowed functions:` line. A solution that calls any other external function fails.
**[exam]** (the rule is in the subjects; how it is checked here is **[our choice]**)

How to check it:

1. Copy the list from the subject into the exercise's `allowed` file, one name per line *(planned)*.
   An empty line or `None` in the subject means no functions are allowed. Function exercises may also
   use whatever the given `main.c` uses; that is checked on the student's file only, so it doesn't matter.
2. Compile the student's files to object files only: `gcc -Wall -Wextra -Werror -c`.
3. List the undefined symbols: `nm -u <file>.o`.
4. Every symbol must be in `allowed` or in the exception list below. If not, the case fails and the
   trace names the function: `forbidden function: strdup`.

Exceptions, always allowed because the compiler adds them, not the student: **[our choice]**

| Symbol | Why it appears |
|--------|----------------|
| `__stack_chk_fail`, `__stack_chk_guard` | stack protector |
| `_GLOBAL_OFFSET_TABLE_` | position-independent code |
| `memcpy`, `memset`, `memmove` | the compiler can emit these for struct copies and array initialisation. Allowed only if the reference solution's `nm -u` also shows them, or if the subject allows them. |
| C++: `__cxa_*`, `__gxx_personality_v0`, `_Unwind_Resume`, `operator new` / `delete` (`_Znwm`, `_ZdlPv`, ...), `std::` symbols (`_ZNSt*`, `_ZSt*`) | C++ runtime and standard library. C++ subjects usually limit headers or features, not functions, so C++ exercises check only what the subject explicitly forbids. |

## 3. Pass/fail rules

A case passes only if **all** of these are true:

- **Same stdout.** The student's stdout, passed through `cat -e`, is byte-for-byte the same as the
  reference's. **[exam]**
- **No crash.** If the process is killed by a signal (exit code above 128), the case fails and the trace
  gives the signal name: `crash: SIGSEGV`. Get it with `kill -l $((status - 128))`. **[our choice]** for
  the message; a crash always fails in the exam.
- **No timeout.** Each case runs under `timeout` (default **10 s**; `config` can change it *(planned)*).
  Exit code 124 is a failure: `timeout after 10s`. Don't use `sleep` / `ps -p` polling loops. **[our choice]**
- **Leaks** are checked only when the exercise's `config` turns them on *(planned)*, with
  `valgrind --leak-check=full --errors-for-leak-kinds=definite --error-exitcode=42`. **[our choice]**

What is compared besides stdout, per exercise type: **[our choice]**

| Type | stdout | stderr | exit code |
|------|--------|--------|-----------|
| Program with arguments | yes | no | no |
| Function (with `main.c`) | yes | no | no |
| Program reading stdin / files (`get_next_line`, `ft_printf`...) | yes | no | no |
| C++ class exercise (custom test `main`) | yes | no | no |
| Exercise whose subject specifies an error message or exit status (`microshell`, `mini_serv`...) | yes | yes, if the subject says where the message goes | yes, if the subject names it |

## 4. Trace format

On failure, the tester writes `.system/grading/traceback`, moves it to the repository root and exits 1
(the examshell then saves it in `traces/`). Every tester writes the same format: **[our choice]**

```
=== CASE 3/12 FAILED: diff ===
COMMAND: ./a.out "hello  world" ""
STDIN:   (none)
EXPECTED (cat -e):
hello world$
GOT (cat -e):
hello  world$
=== END CASE 3 ===
```

- `CASE n/N`: case number and number of cases.
- `FAILED:` the reason, one of `diff`, `crash: SIGSEGV` (any signal name), `timeout after 10s`,
  `compile error`, `forbidden function: <name>`, `leaks`.
- `COMMAND:` the exact command, with every argument shell-quoted so it can be copied and run.
- `STDIN:` the input file or text, or `(none)`.
- `EXPECTED` / `GOT`: both outputs through `cat -e`. For `compile error`, `GOT` is the compiler output
  and `EXPECTED` is left out. For `forbidden function`, both are left out.
- Write the trace with `printf`, never with `echo '\n'` (that prints a literal `\n`).
- Stop at the first failing case.

## 5. Edge cases to cover

Each tester has at least **8 cases**. **[our choice]** Pick the ones that apply from this checklist.

**Programs taking arguments**
- [ ] no arguments
- [ ] too many arguments
- [ ] an empty string `""`
- [ ] only spaces, only tabs, mixed spaces and tabs
- [ ] leading and trailing whitespace
- [ ] a very long string (thousands of characters)
- [ ] non-ASCII bytes (`$'\xc3\xa9'`, `$'\x7f'`)

**String functions**
- [ ] empty strings, for each parameter
- [ ] a very long string
- [ ] characters at the edges of the ASCII table (`\x01`, `' '`, `'~'`, `\x7f`) and above 127

**Number exercises**
- [ ] `0`, `1`, `-1`
- [ ] negative numbers
- [ ] `INT_MIN` (`-2147483648`) and `INT_MAX` (`2147483647`)
- [ ] values just past the overflow edges, when the subject defines that behaviour
- [ ] for atoi-style exercises: leading `+` / `-`, several signs, leading spaces and tabs, junk after the digits

**Memory functions (`ft_strdup`, `ft_range`...)**
- [ ] the result is separate memory: change the copy, then check the original is unchanged
- [ ] the size allocated: the result is used up to its last byte (check with valgrind or `malloc_usable_size` only when `config` turns it on)
- [ ] `NULL` and size 0, when the subject defines them

**Linked lists and arrays**
- [ ] empty (`NULL` / size 0)
- [ ] one element
- [ ] already sorted
- [ ] sorted in reverse
- [ ] duplicates
- [ ] negative values and `INT_MIN` / `INT_MAX`

**Programs reading stdin or files (`get_next_line`, filters...)**
- [ ] empty input
- [ ] input without a final newline
- [ ] only newlines
- [ ] several `BUFFER_SIZE` values: `1`, a small one, the line length, `10000000`
- [ ] a very long line (bigger than any buffer)
- [ ] a file that doesn't exist / a bad file descriptor, when the subject defines it

## 6. Exercise folder layout

```
.subjects/<STUD_PART|PISCINE_PART>/exam_0X/<level>/<name>/
├── attachment/subject.en.txt   subject shown to the student, plus any given files
├── <name>.c                    reference solution, assumed correct (several files or .cpp/.hpp when needed)
├── main.c                      function exercises only: the test main
├── allowed                     (planned, #5) allowed functions, one per line
├── cases                       (planned, #5) test cases, one per line
├── config                      (planned, #5) optional per-exercise settings
├── wrong/                      (planned, #6) known-wrong solutions
├── tester.sh                   today: the tester; after #5: only for custom testers
└── beta                        optional: contributor name, shows a "beta exercise" warning
```

**`cases`** *(planned)*: one case per line. The arguments are written in shell-quoted form, exactly as on a
command line. A `stdin:` prefix gives the file (relative to the exercise folder) to feed on stdin. Lines
starting with `#` and empty lines are ignored.

```
# no arguments
"hello world"
"" "abc"
$'tab\there'
stdin: input/no_final_newline.txt
```

**`config`** *(planned)*: `key=value` lines, all optional.

```
timeout=10
cflags=-D BUFFER_SIZE=42
buffer_sizes=1 42 10000000
leaks=on
```

**Custom `tester.sh`**: allowed only when `cases` can't describe the exercise (`microshell`, `mini_serv`,
the C++ modules, anything needing several processes or a network). A custom tester must still follow
sections 1–4: same flags, same pass/fail rules, the same trace format, and `touch
.system/grading/passed` when every case passes.

## 7. Known-wrong solutions and definition of done

### Writing wrong solutions *(planned, #6)*

Each exercise has a `wrong/` folder of solutions that are almost right. Each file is named after its bug
(`missing_newline.c`, `int_min.c`, `uses_forbidden_strdup.c`). `make check-subjects` runs the tester on
each one; every one must fail, and one that passes is reported as `LEAKY`. **[our choice]**

At least **3** per exercise, including one that uses a forbidden function and one that crashes or loops
forever. Standard set to choose from:

| Exercise type | Wrong solutions |
|---------------|-----------------|
| Any | `uses_forbidden_<fn>.c`, `infinite_loop.c`, `segfault_on_empty.c` |
| Argument programs | `missing_newline.c`, `no_args_case.c` (prints nothing or crashes with no arguments), `wrong_argc.c`, `off_by_one.c` |
| String functions | `empty_string.c`, `off_by_one.c`, `signed_char.c` (breaks on bytes above 127) |
| Number exercises | `int_min.c`, `int_max.c`, `no_sign.c`, `no_whitespace.c` |
| Memory functions | `returns_same_pointer.c`, `missing_nul.c`, `short_malloc.c` |
| Lists / arrays | `empty_list.c`, `single_element.c`, `unstable_duplicates.c` |
| stdin programs | `no_final_newline.c`, `buffer_size_1.c`, `long_line.c` |

### Definition of done

A tester is done when:

- [ ] it follows sections 1–4 (flags, allowed functions, pass/fail rules, trace format)
- [ ] it has at least 8 cases covering the section 5 checklist for its type
- [ ] the reference passes (`check-subjects` says `OK`)
- [ ] an empty submission fails (not `WEAK`)
- [ ] every solution in `wrong/` fails (not `LEAKY`) *(planned)*

Check with `bash .system/check_subjects.sh <exam_0X/name>`.
