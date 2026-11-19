# Custom Memory Allocator — Variant C

## 1. Project Overview

The purpose of this project is to create a custom memory allocator using the C programming language and compare it with the standard memory allocator in Linux.

The program uses `mmap()` to request 64 MiB of memory from the operating system. This memory is divided into smaller blocks when needed.

## 2. Main Functions

- `my_malloc()` — allocates memory.
- `my_free()` — releases allocated memory.
- `my_realloc()` — changes the size of an allocated memory block.
- `split_block()` — divides a large memory block into smaller blocks.
- `merge_blocks()` — combines neighbouring free memory blocks.
- `memory_map()` — displays the current state of memory.
- `fragmentation()` — calculates memory fragmentation.
- `detect_leaks()` — checks for memory that has not been released.
- `check_pointer()` — checks whether a memory pointer is valid.

## 3. Memory Allocation Strategies

### First Fit

First Fit searches for the first available memory block that is large enough for the request.

It can be faster because it stops searching when a suitable block is found.

### Best Fit

Best Fit searches for the smallest available memory block that is large enough for the request.

It can reduce wasted space, but it may take longer because it searches through the available blocks.

## 4. Benchmark Scenarios

The custom allocator is compared with the standard Linux allocator (glibc malloc) in three scenarios:

1. Allocating many small memory blocks.
2. Allocating and freeing memory blocks repeatedly.
3. Allocating large memory blocks.

The program measures execution time, peak memory usage (RSS), and memory fragmentation.

## 5. Memory Fragmentation

Internal fragmentation happens when an allocated block is larger than the requested memory.

External fragmentation happens when free memory is divided into separate small blocks.

The program merges neighbouring free blocks to reduce external fragmentation. External fragmentation is measured directly for the custom allocator.

## 6. Compilation

```bash
gcc -std=c11 -Wall -Wextra -O2 allocator.c -o allocator
```

## 7. Running the Program

Test the custom allocator:

```bash
./allocator test
```

Run First Fit:

```bash
./allocator first 1
```

Run Best Fit:

```bash
./allocator best 1
```

Run the standard Linux allocator:

```bash
./allocator glibc 1
```

Scenario numbers 1, 2 and 3 can be used to select different benchmark scenarios.

## 8. Memory Testing

Valgrind is used to check for memory errors and memory leaks.

```bash
valgrind --tool=memcheck --leak-check=full ./allocator test
```

The custom allocator also includes simple memory leak and pointer-checking functions.

## 9. Development Environment

- Programming language: C
- Operating system: Ubuntu Linux
- Virtual machine: UTM
- Compiler: GCC
- Testing tool: Valgrind

## 10. Conclusion

This project helped me understand how memory allocation works in operating systems. I implemented a custom memory allocator, two allocation strategies, memory block splitting and merging, and fragmentation calculations.

The project demonstrates how different allocation strategies can affect execution time and memory usage.
