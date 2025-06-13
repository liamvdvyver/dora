// Standard definiitons for IPC

#ifndef IPC_H
#define IPC_H

enum status { RUNNING, PAUSED, STOPPED };
enum phase { WORKING, BREAKING };

// As per protocol
struct pomodoro {
    long start;
    long duration;
    char *description;
    char **tags;
};

// As per protocol
struct settings {
    int daily_goal;
    long default_break_duration;
    long default_pomodoro_duration;
    char **default_tags;
};

struct state {
    struct pomodoro pomo;
    struct settings settings;
    enum status status;
    enum phase phase;
    long remaining;
    long finish;
};

// Reqs and Resps

enum field { NO_FIELD, STATUS, REMAINING, FINISH, PHASE };
enum control {
    NO_CONTROL,
    PAUSE,
    RUN,
    TOGGLE,
    STOP,
    RESTART,
    NEXT,
    WORK,
    BRK,
    SET_WORK_LEN,
    SET_BRK_LEN,
};

void init_settings(struct settings *settings);

struct request {
    enum control control;
    long minutes;
};

struct response {
    int exit;
    struct state state;
};

extern const char *SOCK_PATH;
extern const struct request INIT_REQUEST;
extern const int LEN_RESPONSE;

#endif
