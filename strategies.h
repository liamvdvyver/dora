// Strategies to update state

#include <pthread.h>

#include "ipc.h"

void strategy_pause(state *p_state, pthread_mutex_t *p_mutex) {
    pthread_mutex_lock(p_mutex);
    p_state->status = PAUSED;
    pthread_mutex_unlock(p_mutex);
};

void strategy_run(state *p_state, pthread_mutex_t *p_mutex) {
    pthread_mutex_lock(p_mutex);
    p_state->status = RUNNING;
    p_state->finish = time(NULL) + p_state->remaining;
    pthread_mutex_unlock(p_mutex);
};

void strategy_toggle(state *p_state, pthread_mutex_t *p_mutex) {
    switch (p_state->status) {
    case RUNNING:
        strategy_pause(p_state, p_mutex);
        break;
    case PAUSED:
        strategy_run(p_state, p_mutex);
        break;
    case STOPPED:
        break;
    };
};

void strategy_stop(state *p_state, pthread_mutex_t *p_mutex) {
    pthread_mutex_lock(p_mutex);
    p_state->status = STOPPED;
    pthread_mutex_unlock(p_mutex);
};

void strategy_work(state *p_state, pthread_mutex_t *p_mutex) {
    pthread_mutex_lock(p_mutex);
    p_state->phase = WORKING;
    p_state->status = RUNNING;
    p_state->remaining = p_state->work_len;
    p_state->finish = time(NULL) + p_state->remaining;
    pthread_mutex_unlock(p_mutex);
}

void strategy_brk(state *p_state, pthread_mutex_t *p_mutex) {
    pthread_mutex_lock(p_mutex);
    p_state->phase = BREAKING;
    p_state->status = RUNNING;
    p_state->remaining = p_state->break_len;
    p_state->finish = time(NULL) + p_state->remaining;
    pthread_mutex_unlock(p_mutex);
}

void strategy_restart(state *p_state, pthread_mutex_t *p_mutex) {
    switch (p_state->phase) {
    case WORKING:
        strategy_work(p_state, p_mutex);
        break;
    case BREAKING:
        strategy_brk(p_state, p_mutex);
        break;
    };
}

void strategy_next(state *p_state, pthread_mutex_t *p_mutex) {
    switch (p_state->phase) {
    case WORKING:
        strategy_brk(p_state, p_mutex);
        break;
    case BREAKING:
        strategy_work(p_state, p_mutex);
        break;
    };
}

void handle_control(state *p_state, pthread_mutex_t *p_mutex,
                    enum control control) {
    switch (control) {
    case NO_CONTROL:
        break;
    case PAUSE:
        strategy_pause(p_state, p_mutex);
        break;
    case RUN:
        strategy_run(p_state, p_mutex);
        break;
    case TOGGLE:
        strategy_toggle(p_state, p_mutex);
        break;
    case STOP:
        strategy_stop(p_state, p_mutex);
        break;
    case RESTART:
        strategy_restart(p_state, p_mutex);
        break;
    case NEXT:
        strategy_next(p_state, p_mutex);
        break;
    case WORK:
        strategy_work(p_state, p_mutex);
        break;
    case BRK:
        strategy_brk(p_state, p_mutex);
    };
};
