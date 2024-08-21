#include "ipc.h"

const char *SOCK_PATH = "/tmp/dora.socket";
const request INIT_REQUEST = {.control = NO_CONTROL, .minutes = 0};
const int LEN_RESPONSE = 99;
