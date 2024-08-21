// Standard definiitons for IPC

#ifndef IPC_H
#define IPC_H

typedef struct cycles {
    long workLen;
    long breakLen;
} cycles;

enum status { RUNNING, PAUSED, STOPPED };
enum phase { WORKING, BREAKING };

typedef struct state {
    enum status status;
    enum phase phase;
    long remaining;
    long finish;
    long work_len;
    long break_len;

} state;

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
    TICK
};

typedef struct request {
    enum control control;
    long minutes;
} request;

typedef struct response {
    int exit;
    state state;
} response;

extern const char *SOCK_PATH;
extern const request INIT_REQUEST;
extern const int LEN_RESPONSE;

#endif
