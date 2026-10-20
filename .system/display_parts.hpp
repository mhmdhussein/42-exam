#pragma once

#include "display.hpp"
#include "ui.hpp"

// ==> Helpers of the Modern display (display_modern.cpp) also used by the Dashboard
namespace modern
{
std::string uni(const char *u, const char *a); // unicode character, or its ASCII stand-in
std::string dot(void);                         // separator between items on one line
std::string bold(const std::string &s);
std::string dim(const std::string &s);
std::string highlight(const std::string &s); // inverted row; s must not hold escape codes
std::string two(long n);
std::string clock_long(long s);  // 2:41:07
std::string clock_short(long s); // 2:41
std::string duration(long s);    // 1h 52m
std::string wait_time(long s);   // 4m 12s
std::string tries_text(int n);
std::string ordinal(int n);
std::string exam_title(bool student, int exam_number);
std::string untab(const std::string &s);
std::string grading_progress(void); // first line of .system/grading/progress
std::string exam_hint(bool student, int exam_number); // 4 levels · 3h
std::string toggle(bool on);                          // ● on / ○ off
const char *style_label(DisplayStyle style);
// end card: time, cheats, passed levels; then its title, footer and colour
std::vector<std::string> end_lines(const ExamSummary &s);
void end_card(const ExamSummary &s, ui::BoxOpts &opts);
std::string part_label(const practice_entry &e);               // Student / Piscine
std::string practice_row(const practice_entry &e, int name_width); // plain: name, part, exam, level, ✓
// content of the result card; sets title, label, footer and colour of opts
std::vector<std::string> result_lines(const GradeResult &r, ui::BoxOpts &opts, size_t max_block);
}
