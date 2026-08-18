
#ifndef CODEXION_H
# define CODEXION_H 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <sys/time.h>
#include <limits.h>
#include <errno.h>

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
	t_request	*requests;
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

/**
 * All dongle state is protected exclusively by sim->mutex.
 */
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
	/**
	 * Monitors for burnout and sets sim->stop when all coders are done or one burns out.
	 */
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

typedef enum e_opcode
{
    LOCK,
    UNLOCK,
    INIT,
    DESTROY,
    CREATE,
    JOIN,
    INIT_COND,
    WAIT,
    TIMEDWAIT,
    BROADCAST,
    DESTROY_COND,
}	t_opcode;

typedef enum e_time_code
{
    SECONDS,
    MILLISECONDS,
    MICROSECONDS
}   t_time_code;


int		parse_arguments(int argc, char *argv[], t_params *params);

void    print_usage(char *program);

int   initialize_simulation(t_sim *sim, t_params *params);
int	create_coder_threads(t_sim *sim);
int	wait_for_threads(t_sim *sim);
long	gettime(t_time_code time_code);
void	*coding_simulation(void *arg);

/** Safe utility functions for thread and mutex operations */
int safe_mutex_handle(pthread_mutex_t *mutex, t_opcode opcode);
int safe_thread_handle(pthread_t *thread, t_opcode opcode,
    void *(*start_routine)(void *), void *arg);
int safe_cond_handle(pthread_cond_t *cond, t_opcode opcode,
    pthread_mutex_t *mutex, const struct timespec *abstime);

/** Error handling functions */
int handle_error(char *message, int error_code);
int handle_mutex_error(int status, t_opcode opcode);
int handle_thread_error(int status, t_opcode opcode);
int handle_cond_error(int status, t_opcode opcode);

#endif
