# Synchronization Primitives

The previous page gave us the important shock: after `pthread_create`, there is
no single timeline. The main thread, coder threads, and monitor thread can all
run in orders you did not write explicitly.

Synchronization is how you recover a controlled order where the program needs
one.

Do not think of synchronization as "sprinkling locks around variables." Think of
it as creating small rooms where the shared world is stable long enough to make
a correct decision.

## The Gap Between Seeing And Doing

Most concurrency bugs live in the gap between observing a fact and acting on
that fact.

Consider the simplest possible dongle check:

```c
if (dongle->in_use == 0)
	dongle->in_use = 1;
```

In a single-threaded program, this is easy to reason about. If `in_use` is zero,
the code sets it to one. No one else can interfere.

In a threaded program, two coders can both reach the `if` before either one
writes the new value:

```text
coder 1 reads in_use == 0
coder 2 reads in_use == 0
coder 1 writes in_use = 1
coder 2 writes in_use = 1
```

The final value looks harmless. It is just `1`. But the meaning is broken:
two coders now believe they took the same dongle. The variable does not show the
bug clearly; the broken invariant does.

That is why concurrency must be taught in terms of shared truths. The truth
"only one coder owns this dongle" was not protected.

## Critical Sections

A critical section is a region of code where a thread observes or changes shared
state that must stay coherent.

For the dongle check, the critical section is not only the assignment. It is the
whole decision:

```c
pthread_mutex_lock(&sim->mutex);
if (dongle->in_use == 0)
	dongle->in_use = 1;
pthread_mutex_unlock(&sim->mutex);
```

Now only one thread at a time can enter that decision. Coder 1 may see the
dongle as free and take it. Coder 2 enters later and sees the updated state.

This is the first synchronization idea:

```text
When a decision depends on shared state, protect the whole decision.
```

## Communication Through Shared State

Threads in Codexion communicate mostly by changing shared fields.

When a coder begins compiling, it updates `last_compile_start`. Later, the
monitor reads that value to decide whether the coder has burned out. When the
monitor detects burnout, it sets `sim->stop`. Later, coders read `sim->stop` and
exit.

This is communication, even though no message is being sent:

```text
coder writes last_compile_start -> monitor reads it
monitor writes stop             -> coders read it
```

Because this communication happens through shared memory, both sides must agree
on the synchronization rule. If the coder writes a field without the mutex and
the monitor reads it with the mutex, the rule is incomplete. The point is not
that "the monitor used a lock." The point is that every participant in the
shared truth uses the same protection.

## Two Different Needs: Exclusion And Waiting

Codexion needs two kinds of synchronization.

First, it needs exclusion. Sometimes only one thread may inspect and update a
piece of shared reality. A mutex solves that problem. It says: while I am making
this decision, nobody else may make a conflicting decision using the same state.

Second, it needs waiting. A coder may not be able to take a dongle now, but it
should not burn CPU in a loop asking the same question millions of times. A
condition variable solves that problem. It says: sleep until someone changes the
world in a way that might make your condition true.

The distinction is important:

```text
mutex:              protects the shared state while you inspect/change it
condition variable: lets you sleep until protected state may have changed
```

A condition variable does not contain the truth. The truth is still in your
state: owner fields, cooldown timestamps, queue entries, stop flag. The condition
variable is only the doorbell.

## Do Not Freeze The World While Working

Synchronization should make decisions safe, not make the whole simulation stand
still.

This is a common mistake:

```c
pthread_mutex_lock(&sim->mutex);
/* mark compiling */
usleep(time_to_compile * 1000);
pthread_mutex_unlock(&sim->mutex);
```

The coder holds the mutex during the whole compilation. That means the monitor
may be unable to inspect deadlines, other coders may be unable to update their
state, and the scheduler may be unable to grant resources. The program becomes
technically synchronized but behaviorally wrong.

The better idea is:

```c
pthread_mutex_lock(&sim->mutex);
/* update shared state: owns dongles, starts compiling, last_compile_start */
pthread_mutex_unlock(&sim->mutex);

/* simulate the passage of compile time without holding the shared-state lock */
sleep_ms(time_to_compile);
```

Hold a mutex while making shared reality coherent. Release it while doing work
that merely takes time.

## The Rule To Carry Forward

Synchronization is not a set of magic functions. It is a discipline:

```text
Find the shared truth.
Protect the whole decision that reads or changes it.
Sleep when the truth is not yet favorable.
Wake others after changing the truth.
Never hold the shared-world lock just because time is passing.
```

The next page narrows this idea to mutexes. A mutex is simple as an API, but the
engineering question is subtle: what exactly is the mutex protecting?

## Further Reading

- Mutual exclusion: <https://en.wikipedia.org/wiki/Mutual_exclusion>
- Race condition: <https://en.wikipedia.org/wiki/Race_condition>
- POSIX mutexes: <https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html>
- POSIX condition variables: <https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html>

## Navigation

- Previous: [POSIX Threads Syntax](./pthread-syntax.md)
- Next: [Mutexes and Shared State](./mutexes.md)
