# Testing Concurrent Code

By now the project has a clear shape. Coders are threads. Dongles are shared
resources. Mutexes protect shared truths. Condition variables let coders sleep.
The scheduler chooses who may proceed. The monitor watches time.

Testing asks whether that understanding survives contact with the operating
system scheduler.

In a single-threaded program, one passing run tells you a lot. In a concurrent
program, one passing run tells you less, because a bug may only appear under a
different timing. The same code can interleave differently on the next run.

So the goal of testing Codexion is not merely:

```text
Does the program print something plausible once?
```

The better question is:

```text
Do the shared truths stay true under many timings?
```

## Test The Edges First

Start with inputs that check whether the program understands its contract before
threads complicate the picture.

Invalid inputs should fail cleanly:

```sh
./codexion
./codexion 4 800 200 200 200 3 0 invalid
./codexion -1 800 200 200 200 3 0 fifo
./codexion 4 abc 200 200 200 3 0 fifo
```

These tests are not about concurrency yet. They protect the boundary of the
simulation. If invalid parameters enter the model, later failures become harder
to interpret.

Then test the smallest world:

```sh
./codexion 1 800 200 200 200 1 0 fifo
```

The single-coder case is special because one coder cannot acquire two distinct
dongles if the subject says only one dongle exists. This case forces you to
decide what the model means at its smallest size.

## Test The Normal Story

Next, run cases that should make ordinary progress:

```sh
./codexion 4 1000 150 150 150 3 0 fifo
./codexion 4 1000 150 150 150 3 0 edf
```

Here you are asking whether the full cycle works:

```text
take dongles -> compile -> release -> debug -> refactor -> repeat
```

The log should feel boring. Boring is good. Every compile should be preceded by
two dongle-taken messages. Debugging should follow compiling. Refactoring should
follow debugging. The program should eventually stop when the required compile
count is reached.

## Test Pressure

After the normal story works, increase pressure. Cooldown is pressure because a
released dongle is still temporarily unusable:

```sh
./codexion 4 1000 150 150 150 3 100 fifo
./codexion 4 1000 150 150 150 3 100 edf
```

These tests ask whether your program really models resource history. If cooldown
is implemented incorrectly, dongles may be reused too early or coders may sleep
forever waiting for a change that nobody signals.

Then test a case where burnout is likely:

```sh
./codexion 4 200 150 150 150 3 100 fifo
```

This asks whether the monitor can end the simulation cleanly. After `burned out`
appears, normal progress messages should not keep appearing as if the world were
still alive.

## Read Logs As Evidence

The log is not just output. It is evidence about whether the shared truths were
preserved.

For every compile, look for this shape:

```text
X has taken a dongle
X has taken a dongle
X is compiling
```

Then ask what each line proves. The two taken messages should correspond to two
legal dongle grants. The compile line should mean the coder really owns both
dongles. The later debug line should mean both dongles were released and put
into cooldown.

You are looking for violations of the model:

```text
two neighboring coders compiling with the same dongle
compilation after only one dongle was taken
cooldown ignored
normal messages after burnout
mixed or broken log lines
program never exits
burnout logged much later than the deadline
```

A good test habit is to connect every strange log line back to one protected
truth. Ask: which truth became false?

## Repeat Runs Because Timing Moves

A race condition may hide for many runs and then appear once. That does not make
it random magic. It means the bad interleaving only happened under one timing.

Run the same command many times:

```sh
i=0
while [ $i -lt 100 ]
do
	./codexion 5 800 100 100 100 5 20 edf >/tmp/codexion.log || break
	i=$((i + 1))
done
```

If run 37 fails, the failure is real. The first 36 runs did not prove the
program correct; they only failed to expose the bug.

Changing numbers is also useful. Small times create timing pressure. Larger
coder counts create scheduling pressure. Cooldown creates resource-history
pressure. Use parameters to stress one idea at a time.

## Use Tools As Witnesses

Some bugs are hard to see from logs. If your platform supports them, sanitizers
and thread-checking tools can help.

ThreadSanitizer can report data races:

```sh
cc -g -fsanitize=thread -pthread ...
```

AddressSanitizer can report memory mistakes:

```sh
cc -g -fsanitize=address ...
```

Helgrind, from Valgrind, can also help detect threading mistakes on supported
systems.

These tools are not substitutes for understanding. They are witnesses. If a tool
says two threads access the same field without consistent synchronization, bring
that report back to the model and ask which shared truth lacks a clear mutex
boundary.

## What To Carry Forward

Testing concurrent code is testing a theory of time.

Your theory says:

```text
this mutex protects this truth
this condition variable wakes these waiters
this heap chooses this request
this monitor catches this deadline
this stop flag ends this world
```

The tests try to falsify that theory.

The next page turns the theory into an implementation checklist. By then, each
checklist item should feel less like a task and more like a consequence of the
model.

## Further Reading

- ThreadSanitizer C++ manual, still useful conceptually for C: <https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual>
- AddressSanitizer: <https://github.com/google/sanitizers/wiki/AddressSanitizer>
- Helgrind manual: <https://valgrind.org/docs/manual/hg-manual.html>

## Navigation

- Previous: [Monitor Thread and Timing](./monitor-and-timing.md)
- Next: [Implementation Checklist](./implementation-checklist.md)
