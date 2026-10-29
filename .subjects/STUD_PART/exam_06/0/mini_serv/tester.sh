#!/bin/bash
# Custom tester (the shared runner can't drive network clients).
# Runs each scenario against the reference and the student's mini_serv, with
# clients on bash's /dev/tcp (no nc needed), and compares what every client
# received. Runs from the repo root.
source .system/trace.sh
RENDU=${RENDU:-$PWD/rendu/mini_serv} # the pushed snapshot in git submission mode
cd .system/grading || exit 1
rm -f passed

TOTAL=10
WAIT=10          # seconds to wait for the lines a step should produce
SERVER_LIFE=120  # a server never outlives this, even if the tester is killed

PIDS=()
cleanup() {
    local p
    for p in "${PIDS[@]}"; do kill "$p" 2> /dev/null; done
    wait 2> /dev/null
    rm -rf build_ref build_stud run
}
trap cleanup EXIT
trap 'exit 1' INT TERM
fail_build() { trace_build "$1" "$2"; exit 1; }

# ---- build -------------------------------------------------------------------
mkdir -p build_ref build_stud run
gcc -Wall -Wextra -Werror mini_serv.c -o build_ref/mini_serv 2> run/cc \
    || fail_build "reference did not compile" "$(cat run/cc)"
gcc -Wall -Wextra -Werror -c "$RENDU/mini_serv.c" -o build_stud/mini_serv.o 2> run/cc \
    || fail_build "compile error" "$(cat run/cc)"
gcc build_stud/mini_serv.o -o build_stud/mini_serv 2> run/cc || fail_build "compile error" "$(cat run/cc)"
for sym in $(nm -u build_stud/mini_serv.o | awk '{print $NF}'); do
    case "$sym" in __stack_chk_fail | _GLOBAL_OFFSET_TABLE_) continue ;; esac
    grep -qx "$sym" allowed || fail_build "forbidden function: $sym"
done

# ---- servers -----------------------------------------------------------------
# True if something listens on TCP port $1 (read from /proc: connecting to
# check would make the server count a client).
listening() {
    local hex
    hex=$(printf '%04X' "$1")
    awk -v p=":$hex" '$4 == "0A" && substr($2, length($2) - 4) == p { f = 1 } END { exit !f }' \
        /proc/net/tcp /proc/net/tcp6 2> /dev/null
}

# start_server <binary>: runs it on a free port. Sets PORT and SERVER_PID.
start_server() {
    local try t
    for ((try = 0; try < 10; try++)); do
        PORT=$((20000 + RANDOM % 40000))
        listening "$PORT" && continue
        timeout -k 1 "$SERVER_LIFE" "$1" "$PORT" > /dev/null 2>&1 &
        SERVER_PID=$!
        PIDS+=("$SERVER_PID")
        for ((t = 0; t < 50; t++)); do
            listening "$PORT" && return 0
            kill -0 "$SERVER_PID" 2> /dev/null || break
            sleep 0.1
        done
        kill "$SERVER_PID" 2> /dev/null
        wait "$SERVER_PID" 2> /dev/null
    done
    return 1
}

# ---- clients -----------------------------------------------------------------
# Each client is a socket fd in this shell plus a background cat that copies
# what it receives to run/<name>. LINES[name] counts the lines it should have.
declare -A FD READER ID LINES
ORDER=() OPEN=() NEXT_ID=0 LATE=0

# Waits until every open client has received the lines it should have. After
# one wait runs out (a wrong server), the others don't wait: the diff shows it.
settle() {
    local name t
    [ "$LATE" = 1 ] && return
    for name in "${OPEN[@]}"; do
        for ((t = 0; t < WAIT * 20; t++)); do
            [ "$(wc -l < "run/$name")" -ge "${LINES[$name]}" ] && continue 2
            sleep 0.05
        done
        LATE=1
        return
    done
}

# Adds $2 lines to every open client except $1.
expect_others() {
    local name
    for name in "${OPEN[@]}"; do
        [ "$name" != "$1" ] && LINES[$name]=$((LINES[$name] + $2))
    done
}

connect() {
    local fd others name
    exec {fd}<> "/dev/tcp/127.0.0.1/$PORT" || return 1
    : > "run/$1"
    # the reader must not keep the other clients' sockets open
    others=""
    for name in "${OPEN[@]}"; do others="$others ${FD[$name]}"; done
    (
        for f in $others; do eval "exec $f>&-"; done
        exec cat <&"$fd" > "run/$1"
    ) &
    READER[$1]=$!
    PIDS+=("$!")
    FD[$1]=$fd ID[$1]=$NEXT_ID LINES[$1]=0
    NEXT_ID=$((NEXT_ID + 1))
    expect_others "$1" 1
    ORDER+=("$1") OPEN+=("$1")
    settle
}

disconnect() {
    local fd=${FD[$1]} name keep=()
    kill "${READER[$1]}" 2> /dev/null
    wait "${READER[$1]}" 2> /dev/null
    exec {fd}>&-
    for name in "${OPEN[@]}"; do [ "$name" != "$1" ] && keep+=("$name"); done
    OPEN=("${keep[@]}")
    expect_others "$1" 1
    settle
}

