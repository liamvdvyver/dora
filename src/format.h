#ifndef FORMAT_H
#define FORMAT_H

#include "ipc.h"

// # Time
// %r  - Time remaining in mm:ss
// %R  - Time remaining in minutes, rounded
// %!r - Same as %r, but with an exclamation point if the Pomodoro is done
// %!R - Same as %R, but with an exclamation point if the Pomodoro is done
// %l  - Length of the Pomodoro in mm:ss
// %L  - Length of the Pomodoro in minutes
//
// # Metadata
// %d  - Pomodoro description
// %t  - Pomodoro tags, joined by a comma
//
// # Goals
// %g  - Daily Pomodoro goal
// %!g  - Daily Pomodoro goal, with a preceding slash
// %c  - Completed Pomodoros today

void print_status(char *buffer, const char *format, struct state *state);

#endif
