#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.h"
#include "printing.h"

const int PATH_LEN = 99;
const int OUTPUT_LEN = 99;

const char *USAGE = "Usage: doractl [-q query | -c control (-t minutes) | -h]";
const char *QUERY_USAGE =
    "Usage: doractl -q [status | phase | remaining | finish]";
const char *CONTROL_USAGE =
    "Usage: doractl -c [pause | run | stop | restart | next | work | break | "
    "worklen (-t minutes) | breaklen (-t minutes)]";

// Print response
void get_output(state *p_state, enum field query, char *buf, int n) {

    switch (query) {
    case NO_FIELD:
        *buf = '\0';
        break;
    case REMAINING:
        snprintf(buf, n - 1, "%ld", p_state->remaining);
        break;
    case FINISH:
        snprintf(buf, n - 1, "%ld", p_state->finish);
        break;
    case STATUS:
        print_status(buf, n - 1, p_state->status);
        break;
    case PHASE:
        print_phase(buf, n - 1, p_state->phase);
        break;
    };
};

int main(int argc, char **argv) {

    // Bad usage
    if (argc == 1) {
        fprintf(stderr, "%s\n", USAGE);
        exit(1);
    };

    // Initialise request
    request req = INIT_REQUEST;

    // Query to be printed locally
    enum field query = NO_FIELD;

    // Initialise address
    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path) - 1);

    // Populate with input
    char opt;
    while ((opt = (getopt(argc, argv, "s:q:c:t:h"))) != -1) {
        switch (opt) {

        case 's':
            strncpy(remote.sun_path, optarg, sizeof(remote.sun_path));
            break;
            // Get usage
        case 'h':
            printf("%s\n", USAGE);
            exit(0);
            break;

        // Query state
        case 'q':
            if (strcmp(optarg, "status") == 0) {
                query = STATUS;
            } else if (strcmp(optarg, "phase") == 0) {
                query = PHASE;
            } else if (strcmp(optarg, "remaining") == 0) {
                query = REMAINING;
            } else if (strcmp(optarg, "finish") == 0) {
                query = FINISH;
            } else {
                errno = EINVAL;
                perror("-q");
                printf("%s\n", QUERY_USAGE);
                exit(1);
            };
            break;

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
            } else if (strcmp(optarg, "worklen") == 0) {
                req.control = SET_WORK_LEN;
            } else if (strcmp(optarg, "breaklen") == 0) {
                req.control = SET_BRK_LEN;
            } else {
                errno = EINVAL;
                perror("-c");
                printf("%s\n", CONTROL_USAGE);
                exit(1);
            };
            break;

        case 't':
            // TODO: safety
            req.minutes = atol(optarg);
            break;
        };
    };

    // Check minutes provided if needed
    if (req.minutes == 0 &&
        (req.control == SET_WORK_LEN || req.control == SET_BRK_LEN)) {
        fprintf(stderr, "Positive argument to -t required\n");
        exit(1);
    };

    int sock;
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    };

    if ((connect(sock, (const struct sockaddr *)&remote, sizeof(remote))) ==
        -1) {
        perror("connect");
        exit(1);
    };

    if ((send(sock, (const void *)&req, sizeof(request), 0)) == -1) {
        perror("send");
        exit(1);
    };

    response resp;
    if ((recv(sock, &resp, sizeof(resp) - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    };

    // Format query
    if (query != NO_FIELD) {
        char output[OUTPUT_LEN];
        get_output(&resp.state, query, output, OUTPUT_LEN - 1);
        printf("%s\n", output);
    };

    exit(resp.exit);
};
