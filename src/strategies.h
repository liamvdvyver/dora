// Strategies to update state

#ifndef STRATEGIES_H
#define STRATEGIES_H

#include "ipc.h"

#include <pthread.h>
#include <semaphore.h>

void handle_control(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem, struct request *p_req);

void strategy_tick(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);
void strategy_next(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);
void strategy_work(struct state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);

#endif
