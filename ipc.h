const char *SOCK_PATH = "/tmp/dora.socket";

typedef struct cycles {
    long workLen;
    long breakLen;
} cycles;

enum status { RUNNING = 1, PAUSED = 2 };
enum phase { WORKING = 1, BREAKING = 2 };

typedef struct state {
    enum status status;
    enum phase phase;
    long remaining;
    long finish;

} state;

enum field { NO_FIELD, STATUS, REMAINING, FINISH };
enum control { NO_CONTROL, PAUSE, RUN, TOGGLE, STOP, RESTART, NEXT, WORK, BRK };

typedef struct request {
    struct cycles cycles;
    enum control control;
} request;

const request INIT_REQUEST = {.cycles = {0, 0}, .control = NO_CONTROL};

const int LEN_RESPONSE = 99;

typedef struct response {
    int exit;
    state state;
} response;
