#include "codexion.h"

/* Returns non-zero when `first` has priority over `second`. */
static int	request_precedes(t_request first, t_request second,
		const char *scheduler)
{
	if (strcmp(scheduler, "fifo") == 0)
	{
		if (first.arrival_ms != second.arrival_ms)
			return (first.arrival_ms < second.arrival_ms);
	}
	else if (first.deadline_ms != second.deadline_ms)
		return (first.deadline_ms < second.deadline_ms);
	return (first.sequence < second.sequence);
}

static void	swap_requests(t_request *first, t_request *second)
{
	t_request	temporary;

	temporary = *first;
	*first = *second;
	*second = temporary;
}

static void	shift_up(t_heap *heap, int index, const char *scheduler)
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

static void	shift_down(t_heap *heap, int index, const char *scheduler)
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

int	heap_is_empty(const t_heap *heap)
{
	return (heap == NULL || heap->size == 0);
}

static int	heap_can_push(const t_heap *heap, const char *scheduler)
{
	return (heap != NULL && heap->requests != NULL && scheduler != NULL
		&& heap->size >= 0 && heap->size < heap->capacity);
}

static int	heap_can_pop(const t_heap *heap, const char *scheduler)
{
	return (!heap_is_empty(heap) && scheduler != NULL);
}

static void	heap_push_unchecked(t_heap *heap, t_request request,
		const char *scheduler)
{
	heap->requests[heap->size] = request;
	shift_up(heap, heap->size, scheduler);
	heap->size++;
}

int	heap_push_pair(t_heap *first, t_request first_request, t_heap *second,
		t_request second_request, const char *scheduler)
{
	if (!heap_can_push(first, scheduler))
		return (-1);
	if (first == second)
	{
		heap_push_unchecked(first, first_request, scheduler);
		return (0);
	}
	if (!heap_can_push(second, scheduler))
		return (-1);
	heap_push_unchecked(first, first_request, scheduler);
	heap_push_unchecked(second, second_request, scheduler);
	return (0);
}

/** Returns the request with the highest priority without removing it from the heap. */
int	heap_peek(const t_heap *heap, t_request *request)
{
	if (heap_is_empty(heap) || request == NULL)
		return (-1);
	*request = heap->requests[0];
	return (0);
}

static void	heap_pop_unchecked(t_heap *heap, const char *scheduler)
{
	heap->size--;
	if (heap->size > 0)
	{
		heap->requests[0] = heap->requests[heap->size];
		shift_down(heap, 0, scheduler);
	}
}

int	heap_pop_pair(t_heap *first, t_heap *second, const char *scheduler)
{
	if (!heap_can_pop(first, scheduler))
		return (-1);
	if (first == second)
	{
		heap_pop_unchecked(first, scheduler);
		return (0);
	}
	if (!heap_can_pop(second, scheduler))
		return (-1);
	heap_pop_unchecked(first, scheduler);
	heap_pop_unchecked(second, scheduler);
	return (0);
}
