#include "codexion.h"

/**
 * Initializes the heap structure
 */
static int    init_heap(t_heap *heap, int capacity)
{
	heap->requests = malloc(sizeof(t_request) * capacity);
	if (heap->requests == NULL)
			return (-1);
	heap->size = 0;
	heap->capacity = capacity;
	return (0);
}

/**
 * Initializes coder structure and its conditional variables
 * @returns the status from the
 */
static int    init_coder(t_sim *sim, int i)
{
	t_coder *coder;

	coder = &sim->coders[i];
	coder->sim = sim;
	coder->id = i + 1;
	coder->state = STATE_WAITING;
	coder->compile_count = 0;
	coder->last_compile_start = sim->start_ms;
    coder->left_dongle = i; // 
	coder->right_dongle = (i + 1) % sim->params.number_of_coders;
	return (safe_cond_handle(&coder->cond, INIT_COND, &sim->mutex, NULL));
}

static int    init_dongle(t_sim *sim, int i)
{
	t_dongle        *dongle;

	dongle = &sim->dongles[i];
	dongle->id = i;
	dongle->owner_id = 0;
	dongle->cooldown_until = 0;
	return (init_heap(&dongle->waiters, sim->params.number_of_coders));
}

int   initialize_simulation(t_sim *sim, t_params *params)
{
	int     i;
    int status;

	memset(sim, 0, sizeof(*sim));
	sim->params = *params;
	sim->start_ms = gettime(MILLISECONDS);
	sim->sequence = 0;
	sim->stop = 0;
	sim->coders = malloc(sizeof(t_coder) * params->number_of_coders);
	sim->dongles = malloc(sizeof(t_dongle) * params->number_of_coders);
	if (sim->coders == NULL || sim->dongles == NULL)
	{
		if (sim->coders)
			free(sim->coders);
		if (sim->dongles)
			free(sim->dongles);
		return (-1);
	}
	memset(sim->coders, 0, sizeof(t_coder) * params->number_of_coders);
	memset(sim->dongles, 0, sizeof(t_dongle) * params->number_of_coders);
	status = safe_mutex_handle(&sim->mutex, INIT);
	if (status != 0)
			return (status);
	status = safe_mutex_handle(&sim->log_mutex, INIT);
	if (status != 0)
			return (status);
	status = safe_cond_handle(&sim->sched_cond, INIT_COND, &sim->mutex, NULL);
	if (status != 0)
			return (status);
	i = 0;
	while (i < params->number_of_coders)
	{
			status = init_coder(sim, i);
			if (status != 0)
					return (status);
			if (init_dongle(sim, i) == -1)
					return (-1);
			i++;
	}
	return (0);
}
