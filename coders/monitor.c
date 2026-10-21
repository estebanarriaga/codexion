/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 10:58:39 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 12:17:03 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
 * Check completion before burnout while sim->mutex is held.
 * Return 1 for done, 0 for active, and -1 for burnout.
 * Only the monitor sets STATE_DONE and wakes the completed coder.
 */
static int	check_coder_locked(t_coder *coder, long now)
{
	if (coder->state == STATE_DONE)
		return (1);
	if (coder->compile_count >= coder->sim->params.number_of_compiles_required)
	{
		coder->state = STATE_DONE;
		safe_cond_handle(&coder->cond, BROADCAST, NULL, NULL);
		return (1);
	}
	if (now - coder->last_compile_start >= coder->sim->params.time_to_burnout)
	{
		print_message(coder->sim, coder->id, "burned out");
		return (-1);
	}
	return (0);
}

/*
 * Scan coders under sim->mutex. Stop on burnout or when all coders are done.
 */
static int	check_simulation_locked(t_sim *sim)
{
	long	now;
	int		i;
	int		done;
	int		result;

	now = gettime(MILLISECONDS);
	i = 0;
	done = 0;
	while (i < sim->params.number_of_coders)
	{
		result = check_coder_locked(&sim->coders[i], now);
		if (result < 0)
			return (1);
		done += result;
		i++;
	}
	return (done == sim->params.number_of_coders);
}

/*
 * Set the stop flag and wake all waiters while sim->mutex is held.
 */
static void	stop_simulation_locked(t_sim *sim)
{
	int	i;

	sim->stop = 1;
	safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
	i = 0;
	while (i < sim->params.number_of_coders)
	{
		safe_cond_handle(&sim->coders[i].cond, BROADCAST, NULL, NULL);
		i++;
	}
}

/*
 * Own the monitor critical section and poll until completion or burnout.
 * Thread joins remain the responsibility of main.
 */
void	*monitor_thread(void *arg)
{
	t_sim	*sim;
	int		stopped;

	sim = arg;
	while (1)
	{
		safe_mutex_handle(&sim->mutex, LOCK);
		if (!sim->stop && check_simulation_locked(sim))
			stop_simulation_locked(sim);
		stopped = sim->stop;
		safe_mutex_handle(&sim->mutex, UNLOCK);
		if (stopped)
			return (NULL);
		usleep(1000);
	}
}
