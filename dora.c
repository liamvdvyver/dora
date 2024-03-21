#include <libnotify/notify.h>
#include <stdio.h>
#include <unistd.h>
// #include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>

const char *status_path = "/tmp/dora.fifo";

typedef struct cycles {
    int work_len;
    int break_len;
} cycles;

enum status { running = 1, paused = 2 };

typedef struct State {
    enum status status;
    int remaining;
    int finish;
} State;

void get_state(State *state, enum status *pstate) {
    state->status = *pstate;
    return;
};

void countdown(State *state, enum status *pstate) {
    time_t cur_time;
    time(&cur_time);
    while (cur_time < state->finish & state->status == running) {
        sleep(1);
        time(&cur_time);
        get_state(state, pstate);
    };
    return;
};

void *listen(void *arg) {

    enum status *message_state = (enum status *)arg;

    while (1) {

        int fifo_fd = open(status_path, O_RDONLY);
        printf("opened\n");
        fflush(stdout);
        int message = read(fifo_fd, message_state, 1);
        printf("%d", *message_state);
        close(fifo_fd);
    };
};

int main(int argc, char **argv) {

    if (mkfifo(status_path, 0777) == -1) {
        if (errno != EEXIST) {
            printf("Fifo creation failed\n");
            return 1;
        }
    }

    pthread_t listener;

    // state read in from FIFO
    // make sure to keep this updated and locked when updating state
    // from timer runnning out
    enum status mstate = 1;
    enum status *pstate = &mstate;
    pthread_create(&listener, NULL, &listen, pstate);

    printf("hello from parent\n");

    long cur_time;

    long seconds;
    if (argc > 1) {
        seconds = strtol(argv[1], NULL, 10);
    } else {
        return 1;
    };

    if (seconds == 0) {
        return 1;
    };

    State state = {running, seconds, time(NULL) + seconds};

    countdown(&state, pstate);

    // From Arch wiki
    notify_init("Dora");
    NotifyNotification *notify = notify_notification_new("Dora", "B", "C");
    notify_notification_show(notify, NULL);
    g_object_unref(G_OBJECT(notify));
    notify_uninit();

    return 0;
};
