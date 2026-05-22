# Condition Variables

A condition variable lets a thread sleep until another thread announces that
shared state may have changed.

In Codexion, condition variables are useful for:

- waking coders when dongles may be available,
- waking a scheduler when new requests arrive,
- waiting until cooldown expires,
- waking all threads when the monitor stops the simulation.

## Basic Pattern

The correct pattern is:

```c
pthread_mutex_lock(&mutex);
while (!predicate_is_true())
	pthread_cond_wait(&cond, &mutex);
/* predicate is true and mutex is locked */
pthread_mutex_unlock(&mutex);
```

Use `while`, not `if`.

Reasons:

- wakeups can be spurious,
- another thread may consume the resource first,
- a broadcast wakes many threads but only some can proceed,
- the stop flag may have changed.

## What `pthread_cond_wait` Really Does

`pthread_cond_wait(&cond, &mutex)` atomically:

1. unlocks `mutex`,
2. puts the thread to sleep,
3. wakes after signal/broadcast,
4. locks `mutex` again before returning.

That atomic unlock-and-sleep step is why condition variables avoid lost wakeups
when used correctly.

## Signal Versus Broadcast

`pthread_cond_signal` wakes at least one waiting thread.

`pthread_cond_broadcast` wakes all waiting threads.

The subject authorizes `pthread_cond_broadcast`, not `pthread_cond_signal`.
Broadcast is fine when every waiting thread rechecks its own predicate in a
`while` loop.

Use broadcast when:

- a dongle is released,
- cooldown may have expired,
- the scheduler changed ownership,
- `sim->stop` is set.

## Timed Wait And Cooldown

`pthread_cond_timedwait` waits until either:

- the condition variable is signaled/broadcast, or
- an absolute timeout is reached.

This is useful for dongle cooldown:

```c
while (!sim->stop && now_ms() < dongle->cooldown_until)
	pthread_cond_timedwait(&dongle->cond, &sim->mutex, &deadline_ts);
```

Important: POSIX timed waits usually expect an absolute `struct timespec`
deadline, not a relative duration.

The allowed time function in this subject is `gettimeofday`. You can convert
milliseconds to a `struct timespec` deadline when needed.

## Common Condition Variable Bugs

- Waiting without holding the mutex.
- Using `if` instead of `while`.
- Treating the condition variable itself as the state.
- Forgetting to broadcast when the stop flag changes.
- Broadcasting before updating the protected state.
- Using different mutexes for the same condition predicate.

## Further Reading

- POSIX `pthread_cond_wait`: <https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html>
- Condition variable concept: <https://en.wikipedia.org/wiki/Monitor_(synchronization)#Condition_variables>
- `pthread_cond_timedwait`: <https://man7.org/linux/man-pages/man3/pthread_cond_timedwait.3p.html>

## Navigation

- Previous: [Mutexes and Shared State](./mutexes.md)
- Next: [Schedulers: FIFO and EDF](./schedulers.md)
