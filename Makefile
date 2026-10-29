NAME     = .system/a.out
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -pthread
LDLIBS   = -lreadline

# Simulator version, shown on the login screen: bump the VERSION file for a release
VERSION  := $(strip $(shell cat VERSION 2> /dev/null))
CXXFLAGS += -DEXAMSHELL_VERSION='"$(or $(VERSION),dev)"'

SRCS = .system/exercise.cpp .system/main.cpp .system/menu.cpp .system/exam.cpp \
       .system/utils.cpp .system/grade_request.cpp .system/data_persistence.cpp \
       .system/practice.cpp .system/display_legacy.cpp .system/display_examshell.cpp .system/display_modern.cpp \
       .system/display_dashboard.cpp .system/term.cpp .system/ui.cpp .system/ui_screen.cpp .system/git_submit.cpp
OBJDIR = .system/obj
OBJS = $(SRCS:.system/%.cpp=$(OBJDIR)/%.o)

all: build
	@bash .system/launch.sh

grade: all

build: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDLIBS) -o $(NAME)

$(OBJDIR)/%.o: .system/%.cpp .system/exam.hpp .system/exercise.hpp .system/display.hpp .system/display_parts.hpp .system/term.hpp .system/ui.hpp $(wildcard VERSION)
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean build

check-subjects:
	@bash .system/check_subjects.sh

reset:
	rm -rf rendu subjects success traces traceback .system/grading .system/remote.git
	rm -f .system/exam_token/current_token.txt

help:
	@echo "make        build and start the exam"
	@echo "make build  build only"
	@echo "make re     rebuild from scratch"
	@echo "make fclean remove build output"
	@echo "make check-subjects  run every reference solution through its tester"
	@echo "make reset  delete session files (rendu, subjects, traces, saved exam)"

.PHONY: all grade build clean fclean re reset check-subjects help
