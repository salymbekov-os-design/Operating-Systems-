# Task 11 — Page Faults and Thrashing

## Description

In this task, I used C to study page faults, memory access, thrashing and memory performance in Linux.

## Implementation

- Created a 128 MiB memory array.
- Compared sequential and non-sequential memory access.
- Measured minor and major page faults using `getrusage()` and `/proc/self/stat`.
- Used cgroups to limit available memory.
- Tested thrashing using a file-backed memory array.
- Compared row and column traversal of a two-dimensional array.

## Page Faults

A minor page fault happens when the required page can be provided without reading it from disk.

A major page fault happens when the operating system needs to load the required page from storage.

## Thrashing

Thrashing happens when available physical memory is not enough for the working set.

The operating system repeatedly removes and reloads memory pages. This increases page faults and can reduce performance.

I used a 128 MiB working set and compared memory limits of 256 MiB and 64 MiB.

## Row and Column Traversal

C stores arrays row by row.

Row traversal normally works faster because neighbouring elements are close to each other in memory.

Column traversal jumps between memory locations, which can increase cache and TLB misses.

## LRU

LRU means Least Recently Used.

Linux uses page replacement mechanisms that approximate LRU. Recently accessed pages are generally more likely to remain in memory.

When the working set is larger than available memory, pages may be removed and loaded again repeatedly.

## Compilation

```bash
gcc -std=c11 -Wall -Wextra -O2 paging.c -o paging
```

## Execution

```bash
./paging sequential
./paging random
./paging row
./paging column
```

Normal memory conditions:

```bash
sudo systemd-run --scope -p MemoryMax=256M -p MemorySwapMax=0 ./paging thrashing
```

Restricted memory conditions:

```bash
sudo systemd-run --scope -p MemoryMax=64M -p MemorySwapMax=0 ./paging thrashing
```

## Development Environment

- Language: C
- Operating system: Ubuntu Linux
- Virtual machine: UTM
- Compiler: GCC

## Conclusion

This task helped me understand page faults, thrashing and how memory access patterns affect system performance.
