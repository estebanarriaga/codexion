#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THREAD_COUNT 4
#define INCREMENTS 100000

typedef struct s_shared
{
	int				counter;
	pthread_mutex_t	mutex;
}	t_shared;

static void	die_pthread(int err, const char *message)
{
	if (err != 0)
	{
		fprintf(stderr, "%s: %s\n", message, strerror(err));
		exit(EXIT_FAILURE);
	}
}

static void	*unsafe_increment(void *arg)
{
	t_shared	*shared;
	int			i;

	shared = (t_shared *)arg;
	i = 0;
	while (i < INCREMENTS)
	{
		shared->counter++;
		i++;
	}
	return (NULL);
}

static void	*safe_increment(void *arg)
{
	t_shared	*shared;
	int			i;

	shared = (t_shared *)arg;
	i = 0;
	while (i < INCREMENTS)
	{
		die_pthread(pthread_mutex_lock(&shared->mutex), "pthread_mutex_lock");
		shared->counter++;
		die_pthread(pthread_mutex_unlock(&shared->mutex), "pthread_mutex_unlock");
		i++;
	}
	return (NULL);
}

static void	run_counter_case(const char *title, void *(*routine)(void *),
		t_shared *shared)
{
	pthread_t	threads[THREAD_COUNT];
	int			i;

	printf("\n%s\n", title);
	shared->counter = 0;
	i = 0;
	while (i < THREAD_COUNT)
	{
		die_pthread(pthread_create(&threads[i], NULL, routine, shared),
			"pthread_create");
		i++;
	}
	i = 0;
	while (i < THREAD_COUNT)
	{
		die_pthread(pthread_join(threads[i], NULL), "pthread_join");
		i++;
	}
	printf("expected: %d\n", THREAD_COUNT * INCREMENTS);
	printf("actual:   %d\n", shared->counter);
}

static void	demo_unlock_without_lock(void)
{
	pthread_mutex_t	errorcheck;
	pthread_mutexattr_t	attr;
	int					err;

	printf("\nFailure case: unlocking a mutex this thread does not own\n");
	die_pthread(pthread_mutexattr_init(&attr), "pthread_mutexattr_init");
	die_pthread(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK),
		"pthread_mutexattr_settype");
	die_pthread(pthread_mutex_init(&errorcheck, &attr), "pthread_mutex_init");
	err = pthread_mutex_unlock(&errorcheck);
	printf("pthread_mutex_unlock returned %d (%s)\n", err, strerror(err));
	die_pthread(pthread_mutex_destroy(&errorcheck), "pthread_mutex_destroy");
	die_pthread(pthread_mutexattr_destroy(&attr), "pthread_mutexattr_destroy");
}

static void	demo_destroy_locked_mutex(void)
{
	pthread_mutex_t	mutex;
	int				err;

	printf("\nFailure case: destroying a locked mutex\n");
	die_pthread(pthread_mutex_init(&mutex, NULL), "pthread_mutex_init");
	die_pthread(pthread_mutex_lock(&mutex), "pthread_mutex_lock");
	err = pthread_mutex_destroy(&mutex);
	printf("pthread_mutex_destroy returned %d (%s)\n", err, strerror(err));
	die_pthread(pthread_mutex_unlock(&mutex), "pthread_mutex_unlock");
	die_pthread(pthread_mutex_destroy(&mutex), "pthread_mutex_destroy");
}

static void	demo_deadlock_warning(void)
{
	printf("\nFailure case not executed: locking the same normal mutex twice\n");
	printf("A normal mutex does not have to detect this. The second lock can wait forever.\n");
	printf("Use a clear lock order, or use PTHREAD_MUTEX_ERRORCHECK while debugging.\n");
}

int	main(void)
{
	t_shared	shared;

	die_pthread(pthread_mutex_init(&shared.mutex, NULL), "pthread_mutex_init");
	run_counter_case("Unsafe case: shared counter without a mutex",
		unsafe_increment, &shared);
	run_counter_case("Safe case: shared counter protected by a mutex",
		safe_increment, &shared);
	die_pthread(pthread_mutex_destroy(&shared.mutex), "pthread_mutex_destroy");
	demo_unlock_without_lock();
	demo_destroy_locked_mutex();
	demo_deadlock_warning();
	printf("\nBuild with: cc -Wall -Wextra -Werror -pthread pthread_mutex_demo.c -o pthread_mutex_demo\n");
	return (0);
}
