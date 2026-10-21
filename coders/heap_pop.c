/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_pop.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 14:22:14 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 14:22:15 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap_internal.h"

static int	heap_can_pop(const t_heap *heap, const char *scheduler)
{
	return (!heap_is_empty(heap) && scheduler != NULL);
}

static void	heap_pop_unchecked(t_heap *heap, const char *scheduler)
{
	heap->size--;
	if (heap->size > 0)
	{
		heap->requests[0] = heap->requests[heap->size];
		heap_shift_down(heap, 0, scheduler);
	}
}

int	heap_pop_pair(t_heap *first, t_heap *second, const char *scheduler)
{
	if (!heap_can_pop(first, scheduler))
		return (-1);
	if (!heap_can_pop(second, scheduler))
		return (-1);
	heap_pop_unchecked(first, scheduler);
	heap_pop_unchecked(second, scheduler);
	return (0);
}