# say <client> <text>: sends the text (printf %b); each \n is one line for the others.
say() {
    local text n
    text=$(printf '%b_' "$2")
    text=${text%_}
    printf '%s' "$text" >&"${FD[$1]}"
    n=$(printf '%s' "$text" | tr -cd '\n' | wc -c)
    expect_others "$1" "$n"
    settle
}

# say_file <client> <file>
say_file() {
    cat "$2" >&"${FD[$1]}"
    expect_others "$1" "$(tr -cd '\n' < "$2" | wc -c)"
    settle
}

# Runs scenario $2 on server $1 and prints every client's capture. Call it
# in this shell (not in $(...)) so cleanup() knows the processes it starts.
run_scenario() {
    local name fd
    rm -f run/c_*
    FD=() READER=() ID=() LINES=() ORDER=() OPEN=() NEXT_ID=0 LATE=0
    start_server "$1" || { echo "(could not start the server on a free port)"; return; }
    "$2"
    # fence: once a last client's arrival reached everyone, nothing is in flight
    connect c_end
    for name in "${ORDER[@]}"; do
        printf -- '--- client %s ---\n' "${ID[$name]}"
        cat -e "run/$name"
    done
    [ "$LATE" = 1 ] && printf -- '--- (stopped waiting after %ss) ---\n' "$WAIT"
    kill "$SERVER_PID" 2> /dev/null
    wait "$SERVER_PID" 2> /dev/null
    for name in "${OPEN[@]}"; do
        kill "${READER[$name]}" 2> /dev/null
        fd=${FD[$name]}
        exec {fd}>&-
    done
    wait 2> /dev/null
    PIDS=()
}

# ---- scenarios ---------------------------------------------------------------
arrive_and_leave() { connect c_a; connect c_b; disconnect c_b; }
one_line() { connect c_a; connect c_b; say c_b 'hello world\n'; disconnect c_b; }
many_lines() { connect c_a; connect c_b; say c_b 'first\nsecond\n\nfourth after an empty line\n'; }
split_line() { connect c_a; connect c_b; say c_b 'hel'; sleep 0.3; say c_b 'lo\n'; }
three_clients() {
    connect c_a; connect c_b; connect c_c
    say c_c 'from c\n'; say c_a 'from a\n'
    disconnect c_b; say c_c 'b is gone\n'
}
long_line() { connect c_a; connect c_b; say_file c_b other_long_msg.txt; }
long_no_newline() { connect c_a; connect c_b; say_file c_b very_long_msg.txt; say c_b '\n'; }
ids_keep_counting() {
    connect c_a; connect c_b; disconnect c_b; connect c_c; disconnect c_c
    connect c_d; say c_d 'i am client 3\n'
}

i=0
# check <description> <scenario>
check() {
    local expected actual
    i=$((i + 1))
    run_scenario build_ref/mini_serv "$2" > run/expected
    run_scenario build_stud/mini_serv "$2" > run/actual
    expected=$(cat run/expected)
    actual=$(cat run/actual)
    if [ "$expected" != "$actual" ]; then
        trace_case "$i" "$TOTAL" diff "./mini_serv <port>, then: $1" "" \
            "$(cut -c1-200 <<< "$expected")" "$(cut -c1-200 <<< "$actual")"
        exit 1
    fi
}

# check_exit <description> <args...>: compares stderr and the exit status.
check_exit() {
    local desc=$1 expected actual status reason
    shift
    i=$((i + 1))
    expected=$(timeout -k 1 5 build_ref/mini_serv "$@" 2>&1 > /dev/null | cat -e; echo "exit status: ${PIPESTATUS[0]}")
    actual=$(timeout -k 1 5 build_stud/mini_serv "$@" 2>&1 > /dev/null | cat -e; echo "exit status: ${PIPESTATUS[0]}")
    status=${actual##*: }
    reason=$(trace_status_reason "$status" 5)
    [ -z "$reason" ] && [ "$expected" != "$actual" ] && reason=diff
    if [ -n "$reason" ]; then
        trace_case "$i" "$TOTAL" "$reason" "./mini_serv${*:+$(printf " %q" "$@")}   ($desc)" "" "$expected" "$actual"
        exit 1
    fi
}

check_exit "no arguments"
start_server build_ref/mini_serv || fail_build "could not start the reference server"
check_exit "the port is already in use" "$PORT"
kill "$SERVER_PID" 2> /dev/null
wait "$SERVER_PID" 2> /dev/null

check "client 0 connects, client 1 arrives and leaves" arrive_and_leave
check "client 1 sends one line (not sent back to it)" one_line
check "client 1 sends several lines in one write" many_lines
check "client 1 sends \"hel\", then \"lo\\n\"" split_line
check "three clients talk, one leaves" three_clients
check "client 1 sends a 364 KB line" long_line
check "client 1 sends 10 KB without a newline, then \"\\n\"" long_no_newline
check "ids keep counting after clients leave" ids_keep_counting

touch passed
