/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 14:21:50 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 18:45:51 by estarria         ###   ########.fr       */
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
 * Logging function that prints a message with the current timestamp
 * and coder ID.
 * The function uses a mutex to ensure that log messages are not interleaved when
 * multiple threads are logging simultaneously.
 */
void	print_message(t_sim *sim, int id, char *message)
{
	size_t	time;

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
