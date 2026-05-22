# Codexion Learning Notes

These notes explain the concurrency concepts behind `codexion.subject.pdf`.
The project is a variant of the classic Dining Philosophers problem: several
coder threads compete for shared dongles, each compilation needs two dongles,
released dongles have a cooldown period, and waiting coders must be served by a
selected scheduler policy.

The goal of this documentation is not to provide a finished implementation. It
is meant to help you understand the ideas well enough to design, debug, and
defend your own code.

## Suggested Reading Order

1. [Project Model](./project-model.md)
2. [POSIX Threads Syntax](./pthread-syntax.md)
3. [Synchronization Primitives](./synchronization.md)
4. [Mutexes and Shared State](./mutexes.md)
5. [Condition Variables](./condition-variables.md)
6. [Schedulers: FIFO and EDF](./schedulers.md)
7. [Monitor Thread and Timing](./monitor-and-timing.md)
8. [Testing Concurrent Code](./testing.md)
9. [Implementation Checklist](./implementation-checklist.md)
10. [Real-World Threading Case Studies](./real-world-threading.md)

## Core Ideas In One Page

- A thread is one independent path of execution inside the same process.
- All coder threads share the same heap memory, so shared state must be protected.
- A mutex protects an invariant, not just a variable.
- A condition variable lets threads sleep until shared state may have changed.
- `pthread_cond_wait` must always be used with a mutex and a `while` predicate.
- FIFO scheduling orders requests by arrival time.
- EDF scheduling orders requests by deadline, here `last_compile_start + time_to_burnout`.
- A monitor thread should independently detect burnout and stop the simulation.
- Logs must be serialized with a mutex so lines do not interleave.

## External References

- POSIX threads overview: <https://man7.org/linux/man-pages/man7/pthreads.7.html>
- `pthread_create`: <https://man7.org/linux/man-pages/man3/pthread_create.3.html>
- POSIX mutexes: <https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html>
- POSIX condition variables: <https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html>
- `gettimeofday`: <https://man7.org/linux/man-pages/man2/gettimeofday.2.html>
- Dining Philosophers problem: <https://en.wikipedia.org/wiki/Dining_philosophers_problem>
- Earliest Deadline First scheduling: <https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling>
- Binary heap data structure: <https://en.wikipedia.org/wiki/Binary_heap>

## Navigation

- Previous: none
- Next: [Project Model](./project-model.md)
