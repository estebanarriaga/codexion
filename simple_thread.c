#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CONSUMER_COUNT 3
#define JOB_COUNT 12
#define QUEUE_CAPACITY 4

/*
 * This program is a small producer/consumer example.
 *
 * One producer thread creates numbered jobs and puts them into a fixed-size
 * queue. Several consumer threads remove jobs from that queue and process
 * them. The queue is shared memory, so every read or write to its state is
 * protected by a mutex.
 *
 * The two condition variables let threads sleep instead of wasting CPU:
 * - not_empty: consumers wait here while the queue has no jobs.
 * - not_full: the producer waits here while the queue is full.
 * 
 * The program is a small producer/consumer pthread example.
 *
 * It creates:
 *
 * - 1 producer thread
 * - 3 consumer threads
 * - 1 shared queue
 * - 1 mutex
 * - 2 condition variables
 *
 * The producer creates numbered jobs from 1 to 12 and puts them into the queue. The consumers take jobs out of the queue and “process” them by printing a message and sleeping briefly.
 *
 * The queue can only hold 4 jobs at a time, so synchronization matters.
 *
 * The mutex protects the shared queue state:
 *
 * pthread_mutex_t mutex;
 *
 * Any time a thread reads or modifies head, tail, count, done, or jobs, it first locks the mutex.
 * This prevents two threads from changing the queue at the same time.
 *
 * The condition variables let threads sleep efficiently:
 *
 * pthread_cond_t not_empty;
 * pthread_cond_t not_full;
 *
 * not_empty is used by consumers. If the queue is empty, a consumer waits until the producer adds a job.
 *
 * not_full is used by the producer. If the queue is full, the producer waits until a consumer removes a job.
 *
 * The basic flow is:
 *
 * 1. main initializes the queue.
 * 2. main starts the producer thread.
 * 3. main starts 3 consumer threads.
 * 4. The producer adds jobs to the queue.
 * 5. Consumers remove and process jobs.
 * 6. When the producer has created all jobs, it marks the queue as done.
 * 7. Waiting consumers wake up, see there are no more jobs, and exit.
 * 8. main joins all threads and destroys the mutex/condition variables.
 *
 * The important pthread pattern is this:
 *
 * while (queue->count == 0 && !queue->done)
 *     pthread_cond_wait(&queue->not_empty, &queue->mutex);
 *
 * A thread waits inside a while, not an if, because waking up does not guarantee the condition is
 * true. It only means “something may have changed, check again.”
 *
 * So the program demonstrates how multiple threads can safely share work using a mutex for protection and condition variables for waiting/waking.
 */
typedef struct s_queue
{
	/* Circular buffer storage. head reads the next job, tail writes one. */
	int				jobs[QUEUE_CAPACITY];
	int				head;
	int				tail;
	/* Number of jobs currently inside jobs[]. */
	int				count;
	/* Set to 1 when the producer will not add more jobs. */
	int				done;
	/* Protects every field above this line. */
	pthread_mutex_t	mutex;
	/* Signaled when a job is added, so waiting consumers can wake up. */
	pthread_cond_t	not_empty;
	/* Signaled when a job is removed, so the producer can add more work. */
	pthread_cond_t	not_full;
}	t_queue;

typedef struct s_worker
{
	/* A stable id makes the output easier to follow. */
	int		id;
	/* All consumers share the same synchronized queue. */
	t_queue	*queue;
}	t_worker;

/*
 * Initialize the shared queue and the pthread synchronization objects.
 */
static void	queue_init(t_queue *queue)
{
	queue->head = 0;
	queue->tail = 0;
	queue->count = 0;
	queue->done = 0;
	if (pthread_mutex_init(&queue->mutex, NULL) != 0
		|| pthread_cond_init(&queue->not_empty, NULL) != 0
		|| pthread_cond_init(&queue->not_full, NULL) != 0)
	{
		perror("failed to initialize pthread objects");
		exit(EXIT_FAILURE);
	}
}

/*
 * Destroy pthread objects after every thread has stopped using the queue.
 */
static void	queue_destroy(t_queue *queue)
{
	pthread_cond_destroy(&queue->not_full);
	pthread_cond_destroy(&queue->not_empty);
	pthread_mutex_destroy(&queue->mutex);
}

/*
 * Add one job to the queue.
 *
 * The mutex must be locked before checking or modifying queue->count. If the
 * queue is full, pthread_cond_wait temporarily unlocks the mutex and puts this
 * thread to sleep. When it wakes, the mutex is locked again and the while loop
 * rechecks the real condition.
 */
