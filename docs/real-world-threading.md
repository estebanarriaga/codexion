# Real-World Threading Case Studies

This file collects real examples of how large projects use threads, processes,
event loops, schedulers, and queues. The point is not to copy these designs into
Codexion directly. The point is to learn what engineering decisions teams made
when concurrency became a central design problem.

## Go Runtime: Many Goroutines, Fewer OS Threads

Go exposes goroutines as lightweight concurrent tasks. Internally, the runtime
maps them onto operating-system threads using the `G`, `M`, and `P` model:

- `G`: goroutine, the user-level task.
- `M`: machine, an OS worker thread.
- `P`: processor, the execution resource required to run Go code.

The Go runtime source says the scheduler distributes ready goroutines over
worker threads, and that an `M` needs an associated `P` to execute Go code:
<https://go.dev/src/runtime/proc.go>

Important decision:

- Go did not create one OS thread per goroutine.
- It multiplexes many goroutines onto a smaller pool of OS threads.
- It uses per-`P` run queues instead of one fully centralized queue.

Why:

- OS threads are expensive compared with goroutines.
- A single global scheduler queue would create contention.
- Per-processor queues improve locality and reduce lock pressure.
- Work stealing lets idle processors take work from busy ones.

Interesting tradeoff:

- Distributed scheduler state improves scalability.
- It also makes global decisions harder, because no one place always has a
  complete cheap view of all runnable work.

Connection to Codexion:

- Your FIFO/EDF heap is a scheduler queue.
- If you use one global simulation mutex, the design is simpler but less
  scalable.
- If you split queues per dongle, you reduce contention but make global fairness
  harder.

Further reading:

- Go runtime scheduler source: <https://go.dev/src/runtime/proc.go>
- Go scheduler design discussion: <https://groups.google.com/g/golang-dev/c/_H9nXe7jG2U>
- Go 1.2 scheduler preemption note: <https://go.dev/doc/go1.2#pre_emption_in_the_scheduler>
- Go diagnostics and scheduler tracing: <https://go.dev/doc/diagnostics>

## NGINX: Event-Driven Workers Instead Of Thread Per Connection

NGINX uses a master/worker process model. The master reads configuration and
maintains workers; workers process requests. The number of workers can be fixed
or auto-adjusted to CPU cores:
<https://docs.nginx.com/nginx/admin-guide/basic-functionality/runtime-control/>

Important decision:

- NGINX does not use one thread per client connection.
- It uses worker processes and OS event mechanisms to handle many connections
  efficiently.

Why:

- Many web connections spend most time waiting for network I/O.
- A thread per connection would waste memory and scheduler time.
- Event-driven workers can keep many sockets active with fewer execution
  contexts.

Interesting tradeoff:

- Event-driven code scales well for I/O-heavy workloads.
- Blocking inside a worker is dangerous because it can delay many connections
  handled by that worker.

Connection to Codexion:

- If a Codexion thread sleeps while holding the main mutex, it blocks unrelated
  progress in the same way a blocking NGINX worker delays many clients.
- Keep critical sections short.

Further reading:

- NGINX runtime process control: <https://docs.nginx.com/nginx/admin-guide/basic-functionality/runtime-control/>
- NGINX architecture overview: <https://blog.nginx.org/blog/inside-nginx-how-we-designed-for-performance-scale>

## PostgreSQL: Process Per Connection

PostgreSQL uses a process-per-user client/server model. A supervisor process,
called the postmaster, listens for incoming connections and spawns a backend
process for each one. Backends coordinate through semaphores and shared memory:
<https://www.postgresql.org/docs/18/connect-estab.html>

Important decision:

- PostgreSQL historically chose processes rather than threads for client
  sessions.

Why:

- Process isolation improves fault containment.
- One backend crash is less likely to corrupt another backend's private memory.
- Shared memory is used deliberately for the data that must be shared.

Interesting tradeoff:

- Processes cost more than threads.
- Very large connection counts need pooling or careful limits.
- The design favors robustness and isolation over ultra-lightweight sessions.

Connection to Codexion:

- Threads share memory by default, so every shared field is a potential race.
- PostgreSQL's design is a reminder that sharing less can make correctness
  easier.
- In Codexion, you cannot avoid threads, but you can still reduce shared mutable
  state.

Further reading:

- PostgreSQL connection model: <https://www.postgresql.org/docs/18/connect-estab.html>

## Apache HTTP Server: Multiple Concurrency Models

Apache httpd supports different Multi-Processing Modules. The `worker` MPM is a
hybrid multi-process, multi-threaded server. Its documentation says this serves
many requests with fewer resources than a process-only server while retaining
some process-based stability:
<https://httpd.apache.org/docs/current/en/mod/worker.html>

Important decision:

- Apache made the concurrency model configurable.
- `prefork` uses multiple processes.
- `worker` uses multiple processes, each with multiple threads.
- `event` further separates keep-alive connection handling from active request
  processing.

Why:

- Different deployments have different safety and scalability needs.
- Some modules historically were not thread-safe.
- Threaded workers reduce memory use when thread safety is acceptable.

Interesting tradeoff:

- More threads can increase concurrency.
- Too many threads increase memory use, scheduling overhead, and instability.
- Apache exposes limits like `MaxRequestWorkers`, `ThreadsPerChild`, and
  `ThreadLimit`.

Connection to Codexion:

- Your project has a fixed thread model: one coder equals one thread.
- But you still choose internal concurrency structure: one global lock, per
  dongle locks, central scheduler, per-dongle queues, monitor frequency.
- Like Apache, every concurrency knob affects both performance and failure modes.

Further reading:

- Apache `worker` MPM: <https://httpd.apache.org/docs/current/en/mod/worker.html>
- Apache MPM common directives: <https://httpd.apache.org/docs/current/en/mod/mpm_common.html>

