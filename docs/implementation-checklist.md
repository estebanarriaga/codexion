# Implementation Checklist

This checklist is not a separate theory. It is the previous documents turned
into build order.

When you implement Codexion, try not to begin with threads. Begin with the
world. Threads only make sense after the shared world has a shape.

## 1. Define The World

First, parse the arguments and build a valid simulation model.

The program should accept exactly the required arguments, reject invalid
integers, reject an invalid scheduler, and store time values in a type that is
safe for millisecond arithmetic. If bad input enters the model, later
concurrency bugs become harder to diagnose.

Then allocate the main objects:

```text
simulation
coders
dongles
wait queues or heaps
mutexes
condition variables
```

At the end of initialization, the world should already be coherent, even before
threads exist:

```text
every coder has an id
every coder has an initial last_compile_start
every dongle is unowned
every dongle is not cooling
every queue is empty
sim->stop is false
```

This gives the threads a stable reality to enter.

## 2. Create Actors Only After Their Memory Is Stable

Once the shared world exists, create one thread per coder and one monitor thread.
Pass each coder thread a stable pointer, usually `&sim->coders[i]`, not the
address of a loop variable.

The lifetime should stay clear:

```text
initialize everything
create threads
let them run
set stop when the simulation ends
join every thread
destroy synchronization
free memory
```

Joining is not optional cleanup. It is the point where the main thread knows no
worker can still touch the shared world.

## 3. Protect Resource Transitions

The first deep implementation problem is dongle ownership.

A correct dongle transition preserves this truth:

```text
a dongle has at most one owner, and a coder compiles only with two legal dongles
```

That transition should happen under the mutex that protects dongle state,
cooldown state, queue state, and stop state. Do not protect only the assignment.
Protect the decision that makes the assignment correct.

When releasing a dongle, do not immediately treat it as reusable. Set its
cooldown deadline:

```text
cooldown_until = now + dongle_cooldown
```

From then on, any acquisition logic must treat the dongle as unavailable until
that time has passed.

## 4. Add Waiting Without Busy Waiting

If a coder cannot proceed, it should not spin in a loop. It should wait on a
condition variable while the real condition remains false.

The pattern is:

```text
lock
while cannot proceed and simulation has not stopped:
    condition-wait
perform the state transition if still allowed
unlock
```

Every waiting predicate should include `sim->stop`, otherwise a coder can remain
asleep after the monitor ends the simulation.

Whenever a thread changes shared state that may unblock another thread, it
should broadcast on the relevant condition variable after the state change.

## 5. Give Waiting A Scheduler

After waiting works, make it lawful.

Each request needs enough information for the active policy:

```text
coder id
arrival time
burnout deadline
tie-break sequence
```

The heap should expose the current best request. For FIFO, best means earliest
arrival. For EDF, best means earliest deadline.

Be careful with stale requests. If your design allows a request to become
invalid because a coder stopped waiting, the scheduler must detect and discard
it. If your design prevents stale requests entirely, be ready to explain how.

The scheduler's job is to change shared state so the chosen coder's waiting
condition becomes true. The condition variable wakes threads; the scheduler
decides which thread has a reason to continue.

## 6. Add The Monitor

The monitor protects the meaning of time.

For each coder, it computes:

```text
deadline = last_compile_start + time_to_burnout
```

If `now` passes that deadline, the monitor sets `sim->stop`, prints exactly one
burnout message, and wakes blocked threads so they can exit.

The monitor reads state that coder threads write, so those reads belong under
the same synchronization discipline. The 10 ms logging requirement also means
the monitor cannot sleep for long intervals between checks.

## 7. Serialize The Log

The log is the public history of the simulation. It needs two protections.

First, printing itself must be serialized so lines do not mix. A log mutex is a
simple solution.

Second, the decision to print must respect simulation state. After stop, normal
state messages should not continue. The burnout message should appear once.

So logging is not merely:

```text
protect printf
```

It is:

```text
protect the consistency of the printed history
```

## 8. Clean Up In Reverse

When all threads have exited, destroy what you initialized and free what you
allocated.

The order matters. Do not destroy a mutex or condition variable while a thread
could still use it. Do not free coders, dongles, or queues before joining the
threads that received pointers to them.

## Defense Questions

These are the questions that prove you understand the implementation:

```text
What shared truth does your main mutex protect?
Where can a coder sleep, and what wakes it?
Why does every condition wait use while instead of if?
How do you prevent a coder from holding one dongle forever?
What exactly is stored in a scheduler request?
How does FIFO choose differently from EDF?
How is dongle cooldown represented?
Who owns the burnout log message?
What happens after sim->stop becomes true?
What is special about the one-coder case?
```

If you can answer these from your own code, the project is no longer a pile of
pthread calls. It is a designed concurrent system.

## Navigation

- Previous: [Testing Concurrent Code](./testing.md)
- Next: [Real-World Threading Case Studies](./real-world-threading.md)
