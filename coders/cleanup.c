/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 14:22:14 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 14:22:15 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
	while (sim->dongles && i < sim->params.number_of_coders)
	{
		free_heap(&sim->dongles[i].waiters);
		i++;
	}
	free(sim->coders);
	free(sim->dongles);
}
