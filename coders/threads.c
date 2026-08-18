
#include "codexion.h"

/**
 * TODO: Add comment following the program comment style
 */
static void	stop_created_threads(t_sim *sim, int created)
{
	int	i;

	safe_mutex_handle(&sim->mutex, LOCK);
	sim->stop = 1;
	safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
	i = 0;
	while (i < created)
	{
		safe_cond_handle(&sim->coders[i].cond, BROADCAST, NULL, NULL);
		i++;
	}
	safe_mutex_handle(&sim->mutex, UNLOCK);
}

int	create_coder_threads(t_sim *sim)
{
	int	i;
	int	status;

	sim->start_ms = gettime(MILLISECONDS);
	if (sim->start_ms < 0)
		return (-1);
	i = 0;
	while (i < sim->params.number_of_coders)
	{
		sim->coders[i].last_compile_start = sim->start_ms;
		i++;
	}
	i = 0;
	while (i < sim->params.number_of_coders)
	{
		status = safe_thread_handle(&sim->coders[i].thread, CREATE,
				coding_simulation, &sim->coders[i]);
		if (status != 0)
		{
			stop_created_threads(sim, i);
			while (i-- > 0)
				safe_thread_handle(&sim->coders[i].thread, JOIN, NULL, NULL);
			return (status);
		}
		i++;
	}
	return (0);
}

int	wait_for_threads(t_sim *sim)
{
	int	i;
	int	join_status;
	int	status;

	i = 0;
	status = 0;
	while (i < sim->params.number_of_coders)
	{
		join_status = safe_thread_handle(&sim->coders[i].thread, JOIN,
				NULL, NULL);
		if (status == 0 && join_status != 0)
			status = join_status;
		i++;
	}
	return (status);
}
