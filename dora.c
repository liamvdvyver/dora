#include <errno.h>
#include <fcntl.h>
#include <iso646.h>
#include <libnotify/notify.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "ipc.h"
#include "strategies.h"
#include "printing.h"

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

void notify_libnotify(char *heading, char *body) {
    notify_init("Dora");
    NotifyNotification *notify = notify_notification_new(heading, body, " ");
    notify_notification_show(notify, NULL);
    g_object_unref(G_OBJECT(notify));
    notify_uninit();
}

const int HEADING_LEN = 99;
const int BODY_LEN = 99;

void get_notification(state *p_state, char *heading, char *body) {
    // Set heading
    strncpy(heading, "Dora", HEADING_LEN);

    // Get status info
    char status[BODY_LEN];
    char phase[BODY_LEN];
    memset(status, 0, BODY_LEN);
    memset(phase, 0, BODY_LEN);
    print_status(status, BODY_LEN - 1, p_state->status);
    print_phase(phase, BODY_LEN - 1, p_state->phase);

    // Set body
    if (p_state->phase == STOPPED) {
        strncpy(body, phase, BODY_LEN - 1);
    } else {
        snprintf(body, BODY_LEN - 1, "%s (%s)", phase, status);
    };
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
    sem_t *p_sem;
};

void *listener_loop(void *args) {

    // Read in data
    struct sockaddr_un *local;
    local = ((struct listener_args_struct *)args)->p_sockaddr;
    state *p_state = ((struct listener_args_struct *)args)->p_state;
    pthread_mutex_t *p_state_mutex =
        ((struct listener_args_struct *)args)->p_mutex;
    sem_t *p_notify_sem = ((struct listener_args_struct *)args)->p_sem;

    // Set up cycles/socket for run
    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;

    // Open socket
    int sock;
    errno = 0;
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    };

    printf("Opened socket\n");

    // Bind
    if (unlink(local->sun_path) == -1) {
        if (errno != 2) {
            perror("unlink");
            exit(1);
        };
    };

    if (bind(sock, (const struct sockaddr *)local, sizeof(*local)) == -1) {
        perror("bind");
        exit(1);
    };

    printf("Bound socket to %s\n", local->sun_path);

    // Listen
    if (listen(sock, 5) == -1) {
        perror("listen");
        exit(1);
    };

    printf("Listening\n");

    while (p_state->status != STOPPED) {

        // Accept request
        socklen_t len = sizeof(remote);
        int sock_connected;
        if ((sock_connected = accept(sock, (struct sockaddr *)&remote, &len)) == -1) {
            perror("accept");
            exit(1);
        };

        // Receive requests
        request req;
        while (len = recv(sock_connected, &req, sizeof(req), 0), len > 0) {

            // Handle control
            handle_control(p_state, p_state_mutex, p_notify_sem, &req);

            // Respond
            response resp;
            memset(&resp, 0, sizeof(resp));
            resp.exit = 0;
            resp.state = *p_state;
            send(sock_connected, &resp, sizeof(resp) - 1, 0);
        };
    };

    pthread_exit(0);
};

// Run on a loop
void *timer_loop(void *args) {

    state *p_state = ((struct listener_args_struct *)args)->p_state;
    pthread_mutex_t *p_state_mutex =
        ((struct listener_args_struct *)args)->p_mutex;
    sem_t *p_notify_sem = ((struct listener_args_struct *)args)->p_sem;

    while (p_state->status != STOPPED) {

        time_t cur_time = time(NULL);
        if (cur_time < p_state->finish) {

            // Do nothing
            sleep(TIMER_TICK);
            strategy_tick(p_state, p_state_mutex, p_notify_sem);

        } else {

            // Start next phase
            strategy_next(p_state, p_state_mutex, p_notify_sem);
            sem_post(p_notify_sem);
        };
    };

    pthread_exit(0);
};

void *notifier_loop(void *args) {

    state *p_state = ((struct listener_args_struct *)args)->p_state;
    pthread_mutex_t *p_state_mutex =
        ((struct listener_args_struct *)args)->p_mutex;
    sem_t *p_notify_sem = ((struct listener_args_struct *)args)->p_sem;

    while (p_state->status != STOPPED) {

        if ((sem_wait(p_notify_sem)) == -1) {
            perror("sem_wait");
            exit(1);
        };

        char heading[HEADING_LEN];
        char body[BODY_LEN];
        get_notification(p_state, heading, body);
        notify_libnotify(heading, body);
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

    // Mutex to protect state struct
    pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

    // Semphore to notify on phase change
    sem_t notify_sem;
    if ((sem_init(&notify_sem, 0, 1)) == -1) {
        perror("sem_init");
        exit(1);
    };

    // Initialise state
    state active_state = init_state();
    strategy_work(&active_state, &state_mutex, &notify_sem);

    // Args for threads
    struct listener_args_struct args;
    args.p_sockaddr = &local;
    args.p_state = &active_state;
    args.p_mutex = &state_mutex;
    args.p_sem = &notify_sem;

    // Launch threads
    pthread_t listener_t;
    if ((errno = pthread_create(&listener_t, NULL, &listener_loop, &args)), errno != 0) {
        perror("pthread_create");
        exit(1);
    };

    pthread_t timer_t;
    if ((errno = pthread_create(&timer_t, NULL, &timer_loop, &args)), errno != 0) {
        perror("pthread_create");
        exit(1);
    };

    pthread_t notifier_t;
    if ((errno = pthread_create(&notifier_t, NULL, &notifier_loop, &args)), errno != 0) {
        perror("pthread_create");
        exit(1);
    };

    // Terminate
    if ((errno = pthread_join(timer_t, NULL)), errno != 0) {
        perror("pthread_join");
        exit(1);
    };

    if ((errno = pthread_join(notifier_t, NULL)), errno != 0) {
        perror("pthread_join");
        exit(1);
    };

    if ((errno = pthread_join(listener_t, NULL)), errno != 0) {
        perror("pthread_join");
        exit(1);
    };

    if ((sem_destroy(&notify_sem)) == -1) {
        perror("sem_destroy");
        exit(1);
    };

    return 0;
};
