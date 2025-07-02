// Standard definiitons for IPC

#ifndef IPC_H
#define IPC_H

// As per protocol, with additional paused state
enum status {
    INACTIVE, // No active pomodoro
    ACTIVE,   // Active pomodoro running
    PAUSED,   // Active pomodor but paused
    DONE,     // Pomodoro time finished
    DEAD      // Daemon shutting down
};

enum phase { WORKING, BREAKING };

typedef long min_t;
typedef long sec_t;

// As per protocol
struct pomodoro {
    long start;        // Time started, in unix time
    sec_t duration;    // Duration in seconds
    char *description; // Nullable description
    char **tags;       // Nullable tags
};

// As per protocol
struct settings {
    int daily_goal;                  // How many pomodoros to aim for
    min_t default_break_duration;    // In minutes
    min_t default_pomodoro_duration; // In minutes
    char **default_tags;             //
};

struct state {
    struct pomodoro pomo;
    struct settings settings;
    enum status status;
    enum phase phase;
    sec_t remaining;
    sec_t finish;
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
};

typedef enum {
    _START,
    _AMEND,
    _STATUS, // Send continuously if blocking
    _CLEAR,
    _FINISH,
    _BREAK,
    _REPEAT,
    _PAUSE,
    _RESUME,
} control_t;

void init_settings(struct settings *settings);

struct request {
    enum control control;
    min_t minutes;
};

typedef struct {
    control_t control;
    struct pomodoro *pomo;
} req_t;

struct response {
    int exit;
    struct state state;
};

extern const char *SOCK_PATH;
extern const struct request INIT_REQUEST;
extern const int LEN_RESPONSE;

#endif
