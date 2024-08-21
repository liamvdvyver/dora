// Standard definiitons for IPC

#ifndef IPC_H
#define IPC_H

struct cycles {
    long workLen;
    long breakLen;
};

enum status { RUNNING, PAUSED, STOPPED };
enum phase { WORKING, BREAKING };

struct state{
    enum status status;
    enum phase phase;
    long remaining;
    long finish;
    long work_len;
    long break_len;

};

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
