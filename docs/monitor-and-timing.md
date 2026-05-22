# Monitor Thread and Timing

The subject requires a separate monitor thread that detects burnout and stops the
simulation.

Burnout rule:

```text
if a coder does not begin compiling within time_to_burnout ms
from the start of their previous compilation, they burn out
```

At simulation start, each coder's initial deadline is:

```text
simulation_start + time_to_burnout
```

After a coder starts compiling:

```text
last_compile_start = now
next_deadline = last_compile_start + time_to_burnout
```

## Monitor Responsibilities

The monitor thread should:

- periodically inspect every coder,
- compute each coder's deadline,
- detect the first burnout,
- set `sim->stop`,
- print exactly one `burned out` log,
- wake all waiting threads so they can exit.

The burnout log must be printed no more than 10 ms after the real burnout time.

## Time Helpers

A common helper converts `gettimeofday` to milliseconds:

```c
long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}
```

For logs, print elapsed time since simulation start:

```text
timestamp = now_ms() - sim->start_ms
```

For deadlines, absolute milliseconds are usually easier:

```text
deadline_abs = coder->last_compile_start_abs + time_to_burnout
```

## Sleeping Accurately

`usleep(ms * 1000)` is simple, but it can oversleep. For long sleeps, use small
chunks and check `sim->stop`:

```c
while (!stopped(sim) && now_ms() < end)
	usleep(500);
```

This lets the thread react quickly when the monitor stops the simulation.

Do not busy-wait in a tight loop. It wastes CPU and can make timing worse.

## Monitor Sleep Interval

Because burnout must be logged within 10 ms, the monitor cannot sleep for long
periods. Common approaches:

- sleep for 1 ms between scans,
- calculate the nearest deadline and sleep until just before it,
- use condition variables to wake the monitor when compile starts or stop changes.

The simplest defendable implementation is often:

```text
scan coders
if burnout found: stop
else usleep(1000)
```

This is not perfectly real-time, but it is usually good enough for the 10 ms
requirement on normal systems.

## Data Protected By Mutex

The monitor reads:

- `sim->stop`,
- each coder's state,
- each coder's `last_compile_start`,
- each coder's compile count.

Coder threads write some of the same fields. Use the simulation mutex around
these reads and writes.

## Further Reading

- `gettimeofday`: <https://man7.org/linux/man-pages/man2/gettimeofday.2.html>
- `usleep`: <https://man7.org/linux/man-pages/man3/usleep.3.html>
- Time of check to time of use: <https://en.wikipedia.org/wiki/Time-of-check_to_time-of-use>

## Navigation

- Previous: [Schedulers: FIFO and EDF](./schedulers.md)
- Next: [Testing Concurrent Code](./testing.md)
