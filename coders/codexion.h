
#ifndef CODEXION_H
# define CODEXION_H 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <limits.h>

/*
Allowed functions:
    - pthread_create, pthread_join, pthread_mutex_init,
    - pthread_mutex_lock,
    - pthread_mutex_unlock, pthread_mutex_destroy,
    - pthread_cond_init,
    - pthread_cond_wait, pthread_cond_timedwait,
    - pthread_cond_broadcast,
    - pthread_cond_destroy, gettimeofday, usleep, write,
    - malloc, free,
    - printf, fprintf, strcmp, strlen, atoi, memset
*/

typedef enum e_coder_state
{
	STATE_WAITING,
	STATE_COMPILING,
	STATE_DEBUGGING,
	STATE_REFACTORING,
	STATE_DONE
}	t_coder_state;

typedef struct s_sim	t_sim;

typedef struct s_params
{
	char	*scheduler;
	int		number_of_coders;
	int		time_to_burnout;
	int		time_to_compile;
	int		time_to_debug;
	int		time_to_refactor;
	int		number_of_compiles_required;
	int		dongle_cooldown;
}	t_params;

typedef struct s_request
{
	long	arrival_ms;
	long	deadline_ms;
	long	sequence;
	int		coder_id;
}	t_request;

typedef struct s_heap
{
	t_request	*items;
	int			size;
	int			capacity;
}	t_heap;

typedef struct s_coder
{
	pthread_t		thread;
	/**
	 * Woken after scheduler/resource state changes so this coder can recheck.
	 */
	pthread_cond_t	cond;
	/**
	 * Back-reference to shared simulation state used by the thread routine.
	 */
	t_sim			*sim;
	/**
	 * Absolute timestamp in ms of the last time this coder started compiling.
	 */
	long			last_compile_start;
	int				id;
	t_coder_state	state;
	int				compile_count;
	/**
	 * Indexes into sim->dongles; stored to avoid repeating circular math.
	 */
	int				left_dongle;
	int				right_dongle;
}	t_coder;

typedef struct s_dongle
{
	int		id;
	/**
	 * 0 means unowned; otherwise stores the id of the coder holding it.
	 */
	int		owner_id;
	/**
	 * Absolute timestamp in ms before which this dongle cannot be granted.
	 */
	long	cooldown_until;
	/**
	 * Min-heap of waiting requests, ordered by FIFO or EDF comparator.
	 */
	t_heap	waiters;
}	t_dongle;

struct s_sim
{
	t_params		params;
	t_coder			*coders;
	t_dongle		*dongles;
	/**
	 * Absolute timestamp in ms used as origin for elapsed log timestamps.
	 */
	long			start_ms;
	/**
	 * Monotonic tie-breaker for requests with equal arrival/deadline.
	 */
	long			sequence;
	pthread_t		scheduler_thread;
	pthread_t		monitor_thread;
	/**
	 * Protects shared simulation truth: coders, dongles, queues, and stop.
	 */
	pthread_mutex_t	mutex;
	/**
	 * The `log_mutex` is used to ensure that log messages are not interleaved,
	 * and burnout/state logging also needs coordination with stop.
	 */
	pthread_mutex_t	log_mutex;
	/**
	 * Broadcast when scheduler-relevant state changes or stop is set.
	 */
	pthread_cond_t	sched_cond;
	int				stop;
};

int		parse_arguments(int argc, char *argv[], t_params *params);

void    print_usage(char *program);

int   initialize_simulation(t_sim *sim, t_params *params);

#endif
