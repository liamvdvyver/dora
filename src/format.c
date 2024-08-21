#include "ipc.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// # Time
//
// # Metadata
//
// # Goals
//
struct print_ret {
    int read;
    int written;
    bool done;
};

struct print_ret print_char(char *buffer, int maxlen, const char *str,
                            struct state *p_state);

void print_status(char *buffer, int maxlen, const char *str,
                  struct state *p_state) {
    if (maxlen <= 0)
        return;

    struct print_ret ret;
    do {
        ret = print_char(buffer, maxlen, str, p_state);
        buffer += ret.written;
        maxlen -= ret.written;
        str += ret.read;

    } while (!ret.done);
};

struct print_ret print_escaped(char *buffer, int maxlen, const char *str);
struct print_ret print_part(char *buffer, int maxlen, const char *str,
                            struct state *p_state);

struct print_ret print_remaining(char *buffer, struct state *p_state);
struct print_ret print_remaining_trunc(char *buffer, struct state *p_state);
struct print_ret print_length(char *buffer, struct state *p_state);
struct print_ret print_length_mins(char *buffer, struct state *p_state);
struct print_ret print_desc(char *buffer, struct state *p_state);
struct print_ret print_tags(char *buffer, struct state *p_state);
struct print_ret print_goal(char *buffer, struct state *p_state);
struct print_ret print_completed(char *buffer, struct state *p_state);
struct print_ret print_completed(char *buffer, struct state *p_state);
struct print_ret print_remaining_excl(char *buffer, struct state *p_state);
struct print_ret print_remaining_trunc_excl(char *buffer,
                                            struct state *p_state);

// Returns number of characters read/written to buffer
struct print_ret print_char(char *buffer, int maxlen, const char *str,
                            struct state *p_state) {
    struct print_ret ret;
    ret.read = 0;
    ret.written = 0;
    ret.done = false;

    if (maxlen <= 0) {
        ret.done = true;
        return ret;
    }

    switch (*str) {

    // Nothing to read
    case '\0':
        ret.read++;
        ret.done = true;
        break;

    // Hand next char to printf literally
    case '\\':
        ret.read++;
        *buffer = *str;
        ret.written++;
        {
            struct print_ret next = print_escaped(
                buffer + ret.written, maxlen - ret.written, str + ret.read);
            ret.read += next.read;
            ret.written += next.written;
        }
        break;

    // Possible pomo value
    // Handle literal % case in that parser
    case '%':
        ret.read++;
        {
            struct print_ret next =
                print_part(buffer + ret.written, maxlen - ret.written,
                           str + ret.read, p_state);
            ret.read += next.read;
            ret.written += next.written;
        }
        break;

    default:
        ret.read++;
        *buffer = *str;
        ret.written++;
        break;
    }

    return ret;
};

struct print_ret print_escaped(char *buffer, int maxlen, const char *str) {
    struct print_ret ret;
    ret.read = 0;
    ret.written = 0;
    ret.done = false;

    if (maxlen <= 0) {
        ret.done = true;
        return ret;
    }

    switch (*str) {

    // Nothing to read
    case '\0':
        ret.read++;
        ret.done = true;
        break;

    // Hand next char to printf literally
    default:
        ret.read++;
        *buffer = *str;
        ret.written++;
        break;
    };

    return ret;
}

struct print_ret print_part(char *buffer, int maxlen, const char *str,
                            struct state *p_state) {
    struct print_ret ret;
    ret.read = 0;
    ret.written = 0;
    ret.done = false;

    if (maxlen <= 0) {
        ret.done = true;
        return ret;
    }

    switch (*str) {

    // Nothing to read
    case '\0':
        ret.read++;
        ret.done = true;
        break;

        // case 'r': // %r  - Time remaining in mm:s
        //     ret.read++;
        //     ret.written = print_remaining(buffer, state);
        //     break;
        //
        // case 'R': // %R  - Time remaining in minutes, rounded
        //     ret.read++;
        //     ret.written = print_remaining_trunc(buffer, state);
        //     break;
        // case 'l': // %l  - Length of the Pomodoro in mm:ss
        //     ret.read++;
        //     ret.written = print_length(buffer, state);
        //     break;
        // case 'L': // %L  - Length of the Pomodoro in minutes
        //     ret.read++;
        //     ret.written = print_length_mins(buffer, state);
        //     break;
        // case 'd': // %d  - Pomodoro description
        //     ret.read++;
        //     ret.written = print_desc(buffer, state);
        //     break;
        // case 't': // %t  - Pomodoro tags, joined by a comma
        //     ret.read++;
        //     ret.written = print_tags(buffer, state);
        //     break;
        // case 'g': // %g  - Daily Pomodoro goal
        //     ret.read++;
        //     ret.written = print_goal(buffer, state);
        //     break;
        // case 'c': // %c  - Completed Pomodoros today
        //     ret.read++;
        //     ret.written = print_completed(buffer, state);
        //     break;
        // case '!':
        //     ret.read++;
        //     switch (*(buffer + 1)) {
        //     case 'g': // %!g  - Daily Pomodoro goal, with a preceding
        //         slash ret.read++;
        //         ret.written = print_completed(buffer, state);
        //         break;
        //     case 'r': // %!r - Same as %r, but with an exclamation point
        //         if the
        //             // Pomodoro is done
        //             ret.read++;
        //         ret.written = print_remaining_excl(buffer, state);
        //         break;
        //     case 'R': // %!R - Same as %R, but with an exclamation point
        //         if the
        //             ret.read++;
        //         ret.written = print_remaining_trunc_excl(buffer, state);
        //         break;
        //     default:
        //         ret.read++;
        //         ret.written = snprintf(specifier, 1, "%s", buffer);
        //         break;
        //     }
        // default:
        //     ret.read++;
        //     ret.written = snprintf(specifier, 1, "%s", buffer);
        //     break;
        // }
        //
    }
    return ret;
};

int main(int charc, char **argv) {
    char out[10];
    print_status(out, 10, argv[1], NULL);

    printf(out);

}
