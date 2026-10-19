/** Note how it is the heap that becomes the database and a state communication layer in the program,
 * as well as the conditions and agent state add an agent state.
 */

#include "codexion.h"

/**
 * Create one request after the caller has made space in its target heap.
 */
static t_request	create_request(t_coder *coder)
{
    t_request	request;
    t_sim		*sim;

    sim = coder->sim;
    request.arrival_ms = gettime(MILLISECONDS);
    request.deadline_ms = coder->last_compile_start + sim->params.time_to_burnout;
    request.sequence = sim->sequence++;
    request.coder_id = coder->id;
    return (request);
}

/**
 * Enqueue the requests for both dongles as one logical operation.
 *
 * The caller holds sim->mutex, so no other thread can change either heap after
 * this preflight. heap_push() only fails for invalid inputs or a full heap; the
 * preflight rules those cases out before either heap is changed.
 */
static int	add_requests(t_sim *sim, t_coder *coder)
{
    t_dongle	*left_dongle;
    t_dongle	*right_dongle;

    left_dongle = &sim->dongles[coder->left_dongle];
    right_dongle = &sim->dongles[coder->right_dongle];
    heap_push(&left_dongle->waiters, create_request(coder), sim->params.scheduler);
    if (left_dongle != right_dongle)
        heap_push(&right_dongle->waiters, create_request(coder), sim->params.scheduler);
    return (0);
}

/* TODO: We have to clean this, this is work from the monitor thread,
 * we have to find a way to do it from there. */
/* The caller holds sim->mutex. Wake every waiter before ending the run. */
static void	stop_after_request_failure(t_sim *sim)
{
    int	index;

    sim->stop = 1;
    safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
    index = 0;
    while (index < sim->params.number_of_coders)
    {
        safe_cond_handle(&sim->coders[index].cond, BROADCAST, NULL, NULL);
        index++;
    }
}

static void release_both_dongles(t_coder *coder)
{
    t_sim *sim = coder->sim;
    t_dongle *left_dongle = &sim->dongles[coder->left_dongle];
    t_dongle *right_dongle = &sim->dongles[coder->right_dongle];
    long cooldown;

    cooldown = gettime(MILLISECONDS) + sim->params.dongle_cooldown;
    left_dongle->owner_id = 0;
    left_dongle->cooldown_until = cooldown;
    if (left_dongle != right_dongle)
    {
        right_dongle->owner_id = 0;
        right_dongle->cooldown_until = cooldown;
    }
}

/**
 * The implemenation details of the coder thread are:
 *
 * 1. Coder threads should run on loop, untill sim->stop (set by the monitor).
 * 2. sim->mutex should be aquired and released between coder state changes
 *  - This allows other coders who are waiting to aquire or release the dongles
 *  - This also allows for the dongle to cool down
 * 3. The coder should store the last_compile_time under the same lock as claiming
 * both dongles (usleep) so it doesnt get interrupted and we dont get bad data.
 * 4. There should be two heper functions:
 *  - release_both_dongles()
 *  - aquire_both_dongles()
 * 5. The coder thread should not check if it has accomplished all his compilations
 *  This is a task from the monitor thread
 */
void *coding_simulation(void *arg)
{
    t_coder *coder;
    t_sim *sim;
    t_dongle *left_dongle;
    t_dongle *right_dongle;

    coder = (t_coder *)arg;
    sim = coder->sim;
    left_dongle = &sim->dongles[coder->left_dongle];
    right_dongle = &sim->dongles[coder->right_dongle];
    while (1)
    {
        safe_mutex_handle(&sim->mutex, LOCK);
        if (!coder->request_pending)
        {
            coder->state = STATE_WAITING;
            if (add_requests(sim, coder) != 0)
                stop_after_request_failure(sim);
            else
            {
                print_message(sim, coder->id, "is requesting both dongles"); // Debuggin purposes
                coder->request_pending = 1;
                safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
            }
        }

        while (!sim->stop && (left_dongle->owner_id != coder->id || right_dongle->owner_id != coder->id))
            safe_cond_handle(&coder->cond, WAIT, &sim->mutex, NULL);

        if (sim->stop)
        {
            safe_mutex_handle(&sim->mutex, UNLOCK);
            break ;
        }
        coder->request_pending = 0;

        // COMPILATION: This step happens when the scheduler has
        // granted both dongles to the coder. Woken up by the scheduler.
        coder->last_compile_start = gettime(MILLISECONDS);
        coder->state = STATE_COMPILING;
        safe_mutex_handle(&sim->mutex, UNLOCK);
        print_message(sim, coder->id, "is compiling");
        usleep(sim->params.time_to_compile * 1000);

        safe_mutex_handle(&sim->mutex, LOCK);
        release_both_dongles(coder);
        coder->compile_count++;

        // DEBUGGING
        coder->state = STATE_DEBUGGING;
        safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
        safe_mutex_handle(&sim->mutex, UNLOCK);

        print_message(sim, coder->id, "is debugging");
        usleep(sim->params.time_to_debug * 1000);

        safe_mutex_handle(&sim->mutex, LOCK);
        // REFACTORING
        coder->state = STATE_REFACTORING;
        safe_mutex_handle(&sim->mutex, UNLOCK);
        print_message(sim, coder->id, "is refactoring");
        usleep(sim->params.time_to_refactor * 1000);
    }
    return (NULL);
}
