/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: estarria <estarria@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 14:22:14 by estarria          #+#    #+#             */
/*   Updated: 2026/10/21 14:22:15 by estarria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_is_empty(const t_heap *heap)
{
	return (heap == NULL || heap->size == 0);
}

/**
 * Returns the request with the highest priority without removing it
 * from the heap.
 */
int	heap_peek(const t_heap *heap, t_request *request)
{
	if (heap_is_empty(heap) || request == NULL)
		return (-1);
	*request = heap->requests[0];
	return (0);
}
