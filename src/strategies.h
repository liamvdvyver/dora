// Strategies to update state

#ifndef STRATEGIES_H
#define STRATEGIES_H

#include "ipc.h"

#include <pthread.h>
#include <semaphore.h>

void handle_control(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem, request *p_req);

void strategy_tick(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);
void strategy_next(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);
void strategy_work(state *p_state, pthread_mutex_t *p_mutex, sem_t *p_sem);

#endif
