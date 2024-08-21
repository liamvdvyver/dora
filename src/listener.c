#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.h"
#include "listener.h"
#include "strategies.h"

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
        if ((sock_connected = accept(sock, (struct sockaddr *)&remote, &len)) ==
            -1) {
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
