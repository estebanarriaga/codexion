/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:47:37 by estarria          #+#    #+#             */
/*   Updated: 2026/10/22 10:34:21 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Create coder, scheduler, and monitor threads. */
static int	start_simulation(t_sim *sim)
{
	int	status;
	int	i;

	i = 0;
	while (i < sim->params.number_of_coders)
	{
		status = safe_thread_handle(&sim->coders[i].thread, CREATE,
				coding_simulation, &sim->coders[i]);
		if (status != 0)
			return (status);
		i++;
	}
	status = safe_thread_handle(&sim->scheduler_thread, CREATE,
			scheduler_thread, sim);
	if (status != 0)
		return (status);
	status = safe_thread_handle(&sim->monitor_thread, CREATE,
			monitor_thread, sim);
	return (status);
}

int	main(int argc, char *argv[])
{
	t_params	params;
	t_sim		sim;
	int			status;

	status = 0;
	if (parse_arguments(argc, argv, &params) == -1)
		return (1);
	status = initialize_simulation(&sim, &params);
	if (status != 0)
		return (destroy_simulation(&sim, status));
	status = start_simulation(&sim);
	return (destroy_simulation(&sim, status));
}
