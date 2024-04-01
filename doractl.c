#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.h"

const int PATH_LEN = 99;
const int OUTPUT_LEN = 99;

// TODO: Usage info
const char *USAGE = "TODO: usage";

// Print response
void get_output(state *p_state, enum field query, char *buf, int n) {

    switch (query) {
    case NO_FIELD:
        *buf = '\0';
        break;
    case REMAINING:
        snprintf(buf, (unsigned long)n, "%ld\n", p_state->remaining);
        break;
    case FINISH:
        snprintf(buf, n, "%ld\n", p_state->finish);
        break;
    // TODO: pretty print
    case STATUS:
        snprintf(buf, n, "%d\n", p_state->status);
        break;
    };
};

int main(int argc, char **argv) {

    // Initialise request
    request req = INIT_REQUEST;

    // Query to be printed locally
    enum field query = NO_FIELD;

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
            if (strcmp(optarg, "status") == 0) {
                query = STATUS;
            } else if (strcmp(optarg, "remaining") == 0) {
                query = REMAINING;
            } else if (strcmp(optarg, "finish") == 0) {
                query = FINISH;
            } else {
                printf("Bad query\n");
                exit(1);
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

    if ((connect(sock, (const struct sockaddr *)&remote, sizeof(remote))) ==
        -1) {
        printf("fuckd\n");
        exit(1);
    };

    send(sock, (const void *)&req, sizeof(request), 0);

    response resp;
    recv(sock, &resp, sizeof(resp) - 1, 0);

    // Format query
    char output[OUTPUT_LEN];
    get_output(&resp.state, query, output, OUTPUT_LEN - 1);
    printf("%s", output);

    exit(resp.exit);
};
