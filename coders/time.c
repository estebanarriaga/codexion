/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 14:22:14 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 14:22:15 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	wait_phase(t_sim *sim, long duration_ms)
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
