#include "codexion.h"

static int all_coders_done(t_sim *sim)
{
    int i;

    i = 0;
    while (i < sim->params.number_of_coders)
    {
        if (sim->coders[i].compile_count
            < sim->params.number_of_compiles_required)
            return (0);
        i++;
    }
    return (1);
}

// Race condition. Reading sim->stop without holding the mutex is a race condition
static void stop_simulation(t_sim *sim)
{
    int i;

    sim->stop = 1;
    safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
    i = 0;
    while (i < sim->params.number_of_coders)
    {
        safe_cond_handle(&sim->coders[i].cond, BROADCAST, NULL, NULL);
        i++;
    }
}

void    *monitor_thread(void *arg)
{
    t_sim *sim = (t_sim *)arg;
    int index;
    t_coder *coders;
    t_params *params;

    coders = sim->coders;
    params = &sim->params;
    while (1)
    {
        safe_mutex_handle(&sim->mutex, LOCK);
        index = 0;
        while (index < params->number_of_coders)
        {
            /** Check whether any coder exceeded:
             * last_compile_start + time_to_burnout 
             */
            if (coders[index].state != STATE_DONE
                && gettime(MILLISECONDS) - coders[index].last_compile_start
                    >= params->time_to_burnout)
            {
                print_message(sim, coders[index].id, "burned out");
                stop_simulation(sim);
                safe_mutex_handle(&sim->mutex, UNLOCK);
                return (NULL);
            }
            if (coders[index].compile_count >= params->number_of_compiles_required)
            {
                coders[index].state = STATE_DONE;
            }
            /* Check whether any coder reached the required compile count. */
            index++;
        }
        if (all_coders_done(sim))
        {
            stop_simulation(sim);
            safe_mutex_handle(&sim->mutex, UNLOCK);
            return (NULL);
        }
        safe_mutex_handle(&sim->mutex, UNLOCK);
        usleep(1000);
    }
    return (NULL);
}
