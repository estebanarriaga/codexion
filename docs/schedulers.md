# Schedulers: FIFO and EDF

The subject requires a scheduler argument:

- `fifo`: First In, First Out.
- `edf`: Earliest Deadline First.

When several coders request the same dongle, the dongle must be granted to the
request selected by the active scheduler.

## Requests

A useful request structure contains:

```c
typedef struct s_request
{
	int		coder_id;
	long	arrival_ms;
	long	deadline_ms;
	long	sequence;
}	t_request;
```

Fields:

- `coder_id`: who is waiting.
- `arrival_ms`: when the request entered the queue.
- `deadline_ms`: `last_compile_start + time_to_burnout`.
- `sequence`: monotonic tiebreaker for deterministic ordering.

## FIFO Ordering

FIFO picks the oldest request:

```text
smallest arrival_ms wins
```

If two requests have the same timestamp, use `sequence` or `coder_id` as a
tiebreaker so behavior is deterministic.

## EDF Ordering

EDF picks the request whose burnout deadline is closest:

```text
smallest deadline_ms wins
```

In Codexion:

```text
deadline = last_compile_start + time_to_burnout
```

EDF is useful because the coder nearest to burnout receives priority. The subject
expects EDF to avoid burnout when the parameters are viable.

## Why A Heap?

A heap is a priority queue. It lets you:

- push a request in `O(log n)`,
- inspect the best request in `O(1)`,
- pop the best request in `O(log n)`.

The subject explicitly requires a priority queue heap because C does not provide
one in the standard library.

## Binary Min-Heap Shape

Use an array:

```text
index:  0  1  2  3  4  5
value: [A, B, C, D, E, F]
```

Relationships:

```text
parent(i) = (i - 1) / 2
left(i)   = 2 * i + 1
right(i)  = 2 * i + 2
```

For a min-heap, the best request is always at index `0`.

## Comparator

A comparator decides whether request `a` should be above request `b`.

FIFO:

```c
if (a.arrival_ms != b.arrival_ms)
	return (a.arrival_ms < b.arrival_ms);
return (a.sequence < b.sequence);
```

EDF:

```c
if (a.deadline_ms != b.deadline_ms)
	return (a.deadline_ms < b.deadline_ms);
return (a.sequence < b.sequence);
```

## Dongle Granting

For each dongle, the scheduler should only grant it when:

- the dongle is not in use,
- cooldown has expired,
- the wait queue is not empty,
- the selected request is still valid,
- the simulation has not stopped.

If using a single central scheduler, the scheduler can wake whenever:

- a coder adds a request,
- a dongle is released,
- a cooldown expires,
- the monitor stops the simulation.

## Starvation

Starvation means a thread waits forever while others continue making progress.

FIFO prevents starvation for a single queue because every request eventually
reaches the front. EDF is more subtle: it prioritizes urgent deadlines. If the
system is overloaded or deadlines are impossible, burnout can still happen.

"EDF prevents burnout when viable" means the timing parameters must allow enough
total work to be completed before deadlines.

## Further Reading

- Priority queue: <https://en.wikipedia.org/wiki/Priority_queue>
- Binary heap: <https://en.wikipedia.org/wiki/Binary_heap>
- Earliest Deadline First: <https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling>
- Real-time scheduling: <https://en.wikipedia.org/wiki/Real-time_computing>

## Navigation

- Previous: [Condition Variables](./condition-variables.md)
- Next: [Monitor Thread and Timing](./monitor-and-timing.md)
