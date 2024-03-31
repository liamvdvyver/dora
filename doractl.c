#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.h"

const int PATH_LEN = 99;

// TODO: Usage info
const char *USAGE = "TODO: usage";

int main(int argc, char **argv) {

    // Initialise request
    request req = INIT_REQUEST;

    // Populate with input
    char opt;
    while ((opt = (getopt(argc, argv, "q:c:w:b:h"))) != -1) {
        switch (opt) {

            // Get usage
        case 'h':
            printf("%s\n", USAGE);
            exit(0);

        // Query state
        case 'q':
            if (strcmp(optarg, "time") == 0) {
                req.query = TIME;
            } else if (strcmp(optarg, "status") == 0) {
                req.query = STATUS;
            } else if (strcmp(optarg, "remaining") == 0) {
                req.query = REMAINING;
            } else if (strcmp(optarg, "finish") == 0) {
                req.query = FINISH;
            };

        // Control state
        case 'c':
            if (strcmp(optarg, "pause") == 0) {
                req.control = PAUSE;
            } else if (strcmp(optarg, "run") == 0) {
                req.control = RUN;
            } else if (strcmp(optarg, "toggle") == 0) {
                req.control = TOGGLE;
            } else if (strcmp(optarg, "stop") == 0) {
                req.control = STOP;
            } else if (strcmp(optarg, "restart") == 0) {
                req.control = RESTART;
            } else if (strcmp(optarg, "next") == 0) {
                req.control = NEXT;
            } else if (strcmp(optarg, "work") == 0) {
                req.control = WORK;
            } else if (strcmp(optarg, "break") == 0) {
                req.control = BRK;
            };

        // TODO: Adjust work cycle length
        case 'w':
            continue;

        // Adjust break cycle length
        case 'b':
            continue;
        };
    };

    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path) - 1);

    int sock;
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("fuckd\n");
        exit(1);
    };

    printf("created socket\n");

    if ((connect(sock, (const struct sockaddr *)&remote, sizeof(remote))) ==
        -1) {
        printf("fuckd\n");
        exit(1);
    };

    printf("connected\n");

    send(sock, (const void *)&req, sizeof(request), 0);

    response resp;
    recv(sock, &resp, sizeof(resp) - 1, 0);
    printf("%s\n", resp.resp);
    exit (resp.exit);
};
