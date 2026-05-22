# Monitor Thread and Timing

The scheduler tries to make good choices before a coder runs out of time. The
monitor answers the harder question:

```text
Did time already run out?
```

This is why the subject requires a separate monitor thread. If burnout detection
were left to coder threads, a coder might be asleep, blocked on a condition
variable, or waiting behind another request. The program could miss the moment
where the simulation should stop.

The monitor gives Codexion an independent observer of time.

## Deadlines Are Part Of The State

A coder burns out if it does not begin compiling within `time_to_burnout`
milliseconds from the start of its last compilation. At the beginning of the
simulation, the reference point is the simulation start time.

So each coder has a moving deadline:

```text
deadline = last_compile_start + time_to_burnout
```

When a coder begins compiling, it renews its life:

```text
last_compile_start = now
new_deadline = now + time_to_burnout
```

This is why `last_compile_start` is shared state. The coder writes it when
compilation begins. The monitor reads it to judge whether the coder is still
alive. That read/write relationship must be synchronized.

## Absolute Time And Display Time

It helps to separate two kinds of time.

Absolute time is useful for decisions:

```text
now_abs >= last_compile_start_abs + time_to_burnout
```

Elapsed time is useful for logs:

```text
elapsed = now_abs - simulation_start_abs
```

Mixing these two ideas makes timing code confusing. Store deadlines and
comparisons in absolute milliseconds. Convert to elapsed milliseconds only when
printing.

A common helper is:

```c
long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}
```

Then:

```text
deadline_abs = coder->last_compile_start_abs + params->time_to_burnout
log_time     = now_ms() - sim->start_abs
```

## What The Monitor Actually Does

The monitor loop is conceptually simple:

```text
look at every coder
compute that coder's deadline
if now is past the deadline:
    stop the simulation
    print burned out once
    wake sleeping threads
otherwise:
    wait briefly and check again
```

The difficulty is not the algorithm. The difficulty is doing it without racing
against coder updates.

Suppose the monitor reads `last_compile_start` at the same moment a coder starts
compiling and updates it. Without synchronization, the monitor could make a
burnout decision from stale or inconsistent state. With a mutex, the monitor
sees a coherent snapshot.

## The 10 Millisecond Requirement

The subject says the burnout log must appear no more than 10 ms after the real
burnout moment. This requirement shapes the monitor.

If the monitor sleeps for 50 ms between checks, it cannot guarantee a 10 ms
response. It may wake too late. So the monitor must either check frequently or
sleep until the nearest known deadline.

The simple approach is frequent scanning:

```text
scan all coders
if nobody burned out:
    usleep(1000)
```

Sleeping for about 1 ms is easy to reason about, though real operating systems
can still oversleep slightly. A more advanced approach computes the nearest
deadline and waits until just before it, but that design needs more care because
coder compile starts can move deadlines forward.

For this project, the simple scan is often easier to defend. It says: the
monitor never intentionally sleeps long enough to miss the 10 ms window.

## Sleeping Without Becoming Blind

Coder threads also sleep: compiling, debugging, and refactoring all consume
time. A naive sleep like this is simple:

```c
usleep(time_to_compile * 1000);
```

But if the monitor stops the simulation during that sleep, the coder will not
notice until the sleep ends. For long sleeps, a better helper sleeps in small
chunks and checks the stop flag:

```c
while (!stopped(sim) && now_ms() < end)
	usleep(500);
```

This is not busy waiting because the thread still sleeps. It simply wakes often
enough to react to global stop.

The same principle appeared in condition variables: waiting is allowed, but
waiting must not make the program unable to observe important changes.

## Stopping Is A Shared Truth

When the monitor detects burnout, it changes the meaning of the entire
simulation. From that moment, normal coder messages should stop, waiting threads
should wake up, and all routines should move toward exit.

That means `sim->stop` is not just an integer. It is the shared truth that the
world is over.

The monitor should set it while holding the mutex that protects simulation
state. Then it should wake waiters with condition-variable broadcasts. Sleeping
coders must include `sim->stop` in their waiting predicates so they can leave
instead of sleeping forever.

The burnout log also needs special care. There should be exactly one burnout
message. The monitor is the natural owner of that message because it is the
thread responsible for detecting burnout.

## What To Carry Forward

The monitor is not another worker. It does not try to compile. It protects the
meaning of time.

```text
coders:    try to make local progress
scheduler: decides who may use scarce resources
monitor:   decides whether deadlines have been missed
```

The scheduler prevents avoidable waiting. The monitor detects when waiting has
already become failure.

The next page moves from design to verification. Once a concurrent program
exists, testing is not only "does it run?" It is "can I catch the moments where
timing breaks my assumptions?"

## Further Reading

- `gettimeofday`: <https://man7.org/linux/man-pages/man2/gettimeofday.2.html>
- `usleep`: <https://man7.org/linux/man-pages/man3/usleep.3.html>
- Time of check to time of use: <https://en.wikipedia.org/wiki/Time-of-check_to_time-of-use>

## Navigation

- Previous: [Schedulers: FIFO and EDF](./schedulers.md)
- Next: [Testing Concurrent Code](./testing.md)
