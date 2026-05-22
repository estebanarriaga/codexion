# Real-World Threading Case Studies

The previous documents stayed inside Codexion. This last one zooms out.

Real software projects do not choose threads because threads are fashionable.
They choose a concurrency model because some resource is scarce, some work can
overlap, or some failure must be contained. The lesson is the same one we used
throughout Codexion:

```text
First understand the shared world. Then choose the concurrency shape that keeps
that world correct and useful.
```

The examples below are not templates to copy. They are studies in engineering
judgment.

## Go: Many Tasks, Fewer Operating-System Threads

Go programmers create goroutines freely. A server may have thousands or millions
of them over its lifetime. But the Go runtime does not create one operating
system thread per goroutine. Instead, it multiplexes goroutines onto a smaller
set of OS threads.

The runtime source describes the scheduler in terms of `G`, `M`, and `P`:

```text
G: a goroutine, the work to run
M: an OS thread, the machine that runs work
P: a processor token needed to execute Go code
```

Source: <https://go.dev/src/runtime/proc.go>

The important decision is that Go separates the programmer's idea of
concurrency from the operating system's idea of threads. A goroutine is cheap
enough to use for many logical activities. OS threads are more expensive, so the
runtime schedules many goroutines over fewer threads.

That creates a scheduler problem. If all runnable goroutines lived behind one
global lock, the scheduler itself would become a bottleneck. Go therefore uses
more distributed scheduling structures, including per-processor run queues and
work stealing.

The tradeoff is familiar. Distributed queues reduce contention, but they make
global knowledge harder. One central queue is easier to understand; many local
queues scale better.

Codexion has a smaller version of the same decision. A single global mutex and
central scheduler are easier to reason about. Per-dongle queues and locks can be
more parallel, but fairness and correctness become harder to see at once.

Further reading:

- Go runtime scheduler source: <https://go.dev/src/runtime/proc.go>
- Go scheduler design discussion: <https://groups.google.com/g/golang-dev/c/_H9nXe7jG2U>
- Go diagnostics and scheduler tracing: <https://go.dev/doc/diagnostics>

## NGINX: Many Connections, Few Workers

A web server spends much of its life waiting. Clients pause, networks delay,
connections stay open. If a server created one blocking thread for every
connection, memory and scheduling overhead would grow quickly.

NGINX chose a different shape. It uses a master process and worker processes.
Workers handle many connections using event-driven I/O rather than one thread
per connection. The number of workers can be configured or chosen automatically
from CPU count.

Source: <https://docs.nginx.com/nginx/admin-guide/basic-functionality/runtime-control/>

The scarce resource here is not only CPU. It is also memory, kernel scheduling
attention, and worker responsiveness. Event-driven workers let one worker track
many connections because most connections are not actively computing most of the
time.

The tradeoff is that a worker must avoid blocking. If one event-driven worker
blocks on a slow operation, every connection assigned to that worker can suffer.

Codexion has the same lesson in miniature. If a coder holds the main simulation
mutex while sleeping through compilation, debugging, or refactoring, it freezes
unrelated progress. The code is "locked," but the design is wrong. Locks should
protect decisions, not long passages of time.

Further reading:

- NGINX runtime process control: <https://docs.nginx.com/nginx/admin-guide/basic-functionality/runtime-control/>
- NGINX architecture overview: <https://blog.nginx.org/blog/inside-nginx-how-we-designed-for-performance-scale>

## PostgreSQL: Isolation Through Processes

PostgreSQL historically uses a process-per-connection model. A supervisor
process, the postmaster, accepts connections and starts backend processes.
Backends coordinate where necessary through shared memory and synchronization
primitives.

Source: <https://www.postgresql.org/docs/18/connect-estab.html>

This is a different answer to concurrency. Instead of sharing one address space
between many client sessions, PostgreSQL gives each session a process. Processes
are heavier than threads, but they provide stronger isolation. A crash in one
backend is less likely to corrupt another backend's private memory.

The engineering choice values robustness and containment. The cost is that large
numbers of connections are expensive, so production deployments often use
connection pooling and careful limits.

Codexion cannot choose processes; the subject requires pthreads. But the lesson
still matters. Shared memory is not free. Every shared field becomes part of
your correctness story. If a field does not need to be shared, keep it local. If
it must be shared, protect the truth it belongs to.

Further reading:

- PostgreSQL connection model: <https://www.postgresql.org/docs/18/connect-estab.html>

## Apache HTTP Server: Concurrency As A Configurable Policy

Apache httpd supports different Multi-Processing Modules. The `prefork` model
uses processes. The `worker` model uses multiple processes with multiple threads
inside each process. The `event` model improves handling of keep-alive
connections.

The `worker` documentation describes the hybrid process/thread model as a way to
serve many requests with fewer resources than a process-only design while still
retaining some process-level stability.

Source: <https://httpd.apache.org/docs/current/en/mod/worker.html>

Apache's important decision is not one specific model. It is that the model is a
policy. Different modules, platforms, and deployments have different safety and
scaling needs. Some code is not thread-safe, so a process-heavy model may be
safer. Other deployments can use threads to reduce memory cost.

This teaches a practical lesson: concurrency knobs are not cosmetic. More
threads can increase throughput, but they also increase memory use, scheduling
overhead, and the number of interleavings you must survive.

Codexion gives you a fixed outer model: one coder is one thread. But inside that
constraint you still choose policy. One global lock or several locks. One
central scheduler or per-dongle queues. Frequent monitor scans or more precise
deadline waits. Each choice moves complexity around.

Further reading:

