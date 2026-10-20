/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 10:58:36 by estarria          #+#    #+#             */
/*   Updated: 2026/10/20 11:07:55 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


static int both_dongles_are_free(t_coder *coder)
{
    t_sim *sim = coder->sim;
    t_dongle *left_dongle = &sim->dongles[coder->left_dongle];
    t_dongle *right_dongle = &sim->dongles[coder->right_dongle];
    long now;

    now = gettime(MILLISECONDS);
    return (left_dongle->owner_id == 0
        && right_dongle->owner_id == 0
        && left_dongle->cooldown_until <= now
        && right_dongle->cooldown_until <= now);
}

static struct timespec ms_to_timespec(long ms)
{
    struct timespec result;

    result.tv_sec = ms / 1000;
    result.tv_nsec = (ms % 1000) * 1000000;
    return (result);
}

static long next_cooldown_deadline(t_sim *sim)
{
    long now;
    long deadline;
    int i;

    now = gettime(MILLISECONDS);
    deadline = 0;
    i = 0;
    while (i < sim->params.number_of_coders)
    {
        if (!heap_is_empty(&sim->dongles[i].waiters)
            && sim->dongles[i].owner_id == 0
            && sim->dongles[i].cooldown_until > now
            && (deadline == 0 || sim->dongles[i].cooldown_until < deadline))
            deadline = sim->dongles[i].cooldown_until;
        i++;
    }
    return (deadline);
}

/**
 * 1. Peek the top request of both required dongle heaps.
 * 2. Confirm both top requests belong to the same coder.
 * 3. Confirm both dongles are unowned and out of cooldown.
 * 4. Pop both requests.
 * 5. Set both ownew_id = coder->id.
 * 6. Signal the coder condition
 * 
 * > Note: Currently the simulation.c does most of this logic,
 * the coders acquire both dongles by themselves. We should remove
 * this duplicated logic accordingly.
 */
static int try_grant(t_coder *coder)
{
    t_sim *sim;
    t_dongle *left_dongle;
    t_dongle *right_dongle;
    t_request  left_request;
    t_request  right_request;

    sim = coder->sim;
    left_dongle = &sim->dongles[coder->left_dongle];
    right_dongle = &sim->dongles[coder->right_dongle];

    // Get the top requests from both dongle heaps
    if (heap_peek(&left_dongle->waiters, &left_request) != 0)
        return (-1);
    if (heap_peek(&right_dongle->waiters, &right_request) != 0)
        return (-1);
    // Check if both top requests belong to the same coder
    if (left_request.coder_id == coder->id
        && right_request.coder_id == coder->id)
    {
        // If they do belong to the same coder, check if both dongles are free and not in cooldown 
        if (both_dongles_are_free(coder))
        {
            if (heap_pop_pair(&left_dongle->waiters, &right_dongle->waiters,
                    sim->params.scheduler) != 0)
                return (-1);
            left_dongle->owner_id = coder->id;
            right_dongle->owner_id = coder->id;
            safe_cond_handle(&coder->cond, BROADCAST, NULL, NULL);
            return (0);
        }
    }
    return (-1);
}

/**
 * Steps of the scheduler thread:
 * 1. Coder enqueues a request for each required dongle and waits on the sched_cond.
 * 2. Scheduler examines heap tops under sim->mutex.
 * 3. Scheduler atomically grants both dongles to an eligible coder.
 * 4. Scheduler signals that coder's personal condition variable to wake it up.
 * That preserves atomic two-dongle acquisition while giving FIFO/EDF a single authority.
 */
void *scheduler_thread(void *arg)
{
    t_sim *sim = (t_sim *)arg;
    int i;
    long deadline;
    struct timespec wake_time;

    safe_mutex_handle(&sim->mutex, LOCK);
    while (!sim->stop)
    {
        i = 0;
        while (i < sim->params.number_of_coders)
        {
            if (sim->coders[i].request_pending)
                try_grant(&sim->coders[i]);
            i++;
        }
        deadline = next_cooldown_deadline(sim);
        if (!sim->stop && deadline > 0) // If there is a cooldown deadline in a coder dongle, wait until that time to recheck the heaps
        {
            // Note: We wait 
            wake_time = ms_to_timespec(deadline);
            safe_cond_handle(&sim->sched_cond, TIMEDWAIT,
                &sim->mutex, &wake_time);
        }
        else if (!sim->stop)
            safe_cond_handle(&sim->sched_cond, WAIT, &sim->mutex, NULL);
    }
    safe_mutex_handle(&sim->mutex, UNLOCK);
    return (NULL);
}
