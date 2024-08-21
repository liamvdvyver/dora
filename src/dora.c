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
#include "listener.h"
#include "timer.h"
#include "notify.h"

// Usage
const char *USAGE = "Usage: dora [-w work length (min) | -b break length (min) "
                    "| -s socket path]";

// Default cycles
const long WORK_LEN = 25 * 60;
const long BREAK_LEN = 5 * 60;

// Default state
struct state init_state() {
    struct state ret = {.work_len = WORK_LEN, .break_len = BREAK_LEN};
    return ret;
};


// Parse args, assign state and cycle setup to passed pointers
// TODO: safety
void parse_args(int argc, char **argv, struct state *p_state,
                struct sockaddr_un *p_sock) {

    char opt;
    while ((opt = (getopt(argc, argv, "w:b:s:h"))) != -1) {
        switch (opt) {
        case 'w':
            p_state->work_len = atol(optarg) * 60;
            if (p_state->work_len <= 0) {
                fprintf(stderr, "Positive argument to -w required\n");
                exit(1);
            };
            break;
        case 'b':
            p_state->break_len = atol(optarg) * 60;
            if (p_state->work_len <= 0) {
                fprintf(stderr, "Positive argument to -b required\n");
                exit(1);
            };
            break;
        case 's':
            strncpy(p_sock->sun_path, optarg, sizeof(p_sock->sun_path));
            break;
        case 'h':
            printf("%s\n", USAGE);
            exit(0);
            break;
        };
    };
};

int main(int argc, char **argv) {

    // Initialise socket address and cycles
    struct sockaddr_un local;
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path) - 1);

    struct cycles active_cycles = {WORK_LEN, BREAK_LEN};

    // Mutex to protect state struct
    pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

    // Semphore to notify on phase change
    sem_t notify_sem;
    if ((sem_init(&notify_sem, 0, 1)) == -1) {
        perror("sem_init");
        exit(1);
    };

    // Initialise state
    struct state active_state = init_state();
    parse_args(argc, argv, &active_state, &local);
    strategy_work(&active_state, &state_mutex, &notify_sem);

    // Args for threads
    struct listener_args_struct args;
    args.p_sockaddr = &local;
    args.p_state = &active_state;
    args.p_mutex = &state_mutex;
    args.p_sem = &notify_sem;

    // Threads to setup
    struct thread_info {
        pthread_t *thread;
        void *fn;
    };

    // Launch threads
    const int n_threads = 3;
    pthread_t listener_t;
    pthread_t timer_t;
    pthread_t notifier_t;

    struct thread_info threads[] = {
        {&listener_t, listener_loop},
        {&timer_t, timer_loop},
        {&notifier_t, notifier_loop}
    };

    for (int i = 0; i < n_threads; i++) {
        if ((errno = pthread_create(threads[i].thread, NULL, threads[i].fn, &args)),
            errno != 0) {
            perror("pthread_create");
            exit(1);
        };
    }

    for (int i = 0; i < n_threads; i++) {
        if ((errno = pthread_join(*threads[i].thread, NULL)), errno != 0) {
            perror("pthread_join");
            exit(1);
        }
    }

    if ((sem_destroy(&notify_sem)) == -1) {
        perror("sem_destroy");
        exit(1);
    };

    exit(0);
};