static void	queue_push(t_queue *queue, int job)
{
	pthread_mutex_lock(&queue->mutex);
	while (queue->count == QUEUE_CAPACITY)
		pthread_cond_wait(&queue->not_full, &queue->mutex);
	queue->jobs[queue->tail] = job;
	queue->tail = (queue->tail + 1) % QUEUE_CAPACITY;
	queue->count++;
	printf("producer created job %d (queue size: %d)\n", job, queue->count);
	/* Wake one consumer, because at least one job is now available. */
	pthread_cond_signal(&queue->not_empty);
	pthread_mutex_unlock(&queue->mutex);
}

/*
 * Remove one job from the queue.
 *
 * Returns 1 when a job was found and copied into *job.
 * Returns 0 when the producer is done and the queue is empty.
 */
static int	queue_pop(t_queue *queue, int *job)
{
	pthread_mutex_lock(&queue->mutex);
	/*
	 * Always wait in a while loop. A wakeup only means "check again"; it
	 * does not guarantee that the condition is true.
	 */
	while (queue->count == 0 && !queue->done)
		pthread_cond_wait(&queue->not_empty, &queue->mutex);
	if (queue->count == 0 && queue->done)
	{
		pthread_mutex_unlock(&queue->mutex);
		return (0);
	}
	*job = queue->jobs[queue->head];
	queue->head = (queue->head + 1) % QUEUE_CAPACITY;
	queue->count--;
	/* Wake the producer, because at least one queue slot is now free. */
	pthread_cond_signal(&queue->not_full);
	pthread_mutex_unlock(&queue->mutex);
	return (1);
}

/*
 * Mark the queue as closed.
 *
 * Broadcast is used because several consumers may be asleep on not_empty.
 * They all need to wake up, notice done == 1, and exit cleanly.
 */
static void	queue_close(t_queue *queue)
{
	pthread_mutex_lock(&queue->mutex);
	queue->done = 1;
	pthread_cond_broadcast(&queue->not_empty);
	pthread_mutex_unlock(&queue->mutex);
}

/*
 * Thread entry point for the producer.
 *
 * pthread_create requires a function with the signature:
 *     void *routine(void *arg)
 */
static void	*producer_routine(void *arg)
{
	t_queue	*queue;
	int		job;

	queue = arg;
	job = 1;
	while (job <= JOB_COUNT)
	{
		queue_push(queue, job);
		usleep(50000);
		job++;
	}
	queue_close(queue);
	return (NULL);
}

/*
 * Thread entry point for each consumer.
 *
 * Each consumer keeps asking for jobs until queue_pop reports that the queue is
 * closed and empty.
 */
static void	*consumer_routine(void *arg)
{
	t_worker	*worker;
	int			job;

	worker = arg;
	while (queue_pop(worker->queue, &job))
	{
		printf("consumer %d is processing job %d\n", worker->id, job);
		usleep(300000);
	}
	printf("consumer %d has no more jobs\n", worker->id);
	return (NULL);
}

int	main(void)
{
	t_queue		queue;
	pthread_t	producer;
	pthread_t	consumers[CONSUMER_COUNT];
	t_worker	workers[CONSUMER_COUNT];
	int			i;

	queue_init(&queue);
	/* Start the single producer thread. */
	if (pthread_create(&producer, NULL, producer_routine, &queue) != 0)
	{
		perror("failed to create producer");
		queue_destroy(&queue);
		return (EXIT_FAILURE);
	}
	i = 0;
	while (i < CONSUMER_COUNT)
	{
		workers[i].id = i + 1;
		workers[i].queue = &queue;
		/* Start one consumer thread and pass it its worker data. */
		if (pthread_create(&consumers[i], NULL, consumer_routine,
				&workers[i]) != 0)
		{
			perror("failed to create consumer");
			queue_close(&queue);
			pthread_join(producer, NULL);
			while (--i >= 0)
				pthread_join(consumers[i], NULL);
			queue_destroy(&queue);
			return (EXIT_FAILURE);
		}
		i++;
	}
	/* Join waits for each thread to finish before main exits. */
	pthread_join(producer, NULL);
	i = 0;
	while (i < CONSUMER_COUNT)
		pthread_join(consumers[i++], NULL);
	queue_destroy(&queue);
	return (EXIT_SUCCESS);
}
