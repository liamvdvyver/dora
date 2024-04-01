// Standard definiitons for IPC

#ifndef _ipc_h_included
#define _ipc_h_included

const char *SOCK_PATH = "/tmp/dora.socket";

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
enum control { NO_CONTROL, PAUSE, RUN, TOGGLE, STOP, RESTART, NEXT, WORK, BRK, SET_WORK_LEN, SET_BRK_LEN };

typedef struct request {
    enum control control;
    long minutes;
} request;

const request INIT_REQUEST = {.control = NO_CONTROL, .minutes = 0};

const int LEN_RESPONSE = 99;

typedef struct response {
    int exit;
    state state;
} response;

#endif
