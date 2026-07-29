#include "codexion.h"

/**
 * Mutex handling wrapper that checks for errors and prints appropriate messages.
 * Returns 0 on success, or the error code on failure.
 * 
 * Supported opcodes: LOCK, UNLOCK, INIT, DESTROY.
 */
int safe_mutex_handle(pthread_mutex_t *mutex, t_opcode opcode)
{
    if (opcode == LOCK)
        return handle_mutex_error(pthread_mutex_lock(mutex), opcode);
    else if (opcode == UNLOCK)
        return handle_mutex_error(pthread_mutex_unlock(mutex), opcode);
    else if (opcode == INIT)
        return handle_mutex_error(pthread_mutex_init(mutex, NULL), opcode);
    else if (opcode == DESTROY)
        return handle_mutex_error(pthread_mutex_destroy(mutex), opcode);
    else
        return handle_error("Invalid opcode for mutex handling.", EINVAL);
}

/**
 * Thread handling wrapper that checks for errors and prints
 * appropriate messages.
 * Returns 0 on success, or the error code on failure.
 * 
 * Supported opcodes: CREATE, JOIN.
 */
int safe_thread_handle(pthread_t *thread, t_opcode opcode, void *(*start_routine)(void *), void *arg)
{
    if (opcode == CREATE)
        return handle_thread_error(pthread_create(thread, NULL, start_routine, arg), opcode);
    else if (opcode == JOIN)
        return handle_thread_error(pthread_join(*thread, NULL), opcode);
    else
        return handle_error("Invalid opcode for thread handling.", EINVAL);
}

/**
 * Condition variable handling wrapper that checks for errors and prints
 * appropriate messages.
 * Returns 0 on success, or the error code on failure.
 * 
 * Supported opcodes: INIT_COND, WAIT, TIMEDWAIT, BROADCAST, DESTROY_COND.
 */
int safe_cond_handle(pthread_cond_t *cond, t_opcode opcode, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    if (opcode == INIT_COND)
        return handle_cond_error(pthread_cond_init(cond, NULL), opcode);
    else if (opcode == WAIT)
        return handle_cond_error(pthread_cond_wait(cond, mutex), opcode);
    else if (opcode == TIMEDWAIT)
        return handle_cond_error(pthread_cond_timedwait(cond, mutex, abstime), opcode);
    else if (opcode == BROADCAST)
        return handle_cond_error(pthread_cond_broadcast(cond), opcode);
    else if (opcode == DESTROY_COND)
        return handle_cond_error(pthread_cond_destroy(cond), opcode);
    else
        return handle_error("Invalid opcode for condition variable handling.", EINVAL);
}