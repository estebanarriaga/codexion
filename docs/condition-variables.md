# Condition Variables

The mutex page gave us a way to make a safe decision. A coder locks the shared
state, looks at the dongle, the cooldown, the queue, and the stop flag, and asks:

```text
Can I continue now?
```

Sometimes the answer is no.

That "no" is where condition variables enter. A mutex lets a thread inspect the
world safely. A condition variable lets the thread sleep until the world may
have changed.

## The Problem With Waiting

Suppose a coder wants a dongle, but the dongle is currently in use. The coder
could keep checking:

```c
while (dongle->in_use)
{
	/* try again immediately */
}
```

This is called busy waiting. It wastes CPU and can make the simulation worse by
competing with threads that could actually make progress.

The coder needs a better behavior:

```text
I cannot continue now.
Wake me when something relevant may have changed.
```

A condition variable is the pthread tool for that behavior.

## The Condition Is Not Inside The Condition Variable

This is the most important conceptual point.

A condition variable does not know whether a dongle is free. It does not know
whether cooldown expired. It does not know whether the scheduler selected this
coder.

The real condition lives in your shared state:

```text
dongle is not in use
cooldown has expired
this coder is the chosen request
simulation has not stopped
```

The condition variable is only a sleeping and waking mechanism. It is like a
bell. The bell does not contain the truth; it only tells sleepers to recheck the
truth.

## The Correct Waiting Pattern

The standard pattern looks like this:

```c
pthread_mutex_lock(&sim->mutex);
while (!can_take_dongle(sim, coder, dongle) && !sim->stop)
	pthread_cond_wait(&dongle->cond, &sim->mutex);
/* here the mutex is locked again */
pthread_mutex_unlock(&sim->mutex);
```

Read it as a story.

First, the coder locks the mutex so it can inspect shared reality safely. Then
it asks whether the condition is true. If the condition is false, it waits.
`pthread_cond_wait` releases the mutex while the thread sleeps, because other
threads must be able to change the world. When the thread wakes, it locks the
mutex again before returning.

That last detail matters. After waking, the coder is again inside a protected
decision. It can safely recheck the condition.

## Why `while`, Not `if`

The wait must be in a `while` loop, not an `if`.

This is not superstition. It follows from the meaning of a wakeup.

A wakeup does not mean "your condition is true." It means "something may have
changed." Several things can happen:

```text
another coder may take the dongle first
cooldown may still not be over
the monitor may have stopped the simulation
the wakeup may be spurious
```

So the awakened thread must ask the real question again. The `while` loop is the
code form of intellectual honesty: after every wakeup, re-check the world.

## The Atomic Sleep

`pthread_cond_wait(&cond, &mutex)` performs a delicate operation:

```text
release the mutex
go to sleep
wake later
reacquire the mutex before returning
```

The release-and-sleep step is atomic from the point of view of condition
variable use. That prevents a lost-wakeup pattern where a thread unlocks the
mutex, another thread signals, and only then the first thread starts sleeping.

This is why you should not invent your own version with `pthread_mutex_unlock`
followed by `usleep`. Condition variables solve a real timing problem.

## Broadcast In This Project

The subject authorizes `pthread_cond_broadcast`. A broadcast wakes all threads
waiting on a condition variable.

At first this may sound inefficient. Why wake everyone if only one coder can get
the dongle? Because every awakened thread must recheck its own predicate in a
`while` loop. The threads that still cannot proceed simply go back to sleep.

Broadcast is especially useful when the changed fact may matter to several
threads:

```text
a dongle was released
a cooldown may be over
the scheduler changed ownership
the monitor set sim->stop
```

The order is important: change the protected state first, then broadcast. The
wakeup is only useful if the awakened threads can see the new truth.

## Timed Waiting And Cooldown

Cooldown introduces a special kind of waiting. Sometimes no thread will release
the dongle again because it is already released; it is simply not legal to take
until a future time.

`pthread_cond_timedwait` handles this case. It lets a thread sleep until either
someone broadcasts or an absolute time is reached.

Conceptually:

```c
pthread_mutex_lock(&sim->mutex);
while (!sim->stop && now_ms() < dongle->cooldown_until)
	pthread_cond_timedwait(&dongle->cond, &sim->mutex, &cooldown_deadline);
pthread_mutex_unlock(&sim->mutex);
```

In POSIX, the timeout is normally an absolute `struct timespec`, not "sleep for
N milliseconds." Since the subject allows `gettimeofday`, you can build helpers
that convert your millisecond deadlines into the required structure.

## What To Carry Forward

A condition variable is not a second kind of mutex. It does not protect data.
It lets a thread stop running until protected data may have changed.

The full pattern is:

```text
lock the mutex
while the real condition is false:
    wait on the condition variable, which temporarily unlocks the mutex
when the condition is true:
    make the protected state transition
unlock the mutex
```

This pattern is the bridge to scheduling. If many coders are asleep waiting for
resources, the program needs a rule for deciding whose condition becomes true
first. That rule is FIFO or EDF.

## Further Reading

- POSIX `pthread_cond_wait`: <https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html>
- Condition variable concept: <https://en.wikipedia.org/wiki/Monitor_(synchronization)#Condition_variables>
- `pthread_cond_timedwait`: <https://man7.org/linux/man-pages/man3/pthread_cond_timedwait.3p.html>

## Navigation

- Previous: [Mutexes and Shared State](./mutexes.md)
- Next: [Schedulers: FIFO and EDF](./schedulers.md)
