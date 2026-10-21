/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 14:21:50 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 14:21:53 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

void	print_usage(char *program)
{
	fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown scheduler\n", program);
}

/**
 * Logging function that prints a message with the current timestamp and coder ID.
 * The function uses a mutex to ensure that log messages are not interleaved when
 * multiple threads are logging simultaneously.
 */
void	print_message(t_sim *sim, int id, char *message)
{
	size_t time;

	pthread_mutex_lock(&sim->log_mutex);
	time = gettime(MILLISECONDS) - sim->start_ms;
	if (!sim->stop)
		printf("%ld %d %s\n", time, id, message);
	pthread_mutex_unlock(&sim->log_mutex);
}

/**
 * time_code -> SECONDS MILLISECONDS MICROSECONDS
 * gettime returns the current time in milliseconds.
 */
long	gettime(t_time_code time_code)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL))
	{
		handle_error("Gettimeofday failed.", errno);
		return (-1);
	}
	if (time_code == SECONDS)
		return (tv.tv_sec + tv.tv_usec / 1e6);
	else if (time_code == MILLISECONDS)
		return (tv.tv_sec * 1e3 + tv.tv_usec / 1e3);
	else if (time_code == MICROSECONDS)
		return (tv.tv_sec * 1e6 + tv.tv_usec);
	handle_error("Invalid time code.", EINVAL);
	return (-1);
}

int	destroy_simulation(t_sim *sim, int status)
{
	int	i;

	i = 0;
	safe_thread_handle(&sim->monitor_thread, JOIN, NULL, NULL);
	safe_thread_handle(&sim->scheduler_thread, JOIN, NULL, NULL);
	while (sim->coders && i < sim->params.number_of_coders)
	{
		safe_thread_handle(&sim->coders[i].thread, JOIN, NULL, NULL);
		safe_cond_handle(&sim->coders[i].cond, DESTROY_COND, NULL, NULL);
		i++;
	}
	safe_mutex_handle(&sim->mutex, DESTROY);
	safe_mutex_handle(&sim->log_mutex, DESTROY);
	safe_cond_handle(&sim->sched_cond, DESTROY_COND, NULL, NULL);
	free_simulation(sim);
	return (status);
}

void	free_heap(t_heap *heap)
{
	free(heap->requests);
}

/** Free all allocated resources. */
void	free_simulation(t_sim *sim)
{
	int	i;

	i = 0;
	while (sim->dongles && i < sim->number_of_dongles)
	{
		free_heap(&sim->dongles[i].waiters);
		i++;
	}
	free(sim->coders);
	free(sim->dongles);
}