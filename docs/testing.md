# Testing Concurrent Code

Concurrent programs need repeated testing because bugs can depend on timing.
Passing once is not enough.

## Basic Test Categories

Argument parsing:

```sh
./codexion
./codexion 4 800 200 200 200 3 0 invalid
./codexion -1 800 200 200 200 3 0 fifo
./codexion 4 abc 200 200 200 3 0 fifo
```

Single coder:

```sh
./codexion 1 800 200 200 200 1 0 fifo
```

Small normal case:

```sh
./codexion 4 1000 150 150 150 3 0 fifo
./codexion 4 1000 150 150 150 3 0 edf
```

Cooldown pressure:

```sh
./codexion 4 1000 150 150 150 3 100 fifo
./codexion 4 1000 150 150 150 3 100 edf
```

Likely burnout:

```sh
./codexion 4 200 150 150 150 3 100 fifo
```

## What To Check In Logs

For every compile:

```text
X has taken a dongle
X has taken a dongle
X is compiling
```

Check that:

- no two coders own the same dongle at the same time,
- a coder never compiles after taking only one dongle,
- no normal state appears after `burned out`,
- the burnout log is close to the expected deadline,
- all logs are complete single lines,
- the program exits after the stopping condition.

## Stress Testing

Run the same command many times:

```sh
i=0; while [ $i -lt 100 ]; do ./codexion 5 800 100 100 100 5 20 edf >/tmp/codexion.log || break; i=$((i + 1)); done
```

This kind of loop is useful because race conditions often appear only sometimes.

## Sanitizers

If available in your environment, compile temporary debug builds with sanitizers.
They may not be part of the final 42 Makefile, but they are useful locally.

ThreadSanitizer can detect data races:

```sh
cc -g -fsanitize=thread -pthread ...
```

AddressSanitizer can detect memory errors:

```sh
cc -g -fsanitize=address ...
```

Check whether your platform supports them before relying on them.

## Further Reading

- ThreadSanitizer C++ manual, still useful conceptually for C: <https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual>
- AddressSanitizer: <https://github.com/google/sanitizers/wiki/AddressSanitizer>
- Helgrind manual: <https://valgrind.org/docs/manual/hg-manual.html>

## Navigation

- Previous: [Monitor Thread and Timing](./monitor-and-timing.md)
- Next: [Implementation Checklist](./implementation-checklist.md)
