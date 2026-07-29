# Dongle Assignment In `init_coder`

`init_coder()` stores two dongle indexes in each coder:

```c
coder->id = i + 1;
coder->left_dongle = i;
coder->right_dongle = (i + 1) % sim->params.number_of_coders;
```

The loop index `i` is zero-based, but the visible coder id is one-based. So
`sim->coders[0]` becomes coder `1`, `sim->coders[1]` becomes coder `2`, and so
on.

The dongles stay zero-based because they are indexes into `sim->dongles`.

## Formula

For `N` coders:

```text
coder id       = i + 1
left dongle    = i
right dongle   = (i + 1) % N
```

The modulo operator wraps the last coder back to dongle `0`, closing the circle.

## Example With 5 Coders

```text
                 dongle 0
            +----------------+
            |                |
        coder 1          coder 5
        L=0 R=1          L=4 R=0
            |                |
       dongle 1          dongle 4
            |                |
        coder 2          coder 4
        L=1 R=2          L=3 R=4
            |                |
            +---- coder 3 ---+
                 L=2 R=3
                 dongle 2
```

Read each coder as holding the dongle at its own index on the left, and the next
dongle on the right.

## Assignment Table

For `N = 5`, the loop creates this mapping:

```text
+----------+----------+--------------+----------------+
| loop i   | coder id | left_dongle  | right_dongle   |
+----------+----------+--------------+----------------+
| 0        | 1        | 0            | (0 + 1) % 5 = 1|
| 1        | 2        | 1            | (1 + 1) % 5 = 2|
| 2        | 3        | 2            | (2 + 1) % 5 = 3|
| 3        | 4        | 3            | (3 + 1) % 5 = 4|
| 4        | 5        | 4            | (4 + 1) % 5 = 0|
+----------+----------+--------------+----------------+
```

The important last row is coder `5`:

```text
right_dongle = (4 + 1) % 5
             = 5 % 5
             = 0
```

That is how the final coder shares dongle `0` with coder `1`.

## Neighbor Sharing

Each dongle is shared by two neighboring coders:

```text
dongle 0 -> coder 1 left,  coder 5 right
dongle 1 -> coder 2 left,  coder 1 right
dongle 2 -> coder 3 left,  coder 2 right
dongle 3 -> coder 4 left,  coder 3 right
dongle 4 -> coder 5 left,  coder 4 right
```

So `init_coder()` does not allocate new dongles per coder. It gives every coder
two indexes into the same shared `sim->dongles` array.

## Navigation

- Previous: [Project Model](./project-model.md)
- Next: [POSIX Threads Syntax](./pthread-syntax.md)
