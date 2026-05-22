# Mutexes and Shared State

The synchronization page ended with a question: what exactly are we protecting?

A mutex is the simplest pthread answer. Only one thread can hold a mutex at a
time. But that sentence is too mechanical to be useful by itself. The deeper
idea is this:

```text
A mutex protects a shared truth while a thread reads or changes it.
```

If you remember only the function names, you will lock too much, too little, or
in the wrong places. If you remember the truth being protected, the lock
placement starts to make sense.

## The Syntax Is Small

The API is straightforward:

```c
pthread_mutex_t	mutex;

pthread_mutex_init(&mutex, NULL);
pthread_mutex_lock(&mutex);
/* critical section */
pthread_mutex_unlock(&mutex);
pthread_mutex_destroy(&mutex);
```

The code between `lock` and `unlock` is the critical section. In Codexion, that
critical section should be the smallest piece of code that needs a stable view
of shared state.

For example, taking a dongle is not just writing `in_use = 1`. It is the whole
decision that makes the write legitimate:

```c
pthread_mutex_lock(&sim->mutex);
if (!dongle->in_use && now_ms() >= dongle->cooldown_until)
{
	dongle->in_use = 1;
	dongle->owner_id = coder->id;
}
pthread_mutex_unlock(&sim->mutex);
```

The mutex protects the relationship between `in_use`, `cooldown_until`, and
`owner_id`. It protects the rule that a coder only becomes owner when the dongle
is actually available.

## The Mutex Boundary

A useful way to design mutex use is to draw a boundary around a question.

For Codexion, questions like these need a protected boundary:

```text
May this coder take this dongle now?
Has this coder burned out?
Should the simulation stop?
Can this log line still be printed?
Who is first in this wait queue?
```

Each question depends on shared state. The mutex gives the thread a moment where
the answer cannot be changed halfway through the reasoning.

If you lock around only one variable, the answer may still be wrong. Suppose a
thread checks that a dongle is not in use, unlocks, then later checks cooldown.
Between those two checks, another coder might take the dongle. The program has
combined facts from two different moments and treated them as one coherent
world.

This is the kind of bug that makes concurrent programs feel strange. The values
you read may each have been real, but the combination may never have been real
at the same time.

## One Mutex Or Many?

A simple Codexion design can use one main simulation mutex for most shared
state. That makes reasoning easier: if you need to inspect coders, dongles,
queues, or the stop flag together, you take the same lock.

The cost is that threads wait more often. While one coder is making a scheduling
decision, another coder may have to wait even if it wants a different dongle.
For this project, that tradeoff is often acceptable because clarity matters more
than maximum throughput.

A more advanced design can use several mutexes, perhaps one per dongle plus one
for the monitor state and one for logging. That can allow more parallelism, but
it introduces a new problem: if a thread ever needs two locks, the order matters.

For example:

```text
coder 1 locks dongle A, then waits for dongle B
coder 2 locks dongle B, then waits for dongle A
```

Neither can continue. This is deadlock.

Multiple mutexes are not bad, but they demand a locking discipline. You must be
able to explain which lock protects which truth and in what order locks may be
taken.

## Deadlock Is A Design Failure, Not Bad Luck

Deadlock means threads are waiting forever in a cycle. In Codexion, the classic
version is a coder holding one dongle while waiting for the other, while the
neighbor does the symmetrical thing.

The way out is to break the cycle by design. You can do that by giving a central
scheduler authority, by acquiring dongles in a strict global order, or by
releasing a partial acquisition when the complete acquisition cannot be made.

The exact strategy is an implementation choice. The important lesson is that
"I protected each dongle with a mutex" is not enough. Mutual exclusion prevents
two owners for one dongle, but it does not automatically guarantee progress.

Correct concurrent design needs both:

```text
safety: bad things do not happen
progress: good things eventually happen
```

A mutex mostly helps with safety. Your acquisition strategy and scheduler help
with progress.

## Logging Is Also Shared Reality

The log is part of the simulation's truth. If two threads print at the same
time, their messages can mix into one unreadable line. A log mutex prevents
that:

```c
pthread_mutex_lock(&sim->log_mutex);
printf("%ld %d %s\n", elapsed_ms, coder_id, message);
pthread_mutex_unlock(&sim->log_mutex);
```

But the log mutex only protects the physical printing. The decision whether a
message is still allowed may depend on `sim->stop`. That decision also needs
synchronization.

For example, after burnout, the subject expects no normal state messages to keep
appearing. So a good logging design does not merely ask "can I print without
interleaving?" It also asks "is this message still valid in the simulation's
history?"

## What To Carry Forward

A mutex is not a decoration around a variable. It is a boundary around a shared
decision.

When placing a mutex in Codexion, ask:

```text
What truth am I preserving?
Which fields together define that truth?
Can another thread change one of those fields while I decide?
Am I holding the lock only as long as the decision needs?
Could this lock order create deadlock?
```

The next page introduces condition variables. A mutex lets a thread make a safe
decision. A condition variable lets a thread sleep when the safe decision says:
"not yet."

## Further Reading

- Deadlock: <https://en.wikipedia.org/wiki/Deadlock>
- Coffman conditions: <https://en.wikipedia.org/wiki/Deadlock#Necessary_conditions>
- POSIX mutex lock: <https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html>

## Navigation

- Previous: [Synchronization Primitives](./synchronization.md)
- Next: [Condition Variables](./condition-variables.md)
