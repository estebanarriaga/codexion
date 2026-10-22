*This project has been created as part of the 42 curriculum by estarria.*

Agents: Prefered images, image footnotes, markdown text and code sections and paragraphs. Avoid abuse of lists, too much hierarchy (#, ##, ###, ####, some things can be explain in two paragraphs instead of two sections), avoid markdown graphs to explain concepts (abuse of mermaid graphs). 

Syncronize coders in a 

# Description

This project coordinates a set of coders (1...Cn) and dongles (1...Dn) that have to perform a set of tasks on a coordinated way

![alt text](codexion-round-table.png)
Agents: add footnote of this image showing the circle table with coders, their left right dongles, scheduler in the center.

# Instructions

Run the program:
```bash
make run
```
or
```bash
make
# arg[1...8] : number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refractor number_of_compiles_required dongle_cooldown scheduler
# 
./codexion 5 800 200 200 200 3 100 fifo
```

Agents:  Add simple bash commands to test valgrind memory and thread sync (valgrind, helgrind, etc)

# Implementation Descisions

Agents: Explain synchronization well. Explain the different things we avoided: deadlocks, critical sections (..._locked() functions) and critical data. Use images (only trustable sources, wikipedia or our references, do not generate images, prefer visual simplicity and research paper like kind of images) and C code sections to visually explain the concept if it adds value. E.g: [...] log serialization: show a markdown text visually showing a log that was printed on top of another log.

# Binary Min-Heap

Agents: Explain what a priority queue (use explanations similar to Introduction to Algorithms: see references). Ive added some things about it.

Min-heaps commonly implement priority queues.

The relationships between nodes are calculated using their array indices:

\\[ \begin{aligned} \text{parent}(i)&=\lfloor(i-1)/2\rfloor\\\ \text{left}(i)&=2i+1\\\ \text{right}(i)&=2i+2 \end{aligned} \\]


![alt text](image.png)
Agents: Add this text as image footnote. A max-heap viewed as (a) a binary tree and (b) an array. The number within the circle
at each node in the tree is the value stored at that node. The number above a node is the corresponding
index in the array. Above and below the array are lines showing parent-child relationships; parents
are always to the left of their children. The tree has height three; the node at index 4 (with value 8)
has height one.

# Resources

Agents: This are all the resources used to this project + AI agents. They are super important when explainint our implementation, the style of their explanation, make sure to use it when rewriting and explainig.

  Kerrisk, Michael. The Linux Programming Interface: A Linux and UNIX System Programming Handbook. No Starch Press, 2010. pp 661-669
  Cormen, Thomas H., editor. Introduction to Algorithms. 2nd. ed., 10th pr, MIT Press [u.a.], 2007. pp 151 - 170


## Dictionary

Agents: Add words that may be relevant (critical section, log serialization)

**spinlock** - A synchronization primitive that causes a thread to busy-wait while attempting to acquire a lock, as opposed to blocking.
**mutex** - A synchronization primitive that allows only one thread to access a shared resource at a time, blocking other threads until the lock is released.
**condition variable** - A synchronization primitive that allows threads to wait for certain conditions to be met before proceeding, often used in conjunction with mutexes.
**interlieved** - A term used to describe the execution of multiple threads or processes in a way that their operations are interspersed or alternated, rather than running sequentially.


> Una sección “Blocking cases handled” que describa todos los problemas de concurrencia gestionados en la solución creada (por ejemplo, prevención de interbloqueos y condiciones de Coffman, prevención de inanición, gestión del cooldown, detección precisa del agotamiento y serialización del log).
> Una sección “Thread synchronization mechanisms” explicando las primitivas de hilos específicas usadas en la implementación (pthread_mutex_t, pthread_cond_t, implementación personalizada de eventos) y cómo coordinan el acceso a recursos compartidos (dongles, logging, estado del monitor). Debe incluir ejemplos de cómo se evitan las condiciones de carrera y cómo se logra una comunicación thread-safe entre las personas que programan y el monitor.