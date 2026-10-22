/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 10:58:39 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 15:33:03 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Create a queue request while the caller holds sim->mutex. */
static t_request	create_request(t_coder *coder)
{
	t_request	request;
	t_sim		*sim;

	sim = coder->sim;
	request.arrival_ms = gettime(MILLISECONDS);
	request.deadline_ms = coder->last_compile_start
		+ sim->params.time_to_burnout;
	request.coder_id = coder->id;
	return (request);
}

/* Check ownership of both dongles while sim->mutex is held. */
static int	owns_both_dongles_locked(t_coder *coder)
{
	t_dongle	*left;
	t_dongle	*right;

	left = &coder->sim->dongles[coder->left_dongle];
	right = &coder->sim->dongles[coder->right_dongle];
	return (left->owner_id == coder->id && right->owner_id == coder->id);
}

/* Enqueue both requests atomically while sim->mutex is held. */
static int	add_requests_locked(t_sim *sim, t_coder *coder)
{
	t_dongle		*left;
	t_dongle		*right;
	t_request_pair	requests;

	left = &sim->dongles[coder->left_dongle];
	right = &sim->dongles[coder->right_dongle];
	requests.left = create_request(coder);
	requests.right = create_request(coder);
	return (heap_push_pair(&left->waiters, &right->waiters,
			requests, sim->params.scheduler));
}

/* Reject stopped/done coders and notify the scheduler of a pending request. */
static int	request_dongles_locked(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (sim->stop || coder->state == STATE_DONE)
		return (0);
	if (!coder->request_pending)
	{
		if (add_requests_locked(sim, coder) != 0)
			return (0);
		coder->request_pending = 1;
		safe_cond_handle(&sim->sched_cond, BROADCAST, NULL, NULL);
	}
	coder->state = STATE_WAITING;
	return (1);
}

/* Wait for a grant under sim->mutex; return zero when work must stop. */
int	wait_until_grant(t_coder *coder)
{
	t_sim	*sim;
	int		granted;

	sim = coder->sim;
	safe_mutex_handle(&sim->mutex, LOCK);
	granted = request_dongles_locked(coder);
	while (granted && !sim->stop && coder->state != STATE_DONE
		&& !owns_both_dongles_locked(coder))
		safe_cond_handle(&coder->cond, WAIT, &sim->mutex, NULL);
	granted = granted && !sim->stop && coder->state != STATE_DONE;
	if (granted)
	{
		coder->request_pending = 0;
		coder->last_compile_start = gettime(MILLISECONDS);
		coder->state = STATE_COMPILING;
	}
	safe_mutex_handle(&sim->mutex, UNLOCK);
	return (granted);
}
