# Schedulers: FIFO and EDF

The condition-variable page left us with sleeping coders. They are not spinning
uselessly anymore. They are waiting for the shared world to change.

But now a new question appears:

```text
When several coders are waiting, whose condition should become true first?
```

That question is scheduling.

A scheduler is not only an operating-system concept. In Codexion, your program
is itself a small scheduler for dongles. It receives requests, remembers them,
orders them, and grants resources according to a rule.

## Waiting Needs A Law

Imagine three coders are waiting for the same dongle. The dongle becomes
available. If your program simply broadcasts and lets the fastest awakened
thread take it, the policy is accidental. The winner depends on OS timing, CPU
load, and small implementation details.

The subject does not allow accidental policy. It gives two explicit policies:

```text
fifo: first request in line wins
edf:  request with earliest burnout deadline wins
```

Both policies answer the same question, but they define "most deserving" in
different ways.

FIFO says: the coder who has waited longest should go first.

EDF says: the coder closest to burnout should go first.

Neither rule is universally "better." They optimize different meanings of
fairness.

## A Request Is A Memory Of Waiting

The scheduler cannot choose correctly unless it remembers why each coder is
waiting. That memory is a request.

A practical request might look like this:

```c
typedef struct s_request
{
	int		coder_id;
	long	arrival_ms;
	long	deadline_ms;
	long	sequence;
}	t_request;
```

This struct is small, but it carries the scheduler's whole view of a waiting
coder.

`arrival_ms` answers: when did this coder join the line?

`deadline_ms` answers: when will this coder burn out if it does not compile?

`sequence` is a tie-breaker. Real programs should avoid vague ties because
vague ties make behavior hard to reason about and hard to test.

## FIFO: Fairness As Arrival Order

FIFO means First In, First Out. The first request added to the queue is the first
request served.

If coder 3 requested a dongle at time 100 and coder 4 requested it at time 120,
FIFO chooses coder 3. It does not ask which coder is closer to burnout. It only
honors arrival order.

The comparator idea is:

```c
if (a.arrival_ms != b.arrival_ms)
	return (a.arrival_ms < b.arrival_ms);
return (a.sequence < b.sequence);
```

FIFO is easy to explain and has a nice progress property for one queue: if new
requests are added behind old requests, an old request eventually reaches the
front.

## EDF: Fairness As Urgency

EDF means Earliest Deadline First. It chooses the request whose deadline is
closest.

In Codexion, a coder's deadline is:

```text
last_compile_start + time_to_burnout
```

If coder 3 joined the line first but coder 4 will burn out sooner, EDF chooses
coder 4. That may feel unfair from an arrival-order perspective, but it is fair
from a survival perspective. EDF is trying to keep the system alive.

The comparator idea is:

```c
if (a.deadline_ms != b.deadline_ms)
	return (a.deadline_ms < b.deadline_ms);
return (a.sequence < b.sequence);
```

The subject's statement that EDF should prevent burnout "when parameters are
viable" matters. Scheduling cannot perform miracles. If compilation, cooldown,
debugging, and refactoring times make the workload impossible, no comparator can
save every coder. EDF is powerful when the system has enough time but needs good
priority choices.

## Why The Heap Appears

Now the heap becomes natural.

The scheduler repeatedly needs to ask:

```text
Who is currently at the front of the line?
```

For FIFO, "front" means oldest arrival. For EDF, "front" means earliest
deadline. A min-heap is a data structure that keeps the best request at the top.

Internally it can be stored as an array:

```text
index:  0  1  2  3  4  5
value: [A, B, C, D, E, F]
```

The parent and child positions are calculated:

```text
parent(i) = (i - 1) / 2
left(i)   = 2 * i + 1
right(i)  = 2 * i + 2
```

The heap property says that every parent is at least as important as its
children according to your comparator. Therefore, the best request is always at
index `0`.

This is why push and pop are efficient. When a request enters, it moves upward
until the heap property is restored. When the winner leaves, another request
moves downward until the heap property is restored.

The subject requires a heap because the scheduler is not allowed to be a vague
idea. It must have a concrete memory structure.

## Granting A Dongle

A scheduler decision is only valid if the resource is actually grantable.

For a dongle, the scheduler must reason over several facts at once:

```text
the dongle is not owned
cooldown has expired
there is at least one waiting request
the top request is still valid
the simulation has not stopped
```

This decision belongs inside synchronized state, because another thread changing
one of those facts can invalidate the answer.

Once the scheduler grants the dongle, it changes shared reality. It might set an
owner field, remove a request from the heap, and broadcast so the chosen coder
can wake up and recheck its condition.

Notice the connection to the previous page. The condition variable wakes coders,
but the scheduler changes the truth that makes one coder's condition succeed.

## Starvation And Progress

Safety says a wrong thing never happens: two coders do not own one dongle.

Progress says the program does not freeze: waiting coders eventually move when
the system makes that possible.

Scheduling is mostly about progress. FIFO protects progress by respecting the
line. EDF protects progress by giving urgent coders priority. Each policy has a
different failure mode. FIFO can be too slow for a coder whose deadline is near.
EDF can keep favoring urgent work, but if the workload is viable, that urgency
is exactly what prevents burnout.

When you defend your project, do not describe FIFO and EDF as names. Describe
what value each policy optimizes.

## What To Carry Forward

The scheduler is the answer to organized waiting.

```text
condition variable: lets coders sleep
request: records why a coder is waiting
heap: keeps the best request visible
policy: defines what "best" means
scheduler: changes shared state so the chosen coder can proceed
```

The next page adds the final observer: the monitor. The scheduler tries to make
good choices before deadlines. The monitor checks whether time has already run
out.

## Further Reading

- Priority queue: <https://en.wikipedia.org/wiki/Priority_queue>
- Binary heap: <https://en.wikipedia.org/wiki/Binary_heap>
- Earliest Deadline First: <https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling>
- Real-time scheduling: <https://en.wikipedia.org/wiki/Real-time_computing>

## Navigation

- Previous: [Condition Variables](./condition-variables.md)
- Next: [Monitor Thread and Timing](./monitor-and-timing.md)
