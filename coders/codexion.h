#ifndef CODEXION_H
# define CODEXION_H 1

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <pthread.h>
# include <string.h>
# include <sys/time.h>
# include <limits.h>
# include <errno.h>

typedef enum e_coder_state
{
	STATE_WAITING,
	STATE_COMPILING,
	STATE_DEBUGGING,
	STATE_REFACTORING,
	STATE_DONE
}t_coder_state;

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
}t_params;

typedef struct s_request
{
	long	arrival_ms;
	long	deadline_ms;
	long	sequence;
	int		coder_id;
}t_request;

typedef struct s_heap
{
	t_request	*requests;
	int			size;
	int			capacity;
}t_heap;

typedef struct s_coder
{
	pthread_t		thread;
	pthread_cond_t	cond;
	t_sim				*sim;
	long				last_compile_start;
	int				id;
	t_coder_state	state;
	int				compile_count;
	int				request_pending;
	int				left_dongle;
	int				right_dongle;
}t_coder;

typedef struct s_dongle
{
	int		id;
	int		owner_id;
	long	cooldown_until;
	t_heap	waiters;
}t_dongle;

struct s_sim
{
	t_params		params;
	t_coder			*coders;
	t_dongle		*dongles;
	long			start_ms;
	long			sequence;
	pthread_t		scheduler_thread;
	pthread_t		monitor_thread;
	pthread_mutex_t	mutex;
	pthread_mutex_t	log_mutex;
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
}t_opcode;

typedef enum e_time_code
{
	SECONDS,
	MILLISECONDS,
	MICROSECONDS
}t_time_code;

int		parse_arguments(int argc, char *argv[], t_params *params);
void	print_usage(char *program);
int		initialize_simulation(t_sim *sim, t_params *params);
long	gettime(t_time_code time_code);
void	*coding_simulation(void *arg);
void	*scheduler_thread(void *arg);
void	*monitor_thread(void *arg);

int		heap_is_empty(const t_heap *heap);
int		heap_push_pair(t_heap *first, t_request first_request,
			t_heap *second, t_request second_request, const char *scheduler);
int		heap_peek(const t_heap *heap, t_request *request);
int		heap_pop_pair(t_heap *first, t_heap *second,
			const char *scheduler);

int		safe_mutex_handle(pthread_mutex_t *mutex, t_opcode opcode);
int		safe_thread_handle(pthread_t *thread, t_opcode opcode,
			void *(*start_routine)(void *), void *arg);
int		safe_cond_handle(pthread_cond_t *cond, t_opcode opcode,
			pthread_mutex_t *mutex, const struct timespec *abstime);

void	print_message(t_sim *sim, int id, char *message);
int		handle_error(char *message, int error_code);
int		handle_mutex_error(int status, t_opcode opcode);
int		handle_thread_error(int status, t_opcode opcode);
int		handle_cond_error(int status, t_opcode opcode);

int		destroy_simulation(t_sim *sim, int status);
void	free_simulation(t_sim *sim);

#endif
