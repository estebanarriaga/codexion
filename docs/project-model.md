# Project Model

Codexion models a shared-resource problem.

There are `number_of_coders` coder threads and the same number of dongles. Each
coder has a left and right dongle. To compile, a coder must hold both adjacent
dongles at the same time.

The coder lifecycle is:

```text
wait for dongles
take left/right dongles
compile
release dongles
debug
refactor
repeat
```

The simulation stops when either:

- one coder burns out, or
- every coder has compiled at least `number_of_compiles_required` times.

## How This Differs From Basic Philosophers

Classic Dining Philosophers already teaches deadlock, starvation, and mutual
exclusion. Codexion adds several constraints that force a more explicit design:

- **Cooldown:** after release, a dongle cannot be immediately reused.
- **Scheduler policy:** when multiple coders wait for a dongle, the winner is not
  arbitrary. It must follow `fifo` or `edf`.
- **Required heap:** the waiting queue must be priority-based.
- **Monitor thread:** burnout detection cannot be left to coder threads only.
- **Precise logs:** burnout must be logged within 10 ms of the real deadline.

## Important Shared State

Typical shared state includes:

- global stop flag,
- simulation start time,
- each coder's current state,
- each coder's `last_compile_start`,
- each coder's compile count,
- each dongle's owner or availability,
- each dongle's cooldown deadline,
- each dongle's wait queue,
- the log output.

If two threads can read/write the same state concurrently, protect it with a
mutex or redesign ownership so only one thread mutates it.

## Invariants

An invariant is a fact that must always remain true when shared state is visible.
Good concurrent code is built around protecting invariants.

Useful Codexion invariants:

- A dongle is held by at most one coder.
- A coder compiles only while holding two dongles.
- A dongle in cooldown cannot be granted before `cooldown_until`.
- A request exists at most once in a given dongle queue.
- No normal state log is printed after the simulation has stopped.
- Only one burnout log is printed.

The mutex should protect the whole invariant. For example, if `in_use`,
`cooldown_until`, and `wait_queue` together determine dongle availability, then
checking and modifying them should happen under the same mutex.

## Further Reading

- Dining Philosophers: <https://en.wikipedia.org/wiki/Dining_philosophers_problem>
- Race condition: <https://en.wikipedia.org/wiki/Race_condition>
- Deadlock: <https://en.wikipedia.org/wiki/Deadlock>

## Navigation

- Previous: [Codexion Learning Notes](./README.md)
- Next: [POSIX Threads Syntax](./pthread-syntax.md)