- Apache `worker` MPM: <https://httpd.apache.org/docs/current/en/mod/worker.html>
- Apache MPM common directives: <https://httpd.apache.org/docs/current/en/mod/mpm_common.html>

## Kafka: Separate Thread Pools For Different Kinds Of Work

Kafka brokers separate network work from request processing work. Broker
configuration includes network threads, I/O threads, and limits on queued
requests.

Source: <https://kafka.apache.org/37/configuration/broker-configs/>

The reason is that not all work has the same bottleneck. Receiving requests and
sending responses stress the network path. Processing requests may involve disk,
replication state, memory, and coordination. Putting all of that behind one
undifferentiated worker pool would make bottlenecks harder to isolate.

Queues connect the stages. A queue absorbs bursts, but it also introduces a new
truth: how much waiting is allowed? Kafka exposes limits because unbounded queues
turn overload into memory growth and latency.

Codexion's heap is also a queue with policy. It is smaller than Kafka's request
pipeline, but the reasoning is similar. A waiting structure must have an order,
a limit or lifetime rule, and a clear owner. Otherwise waiting becomes invisible
complexity.

Further reading:

- Kafka broker configs: <https://kafka.apache.org/37/configuration/broker-configs/>

## libuv And Node.js: Event Loop Plus Worker Pool

Node.js is known for its event loop, and libuv is the C library that provides
much of that machinery. Many operations can be handled asynchronously by the
event loop. Some operations, such as filesystem work and some DNS helpers, would
block on many systems. libuv moves those operations to a bounded thread pool.

The libuv documentation says the thread pool is global, shared across event
loops, defaults to size 4, and can be configured with `UV_THREADPOOL_SIZE`.

Source: <https://docs.libuv.org/en/stable/threadpool.html>

This is a hybrid decision. Use an event loop where waiting can be represented as
events. Use worker threads where the operation would block the loop. Keep the
pool bounded so the program does not create unlimited threads under load.

The tradeoff is capacity. A small pool protects the process from excessive
thread overhead, but it can become a bottleneck if many blocking operations
arrive. A large pool may increase throughput for some workloads but can waste
memory and create contention.

Codexion's lesson is direct: bounded concurrency is design, not weakness. The
number of coders, dongles, queues, and waiters defines what the system can do.
When a thread cannot proceed, it should wait in a controlled way rather than
creating more uncontrolled activity.

Further reading:

- libuv thread pool: <https://docs.libuv.org/en/stable/threadpool.html>
- libuv overview: <https://docs.libuv.org/en/stable/>

## Java ForkJoinPool: Local Queues And Work Stealing

Java's `ForkJoinPool` is designed for tasks that split into smaller tasks.
Worker threads keep local work queues. When a worker runs out of work, it can
steal from another worker.

Source: <https://docs.oracle.com/javase/8/docs/api/java/util/concurrent/ForkJoinPool.html>

The scarce resource here is CPU execution time. The challenge is load balance:
recursive tasks do not always divide evenly. A single central queue would create
contention. Local queues preserve locality, and stealing lets idle workers help
where work remains.

This design is excellent for CPU-bound fork/join problems. It is less natural
for arbitrary blocking I/O unless blocking is managed carefully, because a
blocked worker is not helping execute CPU work.

Codexion needs a different scheduler because its scarce resource is not CPU
work. The scarce resources are dongles with cooldown and deadlines. Work
stealing asks "which worker needs more tasks?" Codexion asks "which waiting
coder should receive this scarce resource?" That is why FIFO and EDF matter more
than simple load balancing.

Further reading:

- Java `ForkJoinPool`: <https://docs.oracle.com/javase/8/docs/api/java/util/concurrent/ForkJoinPool.html>

## Erlang/BEAM: Concurrency Without Shared Mutable State

Erlang was designed for systems with many concurrent activities. Erlang
processes are lightweight and communicate by message passing.

Source: <https://www2.erlang.org/documentation/doc-15.0/doc/system/ref_man_processes.html>

The important decision is that normal Erlang processes do not share mutable
memory the way pthreads do. One process sends a message. Another receives it in
a mailbox. This removes many data races by construction because there is no
ordinary shared variable for two processes to mutate at the same time.

The tradeoff is that you must design communication protocols. Mailboxes can grow
if producers outrun consumers. Message ordering and backpressure become central
questions.

Codexion uses pthreads, so shared memory is unavoidable. But you can still
borrow the discipline: make communication explicit. A coder updates protected
state. A condition variable notifies waiters. The monitor sets `sim->stop` and
broadcasts. Treat those transitions as protocol messages inside shared memory.

Further reading:

- Erlang processes: <https://www2.erlang.org/documentation/doc-15.0/doc/system/ref_man_processes.html>

## The Shared Lesson

These systems look different because they protect different worlds.

Go protects cheap logical concurrency over expensive OS threads. NGINX protects
worker responsiveness across many mostly-idle connections. PostgreSQL protects
isolation and fault containment. Apache exposes concurrency policy because
deployments differ. Kafka separates work by bottleneck and uses queues for
pressure. libuv keeps an event loop responsive with a bounded worker pool. Java
ForkJoinPool balances CPU work. Erlang avoids shared mutable state through
message passing.

Codexion is smaller, but the same thinking applies. Your world has scarce
dongles, cooldown history, waiting coders, deadlines, and a printed history.
Choose your locks, queues, condition variables, and monitor behavior to protect
that world.

The mature question is never "should I use threads?" The mature question is:

```text
What resource is scarce, what truth must remain coherent, and what concurrency
shape preserves that truth with the least unnecessary complexity?
```

## Navigation

- Previous: [Implementation Checklist](./implementation-checklist.md)
- Next: none
