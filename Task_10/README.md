# Task 10 — Virtual Memory and TLB

## Description

In this task, I used C to study virtual memory, physical addresses, shared pages and TLB performance in Linux.

## Implementation

- Used `/proc/self/maps` to find the memory region.
- Used `/proc/self/pagemap` to find the physical frame number.
- Calculated the physical address using the frame number, page size and offset.
- Used `mmap()` and `fork()` to demonstrate shared memory between two processes.
- Compared memory access time using different step sizes.
- Used `MADV_HUGEPAGE` to test huge pages.

## Physical Address Calculation

Physical Address = Frame Number × Page Size + Offset

Root permissions are required because Linux restricts access to physical frame numbers for security reasons.

## Compilation

```bash
gcc -std=c11 -Wall -Wextra -O2 memory.c -o memory
```

## Execution

```bash
sudo ./memory translate
sudo ./memory shared
./memory small
./memory page
./memory huge
```

## Performance Testing

```bash
sudo perf stat -e dTLB-load-misses ./memory page
```

UTM does not support the required TLB hardware counter in my environment. Therefore, execution time was measured, but the exact number of TLB misses could not be obtained.

The huge pages experiment successfully allocated 64 MiB of huge-page-backed memory.

## Control Questions

1. Without the TLB, the processor needs to access the page table before accessing the requested data. This requires additional memory accesses and reduces performance.

2. A page table entry contains the physical frame number, present bit, access permissions, accessed bit and dirty bit.

## Conclusion

This task helped me understand address translation, shared memory and how the TLB improves memory access performance.
