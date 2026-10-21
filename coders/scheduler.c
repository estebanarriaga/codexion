/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 10:58:36 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 13:31:49 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
 * Check ownership and cooldown for both dongles under sim->mutex.
 */
static int	both_dongles_available(t_dongle *left, t_dongle *right)
{
	long	now;

	now = gettime(MILLISECONDS);
	return (left->owner_id == 0 && right->owner_id == 0
		&& left->cooldown_until <= now && right->cooldown_until <= now);
}

/*
 * Find the earliest future cooldown for an unowned dongle with waiters.
 * The caller holds sim->mutex. Zero means no timed wakeup is needed.
 */
static long	next_cooldown_deadline(t_sim *sim)
{
	t_dongle	*dongle;
	long		now;
	long		deadline;
	int			i;

	now = gettime(MILLISECONDS);
	deadline = 0;
	i = 0;
	while (i < sim->number_of_dongles)
	{
		dongle = &sim->dongles[i];
		if (!heap_is_empty(&dongle->waiters) && dongle->owner_id == 0
			&& dongle->cooldown_until > now
			&& (deadline == 0 || dongle->cooldown_until < deadline))
			deadline = dongle->cooldown_until;
		i++;
	}
	return (deadline);
}

/*
 * Grant both dongles only when the coder leads both queues and they are usable.
 * The caller holds sim->mutex across queue removal, assignment, and wakeup.
 * Return zero on a grant, or -1 if no grant was made.
 */
static int	try_grant(t_coder *coder)
{
	t_dongle	*left;
	t_dongle	*right;
	t_request	left_request;
	t_request	right_request;

	left = &coder->sim->dongles[coder->left_dongle];
	right = &coder->sim->dongles[coder->right_dongle];
	if (heap_peek(&left->waiters, &left_request) != 0
		|| heap_peek(&right->waiters, &right_request) != 0)
		return (-1);
	if (left_request.coder_id != coder->id
		|| right_request.coder_id != coder->id
		|| !both_dongles_available(left, right))
		return (-1);
	if (heap_pop_pair(&left->waiters, &right->waiters,
			coder->sim->params.scheduler) != 0)
		return (-1);
	left->owner_id = coder->id;
	print_message(coder->sim, coder->id, "has taken a dongle");
	right->owner_id = coder->id;
	print_message(coder->sim, coder->id, "has taken a dongle");
	safe_cond_handle(&coder->cond, BROADCAST, NULL, NULL);
	return (0);
}

/*
 * Wait for a scheduler notification or the earliest relevant cooldown.
 * The caller holds sim->mutex; the wait releases it and reacquires it on return.
 */
static void	wait_for_scheduler_event(t_sim *sim)
{
	long			deadline;
	struct timespec	wake_time;

	deadline = next_cooldown_deadline(sim);
	if (deadline > 0)
	{
		wake_time.tv_sec = deadline / 1000;
		wake_time.tv_nsec = (deadline % 1000) * 1000000;
		safe_cond_handle(&sim->sched_cond, TIMEDWAIT,
			&sim->mutex, &wake_time);
	}
	else
		safe_cond_handle(&sim->sched_cond, WAIT, &sim->mutex, NULL);
}

/*
 * Scan pending requests under sim->mutex, then wait for scheduling changes.
 * Queue priority enforces FIFO/EDF; only this thread assigns dongle ownership.
 * Recheck after every wakeup, including spurious ones, until sim->stop is set.
 */
void	*scheduler_thread(void *arg)
{
	t_sim	*sim;
	int		i;

	sim = arg;
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
		if (!sim->stop)
			wait_for_scheduler_event(sim);
	}
	safe_mutex_handle(&sim->mutex, UNLOCK);
	return (NULL);
}
