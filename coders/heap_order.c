/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_order.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 14:22:14 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 18:47:25 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap_internal.h"

/* Returns non-zero when `first` has priority over `second`. */
static int	request_precedes(t_request first, t_request second,
		const char *scheduler)
{
	if (strcmp(scheduler, "fifo") == 0)
		return (first.arrival_ms < second.arrival_ms);
	return (first.deadline_ms < second.deadline_ms);
}

static void	swap_requests(t_request *first, t_request *second)
{
	t_request	temporary;

	temporary = *first;
	*first = *second;
	*second = temporary;
}

void	heap_shift_up(t_heap *heap, int index, const char *scheduler)
{
	int	parent;

	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!request_precedes(heap->requests[index], heap->requests[parent],
				scheduler))
			break ;
		swap_requests(&heap->requests[index], &heap->requests[parent]);
		index = parent;
	}
}

void	heap_shift_down(t_heap *heap, int index, const char *scheduler)
{
	int	left;
	int	right;
	int	best;

	while (1)
	{
		left = index * 2 + 1;
		right = index * 2 + 2;
		best = index;
		if (left < heap->size && request_precedes(heap->requests[left],
				heap->requests[best], scheduler))
			best = left;
		if (right < heap->size && request_precedes(heap->requests[right],
				heap->requests[best], scheduler))
			best = right;
		if (best == index)
			break ;
		swap_requests(&heap->requests[index], &heap->requests[best]);
		index = best;
	}
}
