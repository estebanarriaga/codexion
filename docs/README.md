# Codexion Learning Notes

Codexion is not mainly about learning the names of pthread functions. It is
about learning how a program stays correct when several independent executions
touch the same world at the same time.

Imagine the simulation as a small laboratory. The coders are active agents. The
dongles are scarce resources. Time is always advancing. A scheduler decides who
waits and who moves. A monitor watches whether anyone has waited too long. The
central question is always the same:

```text
What facts must remain true while many threads are trying to change them?
```

That question is the line that holds the whole project together. A mutex, a
condition variable, a heap, a monitor thread, and a scheduler are not separate
pieces of trivia. They are tools for preserving shared truths under pressure.

When you read these notes, do not try to memorize every term immediately. Follow
the progression. First understand the world of the simulation. Then understand
what a thread is. Then understand why shared memory breaks naive reasoning. Only
after that do mutexes, condition variables, scheduling, and monitoring become
natural.

## Reading Path

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

The order matters. Each document assumes the previous one has already changed
how you think about the problem.

## The One Mental Model

In a single-threaded program, you can often read code as a story: line one
happens, then line two, then line three. In a threaded program, there are
several stories being written on the same page. Correctness is not guaranteed by
the order you wrote the lines. Correctness comes from controlling when shared
facts may be observed and changed.

For Codexion, the important facts are simple to say but difficult to preserve:

```text
A dongle cannot belong to two coders.
A coder cannot compile without two dongles.
A cooling dongle cannot be reused early.
The scheduler must choose the correct waiter.
The monitor must notice burnout on time.
The log must tell one coherent history.
```

Everything else in the project is machinery to keep those facts true.

## References

Use references after you understand the local problem. Documentation is more
useful when you already know what question you are asking.

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
