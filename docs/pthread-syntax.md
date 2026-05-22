# POSIX Threads Syntax

In the project model, Codexion is one shared world: coders, dongles, queues, and
time. Pthreads are how that world receives independent actors.

A thread is a path of execution inside the same process. That phrase matters.
Threads do not get separate copies of your heap. If every coder thread can reach
the same `t_sim`, then every coder is looking at the same dongles, the same stop
flag, the same queues, and the same timestamps.

This is both the power and the danger of pthreads. Sharing memory makes
communication simple: one thread writes a field and another thread can read it.
But sharing memory also means the program must decide when those reads and
writes are allowed to overlap.

Before learning the synchronization rules, first learn the small amount of
syntax that creates and finishes threads.

## A Thread Starts At A Function

Pthreads do not start from an object or a class. They start from a C function.
That function must have this exact shape:

```c
void	*routine(void *arg);
```

The argument is `void *` because pthreads are generic. The library does not know
what a `t_coder` is. You pass a pointer to your own data, and inside the routine
you cast it back.

```c
void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	/* this coder now executes its cycle */
	return (NULL);
}
```

This tiny cast has a large design consequence. The pointer you pass is the
thread's entrance into the simulation. A good Codexion design usually gives each
thread a pointer to its own `t_coder`, and that coder struct points back to the
shared simulation.

That gives the routine two kinds of knowledge:

```text
who am I?              -> coder->id, coder->state, coder->compile_count
what world am I in?    -> coder->sim, sim->dongles, sim->stop, sim->time
```

## Creating A Thread Creates A New Timeline

`pthread_create` asks the operating system to begin running a function in a new
thread:

```c
pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
```

Read it slowly:

```text
&coders[i].thread    where the thread handle will be stored
NULL                 default thread attributes
coder_routine        function the new thread begins executing
&coders[i]           argument received by coder_routine
```

After this call succeeds, the new thread may start immediately. It may even run
before the loop in the main thread creates the next coder. This is the first
place where single-threaded intuition fails. You cannot assume "I create all
threads first, then they begin." The OS scheduler decides when each thread runs.

Therefore, everything the new thread needs must already be valid before
`pthread_create` is called.

## Stable Addresses Matter

The most common beginner bug is passing the address of a loop variable:

```c
int	i;

i = 0;
while (i < n)
{
	pthread_create(&threads[i], NULL, routine, &i);
	i++;
}
```

This looks reasonable only if you imagine the new thread immediately copies the
value of `i`. It does not. It receives the address of `i`. All threads receive
the same address, and the main thread keeps changing the value stored there.

The correct version passes each thread a stable piece of memory:

```c
i = 0;
while (i < n)
{
	coders[i].id = i + 1;
	pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
	i++;
}
```

Now coder 1 receives `&coders[0]`, coder 2 receives `&coders[1]`, and so on.
Those addresses remain meaningful for the lifetime of the threads.

## Joining Closes The Lifetime

If `pthread_create` opens a new timeline, `pthread_join` waits for that timeline
to finish:

```c
pthread_join(coders[i].thread, NULL);
```

Joining is not just cleanup. It is part of memory ownership. The main thread
must not destroy the simulation while coder threads may still read it. If you
free `sim->coders` before joining, a coder routine might continue running with a
pointer into freed memory.

The safe lifetime is:

```text
build the shared simulation
initialize mutexes and condition variables
create coder and monitor threads
let the simulation run
signal stop
join every thread
destroy synchronization objects
free memory
```

This order is not ceremony. It is how you ensure that every pointer passed to
every thread remains valid for as long as the thread can use it.

## The Compiler Flag Is Part Of The Contract

Build pthread programs with `-pthread`:

```sh
cc -Wall -Wextra -Werror -pthread ...
```

This flag tells the compiler and linker that the program uses pthreads. Treat it
as part of the project contract, not as an optional linker detail.

## Where This Leads

At this point you can create independent coders. But creating them only makes
the problem visible. The next question is harder: when two coders inspect and
change the same dongle, how do we prevent them from inventing two different
versions of reality?

That is the role of synchronization.

## Further Reading

- pthreads overview: <https://man7.org/linux/man-pages/man7/pthreads.7.html>
- `pthread_create`: <https://man7.org/linux/man-pages/man3/pthread_create.3.html>
- `pthread_join`: <https://man7.org/linux/man-pages/man3/pthread_join.3.html>

## Navigation

- Previous: [Project Model](./project-model.md)
- Next: [Synchronization Primitives](./synchronization.md)
