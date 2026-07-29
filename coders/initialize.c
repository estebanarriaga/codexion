#include "codexion.h"

static int    init_heap(t_heap *heap, int capacity)
{
	heap->items = malloc(sizeof(t_request) * capacity);
	if (heap->items == NULL)
			return (-1);
	heap->size = 0;
	heap->capacity = capacity;
	return (0);
}

static void   init_coder(t_sim *sim, int i)
{
	t_coder *coder;

	coder = &sim->coders[i];
	coder->sim = sim;
	coder->id = i + 1;
	coder->state = STATE_WAITING;
	coder->compile_count = 0;
	coder->last_compile_start = sim->start_ms;
    coder->left_dongle = i;
	coder->right_dongle = (i + 1) % sim->params.number_of_coders;
    // TODO: We have to retrieve the status from all safe condition handles and propagate errors up to the caller instead of just returning void here.
	safe_cond_handle(&coder->cond, INIT_COND, &sim->mutex, NULL);
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
	sim->start_ms = 0;
	sim->sequence = 0;
	sim->stop = 0;
	sim->coders = malloc(sizeof(t_coder) * params->number_of_coders);
	sim->dongles = malloc(sizeof(t_dongle) * params->number_of_coders);
	if (sim->coders == NULL || sim->dongles == NULL)
			return (-1);
	memset(sim->coders, 0, sizeof(t_coder) * params->number_of_coders);
	memset(sim->dongles, 0, sizeof(t_dongle) * params->number_of_coders);
	if (status = safe_mutex_handle(&sim->mutex, INIT) != 0)
			return (status);
	if (status = safe_mutex_handle(&sim->log_mutex, INIT) != 0)
			return (status);
	if (status = safe_cond_handle(&sim->sched_cond, INIT_COND, &sim->mutex, NULL) != 0)
			return (status);
	i = 0;
	while (i < params->number_of_coders)
	{
			init_coder(sim, i);
			if (init_dongle(sim, i) == -1)
					return (-1);
			i++;
	}
	return (0);
}
