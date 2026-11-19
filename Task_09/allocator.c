#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <time.h>
#include <stdint.h>
#include <malloc.h>

#define MEMORY_SIZE (64 * 1024 * 1024)
#define ALIGNMENT _Alignof(max_align_t)
#define COUNT 10000

typedef struct Block {
    size_t size;
    size_t requested;
    int free;
    struct Block *next;
    max_align_t alignment;
} Block;

void *memory;
Block *head;
int strategy = 0;

int init_memory(void) {
    memory = mmap(NULL, MEMORY_SIZE,
                  PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (memory == MAP_FAILED) {
        memory = NULL;
        return 0;
    }

    head = memory;
    head->size = MEMORY_SIZE - sizeof(Block);
    head->requested = 0;
    head->free = 1;
    head->next = NULL;

    return 1;
}

void set_strategy(int value) {
    strategy = value;
}

Block *find_block(size_t size) {
    Block *current = head;
    Block *best = NULL;

    while (current != NULL) {
        if (current->free && current->size >= size) {
            if (strategy == 0) {
                return current;
            }

            if (best == NULL || current->size < best->size) {
                best = current;
            }
        }

        current = current->next;
    }

    return best;
}

void split_block(Block *block, size_t size) {
    if (block->size < size + sizeof(Block) + ALIGNMENT) {
        return;
    }

    Block *new_block =
        (Block *)((char *)(block + 1) + size);

    new_block->size = block->size - size - sizeof(Block);
    new_block->requested = 0;
    new_block->free = 1;
    new_block->next = block->next;

    block->size = size;
    block->next = new_block;
}

void *my_malloc(size_t size) {
    if (size == 0 || size > MEMORY_SIZE) {
        return NULL;
    }

    size_t requested = size;

    size = (size + ALIGNMENT - 1) / ALIGNMENT * ALIGNMENT;

    Block *block = find_block(size);

    if (block == NULL) {
        return NULL;
    }

    split_block(block, size);

    block->free = 0;
    block->requested = requested;

    return block + 1;
}

Block *find_pointer(void *ptr) {
    Block *current = head;

    while (current != NULL) {
        if ((void *)(current + 1) == ptr) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

void merge_blocks(void) {
    Block *current = head;

    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            current->size += sizeof(Block) +
                             current->next->size;

            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void my_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    Block *block = find_pointer(ptr);

    if (block == NULL || block->free) {
        printf("Invalid or double free detected\n");
        return;
    }

    block->free = 1;
    block->requested = 0;

    merge_blocks();
}

void *my_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return my_malloc(size);
    }

    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    Block *block = find_pointer(ptr);

    if (block == NULL || block->free) {
        return NULL;
    }

    if (size > MEMORY_SIZE) {
        return NULL;
    }

    size_t aligned = (size + ALIGNMENT - 1) /
                     ALIGNMENT * ALIGNMENT;

    if (block->size >= aligned) {
        split_block(block, aligned);
        block->requested = size;
        merge_blocks();

        return ptr;
    }

    void *new_ptr = my_malloc(size);

    if (new_ptr == NULL) {
        return NULL;
    }

    size_t copy_size = block->requested;

    if (size < copy_size) {
        copy_size = size;
    }

    memcpy(new_ptr, ptr, copy_size);
    my_free(ptr);

    return new_ptr;
}

void memory_map(void) {
    Block *current = head;
    int number = 1;

    printf("\n===== MEMORY MAP =====\n");

    while (current != NULL) {
        printf("Block %d | Offset: %zu | Size: %zu | %s\n",
               number,
               (size_t)((char *)current - (char *)memory),
               current->size,
               current->free ? "FREE" : "USED");

        current = current->next;
        number++;
    }
}

void fragmentation(void) {
    Block *current = head;

    size_t total_free = 0;
    size_t largest_free = 0;
    size_t internal = 0;
    size_t allocated = 0;

    while (current != NULL) {
        if (current->free) {
            total_free += current->size;

            if (current->size > largest_free) {
                largest_free = current->size;
            }
        } else {
            internal += current->size - current->requested;
            allocated += current->size;
        }

        current = current->next;
    }

    double external_percentage = 0;
    double internal_percentage = 0;

    if (total_free > 0) {
        external_percentage =
            100.0 * (1.0 - (double)largest_free / total_free);
    }

    if (allocated > 0) {
        internal_percentage =
            100.0 * internal / allocated;
    }

    printf("Internal fragmentation: %.2f%%\n",
           internal_percentage);

    printf("External fragmentation: %.2f%%\n",
           external_percentage);
}

void detect_leaks(void) {
    Block *current = head;
    size_t leaks = 0;
    size_t bytes = 0;

    while (current != NULL) {
        if (!current->free) {
            leaks++;
            bytes += current->requested;
        }

        current = current->next;
    }

    printf("Memory leaks: %zu blocks, %zu bytes\n",
           leaks, bytes);
}

int check_pointer(void *ptr) {
    Block *block = find_pointer(ptr);

    if (block == NULL || block->free) {
        printf("Invalid or use-after-free access detected\n");
        return 0;
    }

    return 1;
}

void destroy_memory(void) {
    if (memory != NULL) {
        munmap(memory, MEMORY_SIZE);
    }

    memory = NULL;
    head = NULL;
}

double get_time(void) {
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec + t.tv_nsec / 1000000000.0;
}

void benchmark(int scenario, int system) {
    void *pointers[COUNT] = {NULL};

    size_t requested[COUNT] = {0};

    double start = get_time();

    for (int i = 0; i < COUNT; i++) {
        size_t size;

        if (scenario == 1) {
            size = 32;
        } else if (scenario == 2) {
            size = 32 + (i % 17) * 64;
        } else {
            size = 4096;
        }

        pointers[i] = system ? malloc(size) : my_malloc(size);
        requested[i] = size;

        if (pointers[i] == NULL) {
            printf("Allocation failed\n");
            break;
        }

        memset(pointers[i], 1, size);

        if (scenario == 2 && i % 2 == 1) {
            if (system) {
                free(pointers[i - 1]);
            } else {
                my_free(pointers[i - 1]);
            }

            pointers[i - 1] = NULL;
        }
    }

    double allocation_time = get_time() - start;

    if (!system) {
        fragmentation();
    } else {
        size_t total = 0;
        size_t unused = 0;

        for (int i = 0; i < COUNT; i++) {
            if (pointers[i] != NULL) {
                size_t usable = malloc_usable_size(pointers[i]);

                unused += usable - requested[i];
                total += usable;
            }
        }

        printf("Internal fragmentation: %.2f%%\n",
               total ? 100.0 * unused / total : 0.0);

        printf("External fragmentation: unavailable\n");
    }

    start = get_time();

    for (int i = 0; i < COUNT; i++) {
        if (system) {
            free(pointers[i]);
        } else {
            my_free(pointers[i]);
        }
    }

    double free_time = get_time() - start;

    struct rusage usage;

    getrusage(RUSAGE_SELF, &usage);

    printf("Total time: %.6f seconds\n",
           allocation_time + free_time);

    printf("Peak RSS: %ld KB\n", usage.ru_maxrss);

    if (!system) {
        detect_leaks();
    }
}

void test_allocator(void) {
    void *a = my_malloc(100);
    void *b = my_malloc(200);
    void *c = my_malloc(300);

    if (!a || !b || !c) {
        printf("Allocation failed\n");
        my_free(a);
        my_free(b);
        my_free(c);
        return;
    }

    memory_map();

    my_free(a);
    my_free(b);

    memory_map();

    void *new_c = my_realloc(c, 500);

    if (new_c != NULL) {
        c = new_c;
    }

    memory_map();

    fragmentation();

    my_free(c);

    detect_leaks();
    memory_map();
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "test") == 0) {
        if (!init_memory()) {
            return 1;
        }

        test_allocator();
        destroy_memory();

        return 0;
    }

    if (argc != 3) {
        printf("Usage: ./allocator test\n");
        printf("Usage: ./allocator first 1\n");
        printf("Usage: ./allocator best 2\n");
        printf("Usage: ./allocator glibc 3\n");

        return 1;
    }

    int scenario = atoi(argv[2]);
    int system = strcmp(argv[1], "glibc") == 0;

    if (scenario < 1 || scenario > 3) {
        printf("Invalid scenario\n");
        return 1;
    }

    if (!system) {
        if (strcmp(argv[1], "first") == 0) {
            set_strategy(0);
        } else if (strcmp(argv[1], "best") == 0) {
            set_strategy(1);
        } else {
            printf("Invalid strategy\n");
            return 1;
        }

        if (!init_memory()) {
            printf("Memory initialization failed\n");
            return 1;
        }
    }

    printf("\nAllocator: %s\n", argv[1]);
    printf("Scenario: %d\n", scenario);

    benchmark(scenario, system);

    if (!system) {
        destroy_memory();
    }

    return 0;
}
