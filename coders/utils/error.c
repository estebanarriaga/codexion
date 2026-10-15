#include "codexion.h"

int handle_error(char *message, int error_code)
{
    fprintf(stderr, "%s\n", message);
    return (error_code);
}

/**
 * Handles errors for thread operations and prints appropriate messages.
 * Returns 0 on success, or the error code on failure.
 * 
 * Error codes:
 * - EAGAIN: Insufficient resources to create another thread.
 * - EINVAL: Invalid settings in attr or thread is not joinable.
 * - EPERM: No permission to set the scheduling policy and parameters,
 *      specified in attr.
 * - EDEADLK: A deadlock was detected (e.g., two threads tried
 *      to join with each other); or thread specifies the calling thread.
 * - ESRCH: No thread with the ID thread could be found.
 */
int handle_thread_error(int status, t_opcode opcode)
{
    if (status == 0)
        return 0;
    if (opcode == CREATE && status == EAGAIN)
        return (handle_error("Insufficient resources to create another thread.", status));
    else if (opcode == CREATE && status == EINVAL)
        return (handle_error("Invalid settings in attr.", status));
    else if (opcode == CREATE && status == EPERM)
        return (handle_error("No permission to set the scheduling policy and parameters specified in attr.", status));
    else if (opcode == JOIN && status == EDEADLK)
        return (handle_error("A deadlock was detected (e.g., two threads tried to join with each other); or thread specifies the calling thread.", status));
    else if (opcode == JOIN && status == EINVAL)
        return (handle_error("Thread is not a joinable thread or another thread is already waiting to join with this thread.", status));
    else if (opcode == JOIN && status == ESRCH)
        return (handle_error("No thread with the ID thread could be found.", status));
    else
        return (handle_error("Unknown error for opcode.", status));
}

/**
 * Handles errors for mutex operations and prints appropriate messages.
 * Returns 0 on success, or the error code on failure.
 * 
 * Error codes:
 * - EINVAL: Invalid mutex or attribute.
 * - EDEADLK: Deadlock would occur.
 * - EPERM: Current thread does not hold the lock.
 * - ENOMEM: Not enough memory to create another mutex.
 * - EBUSY: Mutex is locked.
 */
int handle_mutex_error(int status, t_opcode opcode)
{
    if (status == 0)
        return 0;
    if (status == EINVAL && (opcode == LOCK || opcode == UNLOCK))
        return (handle_error("The value specified by mutex is invalid.", status));
    else if (status == EINVAL && opcode == INIT)
        return (handle_error("The value specified by attr is invalid.", status));
    else if (status == EDEADLK)
        return (handle_error("A deadlock would occur if the thread blocked waiting for mutex.", status));
    else if (status == EPERM)
        return (handle_error("The current thread does not hold a lock on mutex.", status));
    else if (status == ENOMEM)
        return (handle_error("The process cannot allocate enough memory to create another mutex.", status));
    else if (status == EBUSY)
        return (handle_error("Mutex is locked.", status));
    else
        return (handle_error("Unknown error for opcode.", status));
    return (status);
}

/**
 * Handles errors for condition variable operations and prints appropriate messages.
 * Returns 0 on success, or the error code on failure.
 * 
 * Error codes:
 * - ETIMEDOUT: The time specified by abstime to pthread_cond_timedwait() has passed.
 * - EINVAL: The abstime argument specified a nanosecond value less than zero or greater than or equal to 1000 million, or invalid opcode.
 * - EOWNERDEAD: The mutex is a robust mutex and the previous owning thread terminated while holding the mutex lock. The mutex lock shall be acquired by the calling thread and it is up to the new owner to make the state consistent.
 * - EPERM: The mutex type is PTHREAD_MUTEX_ERRORCHECK or the mutex is a robust mutex, and the current thread does not own the mutex.
 */
int handle_cond_error(int status, t_opcode opcode)
{
    if (status == 0)
        return 0;
    if (opcode == TIMEDWAIT && status == ETIMEDOUT)
        return (ETIMEDOUT);
    else if (status == EINVAL)
        return (handle_error("The abstime argument specified a nanosecond value less than zero or greater than or equal to 1000 million.", status));
    else if (status == EOWNERDEAD)
        return (handle_error("The mutex is a robust mutex and the previous owning thread terminated while holding the mutex lock. The mutex lock shall be acquired by the calling thread and it is up to the new owner to make the state consistent.", status));
    else if (status == EPERM)
        return (handle_error("The mutex type is PTHREAD_MUTEX_ERRORCHECK or the mutex is a robust mutex, and the current thread does not own the mutex.", status));
    else if (status == EINVAL)
        return (handle_error("Invalid opcode for condition variable handling.", status));
    else
        return (handle_error("Unknown error for opcode.", status));
}
