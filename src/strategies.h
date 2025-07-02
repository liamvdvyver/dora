// Strategies to update state
//
// In general, strategies take in a pointer to state, state mutex, and semaphore,
// And return 0 if the command was valid in the current state.

#ifndef STRATEGIES_H
#define STRATEGIES_H

#include "ipc.h"

#include <pthread.h>
#include <semaphore.h>

void handle_control(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem, struct request *p_req);

void strategy_tick(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);
void strategy_next(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);
void strategy_work(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);
void strategy_stop(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);

#endif
