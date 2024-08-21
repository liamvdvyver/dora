#include "ipc.h"
#include "listener.h"
#include "printing.h"
#include "notify.h"
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

const int HEADING_LEN = 99;
const int BODY_LEN = 99;

void notify_libnotify(char *heading, char *body) {
    notify_init("Dora");
    NotifyNotification *notify = notify_notification_new(heading, body, " ");
    notify_notification_show(notify, NULL);
    g_object_unref(G_OBJECT(notify));
    notify_uninit();
}

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
