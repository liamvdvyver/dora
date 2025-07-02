#include <string.h>
#include "ipc.h"

void print_status(char *buf, int n, enum status status) {
    switch (status) {
    case ACTIVE:
        strncpy(buf, "running", n);
        break;
    case PAUSED:
        strncpy(buf, "paused", n);
        break;
    case DONE:
        strncpy(buf, "done", n);
        break;
    case INACTIVE:
        strncpy(buf, "inactive", n);
        break;
    };
};

void print_phase(char *buf, int n, enum phase phase) {
    switch (phase) {
    case WORKING:
        strncpy(buf, "working", n);
        break;
    case BREAKING:
        strncpy(buf, "breaking", n);
        break;
    };
};
