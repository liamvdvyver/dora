#ifndef LISTENER_H
#define LISTENER_H

#include "ipc.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

struct listener_args_struct {
    struct sockaddr_un *p_sockaddr;
    pthread_mutex_t *p_mutex;
    state *p_state;
    sem_t *p_sem;
};

void *listener_loop(void *args);

#endif
