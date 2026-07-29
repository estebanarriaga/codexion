# Project Model

Before writing pthread code, build the model in your head.

Codexion is a small world with three kinds of things: coders, dongles, and time.
The coders act. The dongles are shared resources. Time decides whether waiting
is still acceptable.

Each coder sits in a circle. There is one dongle between each pair of
neighboring coders. To compile, a coder must hold the dongle on the left and the
dongle on the right at the same time. After compiling, the coder releases both
dongles, spends some time debugging, spends some time refactoring, and then
tries to compile again.

So the visible behavior looks simple:

```text
wait for dongles -> compile -> debug -> refactor -> repeat
```

If this were a single-threaded exercise, that would almost be the whole story.
You would update coder 1, then coder 2, then coder 3. Nothing could happen "at
the same time" because your program would have only one execution path.

But Codexion is not that program. Each coder is its own thread. That means
several coders can make progress independently, and two neighboring coders can
try to take the same dongle at nearly the same moment. The project begins here:
not at `pthread_create`, but at the moment you realize the table has one shared
reality and many actors trying to change it.

## Shared Reality

The first rule of the model is that the table has only one truth.

If coder 1 owns a dongle, coder 2 must not also own it. If a dongle is cooling
down, nobody may take it yet. If the simulation has stopped, normal state
messages should not continue as if nothing happened.

These facts are not just variables. They are relationships between variables.
For example, dongle availability may depend on an owner field, an in-use flag, a
cooldown timestamp, a wait queue, and the global stop flag. If those pieces are
read separately without coordination, a thread can make a decision from a world
that never really existed.

That is the heart of concurrency: the dangerous moment is often not a single
assignment. It is the gap between checking a fact and acting on it.

## Resource History

A normal Dining Philosophers problem often treats a fork as either available or
unavailable. Codexion makes the resource more realistic. A dongle remembers its
recent past.

When a coder releases a dongle, the dongle does not immediately become usable.
It enters cooldown until a specific time. So the question "is this dongle free?"
is too weak. The real question is:

```text
Can this coder take this dongle now?
```

To answer that, the program must combine ownership, cooldown, scheduler choice,
and stop state. This is why scattered checks become fragile. The decision needs
to be made while the relevant shared state is protected.

## Waiting Is Not Neutral

When more than one coder wants the same dongle, the program must not simply wake
whoever happens to run first. The subject gives the waiting line a law.

With FIFO, the oldest request wins. With EDF, the request whose coder is closest
to burnout wins. In both cases, waiting has memory. The program must remember
when requests arrived or what their deadlines are, compare them, and grant the
dongle to the correct request.

This is why the heap belongs in the model. The heap is not just a data structure
you implement because the subject says so. It is the physical shape of the
waiting line. FIFO and EDF are different definitions of "front of the line."

## Time Is An Active Part Of The System

Time does not merely appear in log messages. Time changes what is legal.

A dongle becomes legal to take only after cooldown. A coder remains alive only
until the burnout deadline. The monitor thread exists because the simulation
needs an independent observer of time. A coder thread might be sleeping,
blocked, or waiting for a condition. The monitor must still be able to say,
"This coder has missed the deadline; the simulation is over."

This gives the monitor a different role from the coders. Coders try to make
local progress. The monitor protects the global meaning of the simulation.

## The Model To Carry Forward

By the end of this page, you should see Codexion as one shared world with five
truths that must stay coherent:

```text
ownership: which coder, if any, holds each dongle
cooldown: when each released dongle may be reused
waiting: which coders are queued and who has priority
life: when each coder last began compiling
history: what the log has already announced
```

The rest of the project is the engineering work of preserving those truths while
many threads run. Pthreads create the actors. Mutexes protect shared reality.
Condition variables let actors sleep until the world changes. The scheduler
chooses from the waiting line. The monitor watches time.

That is the line through the whole project.

## Further Reading

- Dining Philosophers: <https://en.wikipedia.org/wiki/Dining_philosophers_problem>
- Race condition: <https://en.wikipedia.org/wiki/Race_condition>
- Deadlock: <https://en.wikipedia.org/wiki/Deadlock>

## Navigation

- Previous: [Codexion Learning Notes](./README.md)
- Next: [Dongle Assignment In `init_coder`](./dongle-assignment.md)