## Kafka Broker: Separate Thread Pools For Separate Work

Kafka brokers separate different kinds of work into configurable thread pools.
The broker docs define:

- `num.network.threads`: threads for receiving requests and sending responses.
- `num.io.threads`: threads for processing requests, possibly including disk I/O.
- `queued.max.requests`: queued data-plane requests before network threads block.

Source:
<https://kafka.apache.org/37/configuration/broker-configs/>

Important decision:

- Kafka separates network handling from request processing.
- It uses queues between stages.
- Thread pool sizes are tunable and can be updated dynamically within limits.

Why:

- Network I/O and disk/request processing have different bottlenecks.
- Queues absorb bursts and provide backpressure.
- Separate pools make it easier to identify and tune bottlenecks.

Interesting tradeoff:

- Queues improve decoupling.
- Queues also add latency and require capacity limits.
- If processing threads are saturated, network threads may eventually block.

Connection to Codexion:

- A dongle wait heap is a queue with a scheduling policy.
- `queued.max.requests` is similar in spirit to preventing unbounded request
  buildup.
- Your monitor is like an independent control loop watching deadlines while
  workers do normal work.

Further reading:

- Kafka broker configs: <https://kafka.apache.org/37/configuration/broker-configs/>

## libuv and Node.js: Event Loop Plus Worker Thread Pool

libuv is the C library behind Node.js's event loop and async I/O model. Its
thread pool is used for work that would otherwise block the loop, such as file
system operations and DNS helpers. The docs say the pool is global, shared
across event loops, defaults to size 4, and can be changed with
`UV_THREADPOOL_SIZE`:
<https://docs.libuv.org/en/stable/threadpool.html>

Important decision:

- Keep the main event loop responsive.
- Move selected blocking operations to a bounded worker pool.

Why:

- Most network I/O can be event-driven.
- Some operations do not have a portable non-blocking OS interface.
- A bounded pool avoids unlimited thread creation.

Interesting tradeoff:

- A small pool protects memory and scheduling overhead.
- A too-small pool can become a bottleneck for file/DNS-heavy workloads.
- A too-large pool may waste memory and increase contention.

Connection to Codexion:

- Your coder threads should not block while holding shared locks.
- If a task may wait, release the mutex first or use a condition variable.
- Bounded concurrency is a design tool, not a limitation by accident.

Further reading:

- libuv thread pool: <https://docs.libuv.org/en/stable/threadpool.html>
- libuv overview: <https://docs.libuv.org/en/stable/>

## Java ForkJoinPool: Work Stealing For Recursive Tasks

Java's `ForkJoinPool` is an executor designed for tasks that split into smaller
tasks. Its documentation says it uses work stealing: worker threads try to find
and execute tasks created by other active tasks:
<https://docs.oracle.com/javase/8/docs/api/java/util/concurrent/ForkJoinPool.html>

Important decision:

- Give workers local queues.
- Let idle workers steal from busy workers.
- Default target parallelism is based on available processors.

Why:

- Recursive parallel tasks create uneven work.
- Local queues reduce contention.
- Stealing balances load without forcing every operation through one global
  queue.

Interesting tradeoff:

- Work stealing is excellent for CPU-bound fork/join workloads.
- Blocking I/O inside pool tasks can reduce effectiveness unless explicitly
  managed.

Connection to Codexion:

- A heap scheduler and a work-stealing scheduler solve different problems.
- Codexion needs fairness/deadline ordering for scarce resources, not just CPU
  load balancing.
- This is why FIFO/EDF is more relevant than "whatever thread is free".

Further reading:

- Java `ForkJoinPool`: <https://docs.oracle.com/javase/8/docs/api/java/util/concurrent/ForkJoinPool.html>

## Erlang/BEAM: Lightweight Processes And Message Passing

Erlang was designed for massive concurrency. Its processes are lightweight,
cheap to create and terminate, and communicate by message passing:
<https://www2.erlang.org/documentation/doc-15.0/doc/system/ref_man_processes.html>

Important decision:

- Erlang processes do not share normal mutable memory.
- Communication happens by sending messages to process mailboxes.
- Failures are often handled by supervision trees rather than shared-state
  recovery.

Why:

- Telecom systems need many independent concurrent activities.
- Isolation makes fault recovery easier.
- Message passing reduces data races by construction.

Interesting tradeoff:

- Message passing avoids many shared-memory bugs.
- It requires designing protocols and mailbox behavior carefully.
- Backpressure and mailbox growth become important concerns.

Connection to Codexion:

- Codexion uses pthreads, so shared memory is unavoidable.
- But you can borrow the idea of explicit communication: coder threads update
  protected state; condition variables notify waiters; the monitor observes and
  stops the system.

Further reading:

- Erlang processes: <https://www2.erlang.org/documentation/doc-15.0/doc/system/ref_man_processes.html>

## What These Cases Teach For Codexion

Real systems do not use threads blindly. They choose concurrency models based on
the resource being protected:

- CPU-bound work often benefits from worker pools and work stealing.
- I/O-heavy servers often use event loops.
- Safety-sensitive systems may prefer processes or message passing.
- Systems with scarce resources need queues, limits, and fairness policies.
- Systems with deadlines need monitors and scheduling rules.

For Codexion, the closest lessons are:

- Treat dongles as scarce resources.
- Treat wait queues as scheduler-owned data structures.
- Protect invariants, not random variables.
- Keep mutex-held sections short.
- Use condition variables for sleeping, not busy loops.
- Make the monitor independent from worker progress.
- Choose FIFO/EDF behavior deliberately and make it easy to explain.

## Navigation

- Previous: [Implementation Checklist](./implementation-checklist.md)
- Next: none
