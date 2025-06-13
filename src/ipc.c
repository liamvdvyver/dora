#include "ipc.h"
#include <stdio.h>

void init_settings(struct settings *settings) {

    settings->daily_goal = 8;
    settings->default_break_duration = 5;
    settings->default_pomodoro_duration = 25;
    settings->default_tags = NULL;
};

const char *SOCK_PATH = "/tmp/dora.socket";
const struct request INIT_REQUEST = {.control = NO_CONTROL, .minutes = 0};
const int LEN_RESPONSE = 99;
