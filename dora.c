#include <errno.h>
#include <fcntl.h>
#include <libnotify/notify.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "ipc.h"
#include "strategies.h"

// Default cycles
const long WORK_LEN = 25 * 60;
const long BREAK_LEN = 5 * 60;

// Default state
state init_state() {
    struct state ret = {.work_len = WORK_LEN, .break_len = BREAK_LEN};
    return ret;
};

// Timer tick period
const int TIMER_TICK = 1;

void notify(char *heading, char *body) {
    notify_init("Dora");
    NotifyNotification *notify = notify_notification_new(heading, body, " ");
    notify_notification_show(notify, NULL);
    g_object_unref(G_OBJECT(notify));
    notify_uninit();
}

// Parse args, assign state and cycle setup to passed pointers
void parse_args(int argc, char **argv, cycles *pcycles,
                struct sockaddr_un *p_sock) {

    char opt;
    while ((opt = (getopt(argc, argv, "w:b:s:"))) != -1) {
        switch (opt) {
        case 'w':
            pcycles->workLen = strtol(optarg, NULL, 10) * 60;
        case 'b':
            pcycles->breakLen = strtol(optarg, NULL, 10) * 60;
        case 's':
            strcpy(p_sock->sun_path, optarg);
        };
    };
};

struct listener_args_struct {
    struct sockaddr_un *p_sockaddr;
    pthread_mutex_t *p_mutex;
    state *p_state;
};

void *listener_loop(void *args) {

    // Read in data
    struct sockaddr_un *local;
    local = ((struct listener_args_struct *)args)->p_sockaddr;
    state *p_state = ((struct listener_args_struct *)args)->p_state;
    pthread_mutex_t *p_state_mutex =
        ((struct listener_args_struct *)args)->p_mutex;

    // Set up cycles/socket for run
    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;

    // Open socket
    int sock;
    errno = 0;
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("Error in socket creation: %d\n", errno);
        exit(1);
    };

    printf("Opened socket\n");

    // Bind
    if (unlink(local->sun_path) == -1) {
        if (errno != 2) {
            printf("Error in unlinking %s: %d\n", local->sun_path, errno);
            exit(1);
        };
    };

    if (bind(sock, (const struct sockaddr *)local, sizeof(*local)) == -1) {
        printf("Error in socket binding: %d\n", errno);
        exit(1);
    };

    printf("Bound socket to %s\n", local->sun_path);

    // Listen
    listen(sock, 5);

    printf("Listening\n");

    while (p_state->status != STOPPED) {

        // Accept request
        socklen_t len = sizeof(remote);
        unsigned int sock_connected =
            accept(sock, (struct sockaddr *)&remote, &len);

        // Receive requests
        request req;
        while (len = recv(sock_connected, &req, sizeof(req), 0), len > 0) {

            // Handle control
            handle_control(p_state, p_state_mutex, req.control);

            // Respond
            response resp;
            memset(&resp, 0, sizeof(resp));
            resp.exit = 0;
            resp.state = *p_state;
            send(sock_connected, &resp, sizeof(resp) - 1, 0);
        };
    };

    pthread_exit(NULL);
};

// Run on a loop
void *timer_loop(void *args) {

    state *p_state = ((struct listener_args_struct *)args)->p_state;
    pthread_mutex_t *p_state_mutex =
        ((struct listener_args_struct *)args)->p_mutex;

    while (p_state->status != STOPPED) {

        time_t cur_time = time(NULL);
        if (cur_time < p_state->finish) {

            // Do nothing
            sleep(TIMER_TICK);
            strategy_tick(p_state, p_state_mutex);

        } else {

            // Start next phase
            printf("switching\n");
            strategy_next(p_state, p_state_mutex);
        };
    };

    pthread_exit(0);
};

int main(int argc, char **argv) {

    // Initialise socket address and cycles
    struct sockaddr_un local;
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path) - 1);

    cycles active_cycles = {WORK_LEN, BREAK_LEN};

    // Parse args
    parse_args(argc, argv, &active_cycles, &local);

    // Launch listener thread
    pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

    // Initialise state
    state active_state = init_state();
    strategy_work(&active_state, &state_mutex);

    struct listener_args_struct args;
    args.p_sockaddr = &local;
    args.p_state = &active_state;
    args.p_mutex = &state_mutex;

    pthread_t listener_t;
    pthread_create(&listener_t, NULL, &listener_loop, &args);

    pthread_t timer_t;
    pthread_create(&timer_t, NULL, &timer_loop, &args);

    pthread_detach(timer_t);
    pthread_join(listener_t, NULL);

    notify("DONE", "bottom text");

    return 0;
};
