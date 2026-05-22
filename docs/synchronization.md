# Synchronization Primitives

Synchronization is how threads coordinate access to shared data and shared resources.

Codexion mainly uses:

- `pthread_mutex_t`,
- `pthread_cond_t`,
- shared flags and state protected by mutexes,
- a serialized logger.

## Race Conditions

A race condition happens when program correctness depends on unpredictable timing between threads.

Example:

```c
if (dongle->in_use == 0)
	dongle->in_use = 1;
```

With two threads, both can read `in_use == 0` before either writes `1`. Both then
believe they own the same dongle.

Protected version:

```c
pthread_mutex_lock(&sim->mutex);
if (dongle->in_use == 0)
	dongle->in_use = 1;
pthread_mutex_unlock(&sim->mutex);
```

Now only one thread at a time can test and change the state.

## Mutex Versus Condition Variable

A mutex answers:

> May I enter this critical section?

A condition variable answers:

> Can I sleep until something about the protected state may have changed?

The condition variable does not store the condition itself. Your shared state
stores the condition.

For example, this condition is state:

```c
dongle->owner == coder->id
```

The condition variable only lets the coder sleep efficiently until another thread
changes state and broadcasts/signals.

## Thread-Safe Communication

Coder threads and the monitor thread communicate through shared state:

- coder updates `last_compile_start` when compilation begins,
- monitor reads `last_compile_start` to detect burnout,
- monitor sets `sim->stop` when someone burns out,
- coder checks `sim->stop` to exit.

All of this must happen under a mutex, otherwise the monitor can read half-updated
or stale state.

## Avoid Holding Locks During Work

Do not hold the main simulation mutex while sleeping for compile/debug/refactor.

Bad:

```c
pthread_mutex_lock(&sim->mutex);
log_state(sim, coder, "is compiling");
usleep(time_to_compile * 1000);
pthread_mutex_unlock(&sim->mutex);
```

This blocks other threads from updating state for the whole compile duration.

Better:

```c
pthread_mutex_lock(&sim->mutex);
/* mark state as compiling, record last_compile_start */
pthread_mutex_unlock(&sim->mutex);
log_state(sim, coder, "is compiling");
sleep_ms(time_to_compile);
```

The exact structure depends on your design, but the principle is stable: hold
locks only while reading or mutating shared state.

## Further Reading

- Mutual exclusion: <https://en.wikipedia.org/wiki/Mutual_exclusion>
- Race condition: <https://en.wikipedia.org/wiki/Race_condition>
- POSIX mutexes: <https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html>
- POSIX condition variables: <https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html>

## Navigation

- Previous: [POSIX Threads Syntax](./pthread-syntax.md)
- Next: [Mutexes and Shared State](./mutexes.md)
