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

// Default cycles
const long WORK_LEN = 25 * 60;
const long BREAK_LEN = 5 * 60;
const cycles CYCLES = {WORK_LEN, BREAK_LEN};

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

void listener_loop() {

    // Set up cycles/socket for run
    struct sockaddr_un local, remote;
    // memset(&local, 0, sizeof(local));
    // memset(&remote, 0, sizeof(remote));

    local.sun_family = AF_UNIX;
    remote.sun_family = AF_UNIX;

    strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path) - 1);

    cycles active_cycles = {WORK_LEN, BREAK_LEN};

    // parse_args(argc, argv, &active_cycles, &local);

    // Open socket
    int sock;
    errno = 0;
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("Error in socket creation: %d\n", errno);
        exit(1);
    };

    printf("Opened socket\n");

    // Bind
    if (unlink(local.sun_path) == -1) {
        if (errno != 2) {
            printf("Error in unlinking %s: %d\n", local.sun_path, errno);
            exit(1);
        };
    };

    if (bind(sock, (const struct sockaddr *)&local, sizeof(local)) == -1) {
        printf("Error in socket binding: %d\n", errno);
        exit(1);
    };

    printf("Bound socket to %s\n", local.sun_path);

    // Listen
    listen(sock, 5);

    printf("Listening\n");

    while (1) {

        // Accept request
        socklen_t len = sizeof(remote);
        unsigned int sock_connected =
            accept(sock, (struct sockaddr *)&remote, &len);

        // Receive requests
        request req;
        while (len = recv(sock_connected, &req, sizeof(req), 0), len > 0) {

            // Respond
            response resp;
            resp.exit = 0;
            strncpy(resp.resp, "ligma" ,LEN_RESPONSE - 1);
            send(sock_connected, &resp, sizeof(resp) - 1, 0);
        };
    };
};

int main(int argc, char **argv) {

    listener_loop();

    // Initialise state
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

    state state = {RUNNING, WORKING, seconds, time(NULL) + seconds};

    // From Arch wiki

    notify("DONE", "bottom text");

    return 0;
};
