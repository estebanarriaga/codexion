#include "codexion.h"

void   *coding_simmulation(void *arg)
{
    t_coder *coder;

    coder = (t_coder *)arg;
    // Spinlock
    wait_all_threads_ready(coder->sim);

    return (NULL);
}

void    coding_start(t_sim *sim)
{
    int i;

    i = 0;
    if (sim->params.number_of_coders == 0)
        return ;
    else if (sim->params.number_of_coders == 1)
        ; // TODO
    else
    {
        while (i < sim->params.number_of_coders)
        {
            // TODO: coding simulation
            safe_thread_handle(&sim->coders[i].thread, CREATE,
                coding_simmulation, &sim->coders[i]);
            i ++;
        }
    }
    // Start the simulation
    sim->start_ms = gettime(MILLISECONDS);

    // Wait for everyone
    while (i < sim->params.number_of_coders)
    {
        safe_thread_handle(&sim->coders[i].thread, JOIN, NULL, NULL);
        i ++;
    }

    // If we manage to reach here, all coders have finished.

}