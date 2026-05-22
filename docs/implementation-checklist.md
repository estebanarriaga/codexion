# Implementation Checklist

Use this checklist while building Codexion.

## Parsing

- Accept exactly 8 arguments after the program name.
- Reject negative numbers.
- Reject non-integers.
- Reject zero if your interpretation requires strictly positive values.
- Reject scheduler values other than `fifo` and `edf`.
- Store time values in a type large enough for millisecond arithmetic.

## Initialization

- Allocate coders.
- Allocate dongles.
- Initialize all mutexes.
- Initialize all condition variables.
- Initialize wait queues/heaps.
- Set every coder's initial `last_compile_start` to simulation start.
- Set every dongle as free and not cooling.

## Threads

- Create one thread per coder.
- Create one monitor thread.
- Join all threads before cleanup.
- Ensure threads can exit when `sim->stop` becomes true.

## Dongles

- Protect dongle state with mutexes.
- Enforce at most one owner per dongle.
- Enforce cooldown after release.
- Do not grant a cooling dongle before `cooldown_until`.
- Use the selected scheduler to choose among waiting requests.

## Scheduler

- Implement a min-heap priority queue.
- FIFO comparator uses arrival order.
- EDF comparator uses burnout deadline.
- Add deterministic tiebreakers.
- Remove stale/canceled requests if your design can create them.

## Monitor

- Detect burnout from `last_compile_start + time_to_burnout`.
- Print burnout within 10 ms.
- Stop the simulation exactly once.
- Wake blocked coder/scheduler waits after stop.

## Logging

- Protect output with a mutex.
- Print timestamps in milliseconds since simulation start.
- Do not interleave messages.
- Do not print normal state messages after stop.
- Print exactly one burnout message.

## Cleanup

- Free all allocated memory.
- Destroy all mutexes.
- Destroy all condition variables.
- Avoid destroying synchronization objects before all users have exited.

## Defense Questions To Practice

- Why can a data race happen even on an `int`?
- What invariant does your main mutex protect?
- Why must `pthread_cond_wait` use a `while` loop?
- How does your implementation avoid deadlock?
- How does FIFO differ from EDF in your heap comparator?
- How do you enforce dongle cooldown?
- How does the monitor guarantee burnout detection timing?
- What happens when there is only one coder?

## Navigation

- Previous: [Testing Concurrent Code](./testing.md)
- Next: [Real-World Threading Case Studies](./real-world-threading.md)
