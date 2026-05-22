
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <limits.h>

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
	int		number_of_coders;
	int		time_to_burnout;
	int		time_to_compile;
	int		time_to_debug;
	int		time_to_refactor;
	int		number_of_compiles_required;
	int		dongle_cooldown;
	char	*scheduler;
}	t_params;

typedef struct s_request
{
	int		coder_id;
	long	arrival_ms;
	long	deadline_ms;
	long	sequence;
}	t_request;

typedef struct s_heap
{
	t_request	*items;
	int			size;
	int			capacity;
}	t_heap;

typedef struct s_coder
{
	int				id;
	t_coder_state	state;
	int				compile_count;
	/**
	 * Absolute timestamp in ms of the last time this coder started compiling.
	 */
	long			last_compile_start;
	/**
	 * Indexes into sim->dongles; stored to avoid repeating circular math.
	 */
	int				left_dongle;
	int				right_dongle;
	pthread_t		thread;
	/**
	 * Woken after scheduler/resource state changes so this coder can recheck.
	 */
	pthread_cond_t	cond;
	/**
	 * Back-reference to shared simulation state used by the thread routine.
	 */
	t_sim			*sim;
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

static void	print_usage(char *program)
{
	fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown scheduler\n", program);
}

static int	parse_positive_int(char *arg, int *value)
{
	long	result;
	int		i;
	int		digit;

	if (arg[0] == '\0')
		return (-1);
	result = 0;
	i = 0;
	while (arg[i] != '\0')
	{
		if (arg[i] < '0' || arg[i] > '9')
			return (-1);
		digit = arg[i] - '0';
		if (result > (INT_MAX - digit) / 10)
			return (-1);
		result = result * 10 + digit;
		i++;
	}
	if (result <= 0)
		return (-1);
	*value = (int)result;
	return (0);
}

static void	set_fields(t_params *params, int *fields[7])
{
	fields[0] = &params->number_of_coders;
	fields[1] = &params->time_to_burnout;
	fields[2] = &params->time_to_compile;
	fields[3] = &params->time_to_debug;
	fields[4] = &params->time_to_refactor;
	fields[5] = &params->number_of_compiles_required;
	fields[6] = &params->dongle_cooldown;
}

int	parse_arguments(int argc, char *argv[], t_params *params)
{
	int	i;
	int	*fields[7];

	if (argc != 9)
	{
		print_usage(argv[0]);
		return (-1);
	}
	set_fields(params, fields);
	i = 0;
	while (i < 7)
	{
		if (parse_positive_int(argv[i + 1], fields[i]) == -1)
		{
			fprintf(stderr, "Error: argument %d (%s) must be a positive "
				"integer between 1 and %d.\n", i + 1, argv[i + 1], INT_MAX);
			return (-1);
		}
		i++;
	}
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
	{
		fprintf(stderr, "Error: scheduler must be exactly fifo or edf.\n");
		return (-1);
	}
	params->scheduler = argv[8];
	return (0);
}


int main(int argc, char *argv[])
{
	t_params	params;

	if (parse_arguments(argc, argv, &params) == -1)
		return (1);
	(void)params;

	// Initialize coders, dongles, and other necessary data structures here

	// Create threads for each coder and start the simulation

	// Wait for all threads to finish

	// Clean up resources

	return (0);
}
