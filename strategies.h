// Strategies to update state

#include <pthread.h>
#include <semaphore.h>

#include "ipc.h"

void strategy_tick(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    if (p_state->status == RUNNING) {
        pthread_mutex_lock(p_mutex);
        p_state->remaining = p_state->finish - time(NULL);
        pthread_mutex_unlock(p_mutex);
    };
};

void strategy_pause(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->status = PAUSED;
    pthread_mutex_unlock(p_mutex);
};

void strategy_run(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->status = RUNNING;
    p_state->finish = time(NULL) + p_state->remaining;
    pthread_mutex_unlock(p_mutex);
};

void strategy_toggle(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    switch (p_state->status) {
    case RUNNING:
        strategy_pause(p_state, p_mutex, p_sem);
        break;
    case PAUSED:
        strategy_run(p_state, p_mutex, p_sem);
        break;
    case STOPPED:
        break;
    };
};

void strategy_stop(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->status = STOPPED;
    pthread_mutex_unlock(p_mutex);
};

void strategy_work(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->phase = WORKING;
    p_state->status = RUNNING;
    p_state->remaining = p_state->work_len;
    p_state->finish = time(NULL) + p_state->remaining;
    pthread_mutex_unlock(p_mutex);
}

void strategy_brk(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->phase = BREAKING;
    p_state->status = RUNNING;
    p_state->remaining = p_state->break_len;
    p_state->finish = time(NULL) + p_state->remaining;
    pthread_mutex_unlock(p_mutex);
}

void strategy_restart(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    switch (p_state->phase) {
    case WORKING:
        strategy_work(p_state, p_mutex, p_sem);
        break;
    case BREAKING:
        strategy_brk(p_state, p_mutex, p_sem);
        break;
    };
}

void strategy_next(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    switch (p_state->phase) {
    case WORKING:
        strategy_brk(p_state, p_mutex, p_sem);
        break;
    case BREAKING:
        strategy_work(p_state, p_mutex, p_sem);
        break;
    };
}

void handle_control(state *p_state, pthread_mutex_t *p_mutex,
                    enum control control, sem_t *p_sem) {
    if (control != NO_CONTROL) {
        switch (control) {
        case NO_CONTROL:
            break;
        case PAUSE:
            strategy_pause(p_state, p_mutex, p_sem);
            break;
        case RUN:
            strategy_run(p_state, p_mutex, p_sem);
            break;
        case TOGGLE:
            strategy_toggle(p_state, p_mutex, p_sem);
            break;
        case STOP:
            strategy_stop(p_state, p_mutex, p_sem);
            break;
        case RESTART:
            strategy_restart(p_state, p_mutex, p_sem);
            break;
        case NEXT:
            strategy_next(p_state, p_mutex, p_sem);
            break;
        case WORK:
            strategy_work(p_state, p_mutex, p_sem);
            break;
        case BRK:
            strategy_brk(p_state, p_mutex, p_sem);
            break;
        case SET_BRK_LEN:
            break;
        case SET_WORK_LEN:
            break;
        };
        sem_post(p_sem);
    };
};
