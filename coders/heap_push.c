/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_push.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 14:22:14 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 15:32:38 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap_internal.h"

static int	heap_can_push(const t_heap *heap, const char *scheduler)
{
	return (heap != NULL && heap->requests != NULL && scheduler != NULL
		&& heap->size >= 0 && heap->size < heap->capacity);
}

static void	heap_push_unchecked(t_heap *heap, t_request request,
		const char *scheduler)
{
	heap->requests[heap->size] = request;
	heap_shift_up(heap, heap->size, scheduler);
	heap->size++;
}

int	heap_push_pair(t_heap *first, t_heap *second,
		t_request_pair requests, const char *scheduler)
{
	if (!heap_can_push(first, scheduler))
		return (-1);
	if (!heap_can_push(second, scheduler))
		return (-1);
	heap_push_unchecked(first, requests.left, scheduler);
	heap_push_unchecked(second, requests.right, scheduler);
	return (0);
}
