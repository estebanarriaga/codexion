/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 10:58:39 by estarria          #+#    #+#             */
/*   Updated: 2026/10/19 10:59:01 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * Create one request after the caller has made space in its target heap.
 * Assumes:
 *  - The caller holds sim->mutex.
 *  - There is space in the target heap(s).
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

/** Checks if the coder owns both dongles while holding the simulation mutex. */
static int	owns_both_dongles_locked(t_coder *coder)
{
	t_dongle	*left_dongle;
	t_dongle	*right_dongle;
	t_sim		*sim;

	sim = coder->sim;
	left_dongle = &sim->dongles[coder->left_dongle];
	right_dongle = &sim->dongles[coder->right_dongle];
	return (left_dongle->owner_id == coder->id &&
		right_dongle->owner_id == coder->id);
}

/**
 * Enqueue the requests for both dongles as one logical operation.
 *
 * The caller holds sim->mutex, so no other thread can change either heap after
 * this preflight.
 */
static int	add_requests_locked(t_sim *sim, t_coder *coder)
{
	t_dongle	*left_dongle;
	t_dongle	*right_dongle;
	t_request	left_request;
	t_request	right_request;

	left_dongle = &sim->dongles[coder->left_dongle];
	right_dongle = &sim->dongles[coder->right_dongle];
	left_request = create_request(coder);
	right_request = left_request;
	if (left_dongle != right_dongle)
		right_request = create_request(coder);
	return (heap_push_pair(&left_dongle->waiters, left_request,
			&right_dongle->waiters, right_request, sim->params.scheduler));
}

/** Releases both dongles while holding the simulation mutex. */
static void	release_both_dongles_locked(t_coder *coder)
{
	t_sim		*sim;
	t_dongle	*left_dongle;
	t_dongle	*right_dongle;
	long		cooldown;

	sim = coder->sim;
	left_dongle = &sim->dongles[coder->left_dongle];
	right_dongle = &sim->dongles[coder->right_dongle];
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
 * Request dongles and wait on them with the simulation mutex.
 * @returns 0 on error, 1 on success
 */
static int  wait_until_grant(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	safe_mutex_handle(&sim->mutex, LOCK);
	coder->state = STATE_WAITING;
	if (!coder->request_pending)
	{
		if (add_requests_locked(sim, coder) != 0)
		{
			safe_mutex_handle(&sim->mutex, UNLOCK);
			return (0);
		}
		coder->request_pending = 1;
		safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
	}
	print_message(sim, coder->id, "is requesting both dongles"); // Debuggin purposes
	while (!sim->stop && !owns_both_dongles_locked(coder))
		safe_cond_handle(&coder->cond, WAIT, &sim->mutex, NULL);
	if (sim->stop)
	{
		safe_mutex_handle(&sim->mutex, UNLOCK);
		return (0);
	}
	coder->request_pending = 0;
	coder->last_compile_start = gettime(MILLISECONDS);
	coder->state = STATE_COMPILING;
	safe_mutex_handle(&sim->mutex, UNLOCK);
	return (1);
}

/** Owns the compilation and its completion transitions.
 * Waits in stale state until granted both dongles, then compiles,
 * releases dongles, and wakes the scheduler.
 * @returns 0 on error, 1 on success
 */
static int	run_compile(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (wait_until_grant(coder) == 0)
		return (0);
	print_message(sim, coder->id, "is compiling");
	usleep(sim->params.time_to_compile * 1000);
	safe_mutex_handle(&sim->mutex, LOCK);
	release_both_dongles_locked(coder);
	coder->compile_count++;
	safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
	safe_mutex_handle(&sim->mutex, UNLOCK);
	return (1);
}

/** Shared function for actions with the same logic.
 * @returns 0 on error 1 on success
 */
static int	run_timed_phase(t_coder *coder, t_coder_state state, char *message, long duration_ms)
{
	t_sim	*sim;

	sim = coder->sim;
	safe_mutex_handle(&sim->mutex, LOCK);
	if (sim->stop)
	{
		safe_mutex_handle(&sim->mutex, UNLOCK);
		return (0);
	}
	coder->state = state;
	safe_mutex_handle(&sim->mutex, UNLOCK);
	print_message(sim, coder->id, message);
	usleep(duration_ms * 1000);
	return (1);
}

void	*coding_simulation(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	while (run_compile(coder))
	{
		if (!run_timed_phase(coder, STATE_DEBUGGING, "is debugging", sim->params.time_to_debug))
			break ;
		if (!run_timed_phase(coder, STATE_REFACTORING, "is refractoring", sim->params.time_to_refactor))
			break ;
	}
	return NULL;
}
