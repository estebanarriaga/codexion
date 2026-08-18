

## Requirements

- Dongle cooldown — after a dongle is released, it enters an unavailable period. This means a dongle has three states: FREE, IN_USE, COOLING. This alone makes a naive mutex-per-fork approach painful — you can't just pthread_mutex_trylock and call it done. Someone needs to track the cooldown timer.

- Scheduler as a first-class argument — FIFO and EDF are not hints, they're enforced arbitration policies. When two coders want the same dongle, the dongle must grant it to the right one. This is the core reason a priority queue (min-heap) is explicitly required by the subject.

- EDF deadline — last_compile_start + time_to_burnout. This is a real-time scheduling concept. The person closest to burning out gets priority. This is exactly what makes EDF starvation-free when parameters are viable.

- Monitor thread — separate thread, must detect burnout within 10ms. Not the coder thread self-detecting.

## Description / Notes

THe dongle wait queue (`min-heap`) is the central design piece. When a coder wants
a dongle and it's unavailable (in use or coding), it enqueues a `t_request { coder_id, arrival_time, deadline }` into the dongle's heap. When the dongle becomes available, the heap pops the winner based on scheduler policy. Arrival time for FIFO, soonest deadline for EDF.

Flow per coder thread:
```txt
loop:
  1. enqueue request into left_dongle.waitqueue
     wait on personal_cond until granted
     log "has taken a dongle"
  
  2. enqueue request into right_dongle.waitqueue
     wait on personal_cond until granted
     log "has taken a dongle"
  
  3. record last_compile_start = now()
     log "is compiling"
     usleep(time_to_compile)
     compile_count++
  
  4. release both dongles
     → set state = COOLING, record cooldown_until
     → signal dongle's available cond
     → a background mechanism or the next waiter handles cooldown
  
  5. log "is debugging" → usleep(time_to_debug)
  6. log "is refactoring" → usleep(time_to_refactor)
  7. check sim.stop → if set, exit
```

Dongle release + cooldown is the tricky part. When a dongle is released, It can't just
be set `FREE` immediately. One clean approach: set it to `COOLING`, note `cooldown_until`.
When the next waiter checks if it can take the dongle, it does a `pthread_cond_timedwait` with an absolute timeout set to exactly `cooldown_until`.

```txt
coders/
├── codexion.h          (all types, constants, prototypes)
├── main.c              (parse args, init, launch threads, join)
├── init.c              (init sim, coders, dongles, monitor)
├── coder.c             (coder thread routine + state machine)
├── dongle.c            (acquire, release, cooldown, dispatch)
├── scheduler.c         (min-heap impl: FIFO/EDF comparator)
├── monitor.c           (monitor thread, burnout detection)
├── logger.c            (serialized log output)
├── time_utils.c        (gettimeofday wrappers → ms)
└── Makefile
```

## Dictionary

**spinlock** - A synchronization primitive that causes a thread to busy-wait while attempting to acquire a lock, as opposed to blocking.
**mutex** - A synchronization primitive that allows only one thread to access a shared resource at a time, blocking other threads until the lock is released.
**condition variable** - A synchronization primitive that allows threads to wait for certain conditions to be met before proceeding, often used in conjunction with mutexes.
**interlieved** - A term used to describe the execution of multiple threads or processes in a way that their operations are interspersed or alternated, rather than running sequentially.