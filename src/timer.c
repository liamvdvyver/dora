#include "ipc.h"
#include "listener.h"
#include "strategies.h"

#include "timer.h"

// Timer tick period
const int TIMER_TICK = 1;

// Run on a loop
void *timer_loop(void *args) {

    state *p_state = ((struct listener_args_struct *)args)->p_state;
    pthread_mutex_t *p_state_mutex =
        ((struct listener_args_struct *)args)->p_mutex;
    sem_t *p_notify_sem = ((struct listener_args_struct *)args)->p_sem;

    while (p_state->status != STOPPED) {

        time_t cur_time = time(NULL);
        if (cur_time < p_state->finish) {

            // Do nothing
            sleep(TIMER_TICK);
            strategy_tick(p_state, p_state_mutex, p_notify_sem);

        } else {

            // Start next phase
            strategy_next(p_state, p_state_mutex, p_notify_sem);
            sem_post(p_notify_sem);
        };
    };

    pthread_exit(0);
};
