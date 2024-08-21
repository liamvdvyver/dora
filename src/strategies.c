#include "ipc.h"

#include <pthread.h>
#include <semaphore.h>

void strategy_tick(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    if (p_state->status == RUNNING) {
        pthread_mutex_lock(p_mutex);
        p_state->remaining = p_state->finish - time(NULL);
        pthread_mutex_unlock(p_mutex);
    };
};

void strategy_pause(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->status = PAUSED;
    pthread_mutex_unlock(p_mutex);
};

void strategy_run(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->status = RUNNING;
    p_state->finish = time(NULL) + p_state->remaining;
    pthread_mutex_unlock(p_mutex);
};

void strategy_toggle(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
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

void strategy_stop(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->status = STOPPED;
    pthread_mutex_unlock(p_mutex);
};

void strategy_work(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->phase = WORKING;
    p_state->status = RUNNING;
    p_state->remaining = p_state->work_len;
    p_state->finish = time(NULL) + p_state->remaining;
    pthread_mutex_unlock(p_mutex);
}

void strategy_brk(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    pthread_mutex_lock(p_mutex);
    p_state->phase = BREAKING;
    p_state->status = RUNNING;
    p_state->remaining = p_state->break_len;
    p_state->finish = time(NULL) + p_state->remaining;
    pthread_mutex_unlock(p_mutex);
}

void strategy_restart(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    switch (p_state->phase) {
    case WORKING:
        strategy_work(p_state, p_mutex, p_sem);
        break;
    case BREAKING:
        strategy_brk(p_state, p_mutex, p_sem);
        break;
    };
}

void strategy_next(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem) {
    switch (p_state->phase) {
    case WORKING:
        strategy_brk(p_state, p_mutex, p_sem);
        break;
    case BREAKING:
        strategy_work(p_state, p_mutex, p_sem);
        break;
    };
}

void strategy_wrklen(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem,
                     long minutes) {
    int seconds = minutes * 60;
    if (p_state->phase == WORKING) {
        long offset = seconds - p_state->work_len;
        p_state->remaining += offset;
        p_state->finish += offset;
    };
    p_state->work_len = seconds;
}

void strategy_brklen(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem,
                     long minutes) {
    int seconds = minutes * 60;
    if (p_state->phase == BREAKING) {
        long offset = seconds - p_state->break_len;
        p_state->remaining += offset;
        p_state->finish += offset;
    };
    p_state->work_len = seconds;
}

void handle_control(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem,
                    struct request *p_req) {
    if (p_req->control != NO_CONTROL) {
        switch (p_req->control) {
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
            strategy_brklen(p_state, p_mutex, p_sem, p_req->minutes);
            break;
        case SET_WORK_LEN:
            strategy_wrklen(p_state, p_mutex, p_sem, p_req->minutes);
            break;
        };
        sem_post(p_sem);
    };
};
