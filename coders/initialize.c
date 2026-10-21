/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initialize.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:47:37 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 13:31:23 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	allocate_arrays(t_sim *sim)
{
	sim->number_of_dongles = sim->params.number_of_coders;
	if (sim->number_of_dongles == 1)
		sim->number_of_dongles++;
	sim->coders = malloc(sizeof(t_coder) * sim->params.number_of_coders);
	if (sim->coders == NULL)
		return (-1);
	memset(sim->coders, 0,
		sizeof(t_coder) * sim->params.number_of_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->number_of_dongles);
	if (sim->dongles == NULL)
		return (-1);
	memset(sim->dongles, 0,
		sizeof(t_dongle) * sim->number_of_dongles);
	return (0);
}

static int	init_sync(t_sim *sim)
{
	int	status;

	status = safe_mutex_handle(&sim->mutex, INIT);
	if (status != 0)
		return (status);
	status = safe_mutex_handle(&sim->log_mutex, INIT);
	if (status != 0)
		return (status);
	return (safe_cond_handle(&sim->sched_cond, INIT_COND, &sim->mutex, NULL));
}

static int	init_coder(t_sim *sim, int i)
{
	t_coder	*coder;

	coder = &sim->coders[i];
	coder->sim = sim;
	coder->id = i + 1;
	coder->state = STATE_WAITING;
	coder->compile_count = 0;
	coder->last_compile_start = sim->start_ms;
	coder->left_dongle = i;
	coder->right_dongle = (i + 1) % sim->number_of_dongles;
	return (safe_cond_handle(&coder->cond, INIT_COND, &sim->mutex, NULL));
}

static int	init_dongle(t_sim *sim, int i)
{
	t_dongle	*dongle;

	dongle = &sim->dongles[i];
	dongle->id = i;
	dongle->owner_id = 0;
	dongle->cooldown_until = 0;
	dongle->waiters.requests = malloc(sizeof(t_request)
			* sim->params.number_of_coders);
	if (dongle->waiters.requests == NULL)
		return (-1);
	dongle->waiters.size = 0;
	dongle->waiters.capacity = sim->params.number_of_coders;
	return (0);
}

int	initialize_simulation(t_sim *sim, t_params *params)
{
	int	i;
	int	status;

	memset(sim, 0, sizeof(*sim));
	sim->params = *params;
	sim->start_ms = gettime(MILLISECONDS);
	status = allocate_arrays(sim);
	if (status == 0)
		status = init_sync(sim);
	if (status != 0)
		return (status);
	i = 0;
	while (i < sim->number_of_dongles)
	{
		if (i < params->number_of_coders)
			status = init_coder(sim, i);
		if (status != 0)
			return (status);
		if (init_dongle(sim, i) == -1)
			return (-1);
		i++;
	}
	return (0);
}
