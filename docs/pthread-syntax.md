# POSIX Threads Syntax

The project uses POSIX threads, usually called pthreads. A pthread is created by
passing a function pointer to `pthread_create`. That function becomes the entry
point of the new thread.

## Thread Function Shape

Pthread routines have this exact shape:

```c
void	*routine(void *arg);
```

Example:

```c
void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	/* thread work here */
	return (NULL);
}
```

The argument is `void *` so you can pass any pointer type. In Codexion, you
usually pass a pointer to a `t_coder`, `t_sim`, or a small context struct.

## Creating Threads

```c
pthread_t	thread;
t_coder		coder;

if (pthread_create(&thread, NULL, coder_routine, &coder) != 0)
{
	/* handle error */
}
```

Arguments:

- `&thread`: where pthread stores the thread handle.
- `NULL`: default thread attributes.
- `coder_routine`: function the new thread runs.
- `&coder`: pointer passed as `arg` to the routine.

The thread handle is not the same as your coder id. It is an operating-system
thread identifier used by pthread functions.

## Joining Threads

`pthread_join` waits until a thread finishes:

```c
pthread_join(thread, NULL);
```

For Codexion:

- create all coder threads,
- create the monitor thread,
- wait until stop condition,
- join every thread,
- destroy mutexes and condition variables.

Do not free memory that a thread might still access. Join first, clean up after.

## Passing Arguments Safely

This is wrong:

```c
int	i;

i = 0;
while (i < n)
{
	pthread_create(&threads[i], NULL, routine, &i);
	i++;
}
```

Every thread receives the address of the same `i`, so they may all observe the
wrong value.

Use stable per-thread storage:

```c
i = 0;
while (i < n)
{
	coders[i].id = i + 1;
	pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
	i++;
}
```

## Compilation

Compile and link with pthread support:

```sh
cc -Wall -Wextra -Werror -pthread ...
```

The `-pthread` flag matters. It tells the compiler and linker to enable pthread
support correctly.

## Common Mistakes

- Returning from `main` before joining worker threads.
- Passing the address of a loop variable to all threads.
- Reading shared state without a mutex because "it is only an int".
- Forgetting that all threads share heap memory.
- Destroying a mutex or condition variable while another thread may still use it.
- Calling `usleep` while holding a mutex needed by other threads.

## Further Reading

- pthreads overview: <https://man7.org/linux/man-pages/man7/pthreads.7.html>
- `pthread_create`: <https://man7.org/linux/man-pages/man3/pthread_create.3.html>
- `pthread_join`: <https://man7.org/linux/man-pages/man3/pthread_join.3.html>

## Navigation

- Previous: [Project Model](./project-model.md)
- Next: [Synchronization Primitives](./synchronization.md)
