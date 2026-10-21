/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 10:58:39 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 13:07:13 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	wait_phase(t_sim *sim, long duration_ms)
{
	long	start;
	long	now;
	int		stopped;

	start = gettime(MILLISECONDS);
	if (start < 0)
		return (0);
	while (1)
	{
		safe_mutex_handle(&sim->mutex, LOCK);
		stopped = sim->stop;
		safe_mutex_handle(&sim->mutex, UNLOCK);
		if (stopped)
			return (0);
		now = gettime(MILLISECONDS);
		if (now < 0)
			return (0);
		if (now - start >= duration_ms)
			return (1);
		usleep(1000);
	}
}

/* Release both dongles and start their cooldown while sim->mutex is held. */
static void	release_both_dongles_locked(t_coder *coder)
{
	t_dongle	*left;
	t_dongle	*right;
	long		cooldown;

	left = &coder->sim->dongles[coder->left_dongle];
	right = &coder->sim->dongles[coder->right_dongle];
	cooldown = gettime(MILLISECONDS) + coder->sim->params.dongle_cooldown;
	left->owner_id = 0;
	left->cooldown_until = cooldown;
	right->owner_id = 0;
	right->cooldown_until = cooldown;
}

/*
 * Report progress and release resources under sim->mutex.
 * At the compile target, wait for the monitor's completion decision.
 * Condition waits release the mutex so the monitor can mark STATE_DONE.
 */
static int	finish_compile_locked(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	release_both_dongles_locked(coder);
	coder->compile_count++;
	safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
	while (!sim->stop && coder->state != STATE_DONE
		&& coder->compile_count >= sim->params.number_of_compiles_required)
		safe_cond_handle(&coder->cond, WAIT, &sim->mutex, NULL);
	return (!sim->stop && coder->state != STATE_DONE);
}

/* Compile after a grant; return zero on stop, error, or completion. */
static int	run_compile(t_coder *coder)
{
	t_sim	*sim;
	int		keep_running;

	sim = coder->sim;
	if (!wait_until_grant(coder))
		return (0);
	print_message(sim, coder->id, "is compiling");
	wait_phase(sim, sim->params.time_to_compile);
	safe_mutex_handle(&sim->mutex, LOCK);
	keep_running = finish_compile_locked(coder);
	safe_mutex_handle(&sim->mutex, UNLOCK);
	return (keep_running);
}

/* Run a timed phase only while active; STATE_DONE is terminal. */
static int	run_timed_phase(t_coder *coder, t_coder_state state,
		char *message, long duration_ms)
{
	t_sim	*sim;

	sim = coder->sim;
	safe_mutex_handle(&sim->mutex, LOCK);
	if (sim->stop || coder->state == STATE_DONE)
	{
		safe_mutex_handle(&sim->mutex, UNLOCK);
		return (0);
	}
	coder->state = state;
	safe_mutex_handle(&sim->mutex, UNLOCK);
	print_message(sim, coder->id, message);
	wait_phase(sim, duration_ms);
	return (1);
}

/* Repeat work until stop or completion, then return for main to join. */
void	*coding_simulation(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = arg;
	sim = coder->sim;
	while (run_compile(coder))
	{
		if (!run_timed_phase(coder, STATE_DEBUGGING, "is debugging",
				sim->params.time_to_debug))
			break ;
		if (!run_timed_phase(coder, STATE_REFACTORING, "is refractoring",
				sim->params.time_to_refactor))
			break ;
	}
	return (NULL);
}
