#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>

#define ARRAY_SIZE (64 * 1024 * 1024)
#define REPEAT 100

size_t page_size;

void show_mapping(void *ptr) {
    FILE *file = fopen("/proc/self/maps", "r");

    if (file == NULL) {
        perror("maps");
        return;
    }

    char line[512];
    unsigned long start, end;

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
            if ((uintptr_t)ptr >= start &&
                (uintptr_t)ptr < end) {
                printf("Mapping: %s", line);
                break;
            }
        }
    }

    fclose(file);
}

uint64_t get_frame(void *ptr) {
    uint64_t address = (uintptr_t)ptr;
    uint64_t entry;

    int fd = open("/proc/self/pagemap", O_RDONLY);

    if (fd == -1) {
        perror("pagemap");
        return 0;
    }

    off_t position = (address / page_size) * 8;

    if (pread(fd, &entry, sizeof(entry), position)
        != sizeof(entry)) {
        perror("pread");
        close(fd);
        return 0;
    }

    close(fd);

    if (!(entry & (1ULL << 63))) {
        printf("Page is not present in physical memory\n");
        return 0;
    }

    uint64_t frame = entry & ((1ULL << 55) - 1);

    if (frame == 0) {
        printf("PFN unavailable. Root privileges may be required.\n");
    }

    return frame;
}

void translation(void) {
    int number = 100;

    volatile int *value = &number;
    *value = 100;

    uint64_t address = (uintptr_t)&number;
    uint64_t offset = address % page_size;
    uint64_t frame = get_frame(&number);

    printf("\n===== ADDRESS TRANSLATION =====\n");
    printf("Page size: %zu bytes\n", page_size);
    printf("Virtual address: 0x%lx\n",
           (unsigned long)address);
    printf("Virtual page number: %lu\n",
           (unsigned long)(address / page_size));
    printf("Offset: %lu\n", (unsigned long)offset);

    show_mapping(&number);

    if (frame == 0) {
        return;
    }

    uint64_t physical = frame * page_size + offset;

    printf("Physical frame: %lu\n",
           (unsigned long)frame);
    printf("Physical address: 0x%lx\n",
           (unsigned long)physical);
    printf("Physical address (decimal): %lu\n",
           (unsigned long)physical);

    printf("Calculation: %lu * %zu + %lu = %lu\n",
           (unsigned long)frame,
           page_size,
           (unsigned long)offset,
           (unsigned long)physical);
}

void shared_pages(void) {
    char filename[] = "/tmp/shared_page_XXXXXX";

    int fd = mkstemp(filename);

    if (fd == -1) {
        perror("mkstemp");
        return;
    }

    if (ftruncate(fd, page_size) == -1) {
        perror("ftruncate");
        close(fd);
        unlink(filename);
        return;
    }

    unlink(filename);

    char *parent = mmap(NULL, page_size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

    if (parent == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }

    strcpy(parent, "Shared memory");

    uint64_t parent_frame = get_frame(parent);

    printf("\n===== SHARED PAGES =====\n");
    printf("Parent virtual address: %p\n", (void *)parent);
    printf("Parent physical frame: %lu\n",
           (unsigned long)parent_frame);

    fflush(stdout);

    pid_t pid = fork();

    if (pid == 0) {
        char *child = mmap(NULL, page_size,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED, fd, 0);

        if (child == MAP_FAILED) {
            perror("mmap");
            _exit(1);
        }

        printf("Child virtual address: %p\n", (void *)child);
        printf("Child data: %s\n", child);

        uint64_t child_frame = get_frame(child);

        printf("Child physical frame: %lu\n",
               (unsigned long)child_frame);

        if (parent_frame != 0 && child_frame != 0) {
            if (parent_frame == child_frame) {
                printf("Both processes share the same frame\n");
            } else {
                printf("Physical frames are different\n");
            }
        }

        strcpy(child, "Changed by child");

        munmap(child, page_size);
        munmap(parent, page_size);
        close(fd);

        fflush(stdout);
        _exit(0);
    }

    if (pid > 0) {
        waitpid(pid, NULL, 0);

        printf("Parent receives: %s\n", parent);
    } else {
        perror("fork");
    }

    munmap(parent, page_size);
    close(fd);
}

double get_time(void) {
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec + t.tv_nsec / 1000000000.0;
}

void show_huge_pages(void *ptr) {
    FILE *file = fopen("/proc/self/smaps", "r");

    if (file == NULL) {
        return;
    }

    char line[512];
    unsigned long start, end;
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
            found = (uintptr_t)ptr >= start &&
                    (uintptr_t)ptr < end;
        }

        if (found && strncmp(line, "AnonHugePages:", 14) == 0) {
            printf("%s", line);
            break;
        }
    }

    fclose(file);
}

void tlb_test(size_t step, int huge) {
    unsigned char *array = mmap(NULL, ARRAY_SIZE,
                                PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS,
                                -1, 0);

    if (array == MAP_FAILED) {
        perror("mmap");
        return;
    }

    int advice = huge ? MADV_HUGEPAGE : MADV_NOHUGEPAGE;

    if (madvise(array, ARRAY_SIZE, advice) == -1) {
        perror("madvise");
    }

    memset(array, 1, ARRAY_SIZE);

    volatile unsigned char *data = array;
    volatile unsigned long sum = 0;
    size_t accesses = ARRAY_SIZE / page_size;

    double start = get_time();

    for (int r = 0; r < REPEAT; r++) {
        for (size_t i = 0; i < accesses; i++) {
            sum += data[i * step];
        }
    }

    double elapsed = get_time() - start;

    printf("\n===== TLB TEST =====\n");
    printf("Array size: %d MiB\n", ARRAY_SIZE / 1024 / 1024);
    printf("Page size: %zu bytes\n", page_size);
    printf("Step: %zu bytes\n", step);
    printf("Huge pages requested: %s\n", huge ? "Yes" : "No");
    printf("Accesses: %zu\n", accesses * REPEAT);
    printf("Time: %.6f seconds\n", elapsed);
    printf("Checksum: %lu\n", sum);

    show_huge_pages(array);

    munmap(array, ARRAY_SIZE);
}

int main(int argc, char *argv[]) {
    long size = sysconf(_SC_PAGESIZE);

    if (size <= 0) {
        return 1;
    }

    page_size = (size_t)size;

    if (argc != 2) {
        printf("Usage: ./memory translate\n");
        printf("Usage: ./memory shared\n");
        printf("Usage: ./memory small\n");
        printf("Usage: ./memory page\n");
        printf("Usage: ./memory huge\n");
        return 1;
    }

    if (strcmp(argv[1], "translate") == 0) {
        translation();
    } else if (strcmp(argv[1], "shared") == 0) {
        shared_pages();
    } else if (strcmp(argv[1], "small") == 0) {
        tlb_test(64, 0);
    } else if (strcmp(argv[1], "page") == 0) {
        tlb_test(page_size, 0);
    } else if (strcmp(argv[1], "huge") == 0) {
        tlb_test(page_size, 1);
    } else {
        printf("Invalid option\n");
        return 1;
    }

    return 0;
}
