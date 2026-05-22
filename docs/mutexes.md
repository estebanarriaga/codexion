# Mutexes and Shared State

A mutex is a lock. Only one thread can hold it at a time.

In Codexion, a mutex should protect meaningful shared state:

- dongle state,
- wait queues,
- coder state,
- monitor stop flag,
- log output.

## Basic Syntax

```c
pthread_mutex_t	mutex;

pthread_mutex_init(&mutex, NULL);
pthread_mutex_lock(&mutex);
/* critical section */
pthread_mutex_unlock(&mutex);
pthread_mutex_destroy(&mutex);
```

Always unlock every path that leaves the critical section.

## One Global Mutex Or Per-Dongle Mutexes?

Both designs can work.

A single simulation mutex is simpler:

- easier to reason about,
- fewer lock-order bugs,
- enough for a small project,
- may reduce parallelism.

Per-dongle mutexes can be more scalable:

- independent dongles can be accessed concurrently,
- lock ordering becomes harder,
- acquiring two dongles risks deadlock unless carefully designed.

For this project, a central scheduler protected by one simulation mutex is often
easier to defend because FIFO/EDF decisions need consistent queue state.

## Deadlock

A deadlock happens when threads wait forever for each other.

Classic example:

```text
coder 1 holds dongle 1 and waits for dongle 2
coder 2 holds dongle 2 and waits for dongle 1
```

The four Coffman conditions for deadlock are:

- mutual exclusion,
- hold and wait,
- no preemption,
- circular wait.

Break at least one condition to prevent deadlock.

Common Codexion strategies:

- central scheduler grants both dongles only when both are available,
- impose a total order on dongle acquisition,
- release the first dongle if the second cannot be acquired,
- avoid blocking while holding one dongle forever.

Because the subject requires arbitration per dongle, be careful with a naive
"take left then right" approach. It can satisfy mutual exclusion but still cause
deadlock or starvation.

## Log Mutex

Use a dedicated log mutex or ensure logging is protected by your simulation
mutex. A dedicated log mutex is common:

```c
pthread_mutex_lock(&sim->log_mutex);
printf("%ld %d %s\n", now_ms, coder_id, message);
pthread_mutex_unlock(&sim->log_mutex);
```

This prevents mixed lines such as:

```text
120 1 is comp121 2 is debugging
iling
```

Also protect the rule that no extra state messages are printed after stop.

## Further Reading

- Deadlock: <https://en.wikipedia.org/wiki/Deadlock>
- Coffman conditions: <https://en.wikipedia.org/wiki/Deadlock#Necessary_conditions>
- POSIX mutex lock: <https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html>

## Navigation

- Previous: [Synchronization Primitives](./synchronization.md)
- Next: [Condition Variables](./condition-variables.md)
