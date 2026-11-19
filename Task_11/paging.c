#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <time.h>

#define SIZE (128 * 1024 * 1024)
#define N 4096

typedef struct {
    long minor;
    long major;
} Faults;

double get_time(void) {
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec + t.tv_nsec / 1000000000.0;
}

Faults get_faults(void) {
    struct rusage usage;

    getrusage(RUSAGE_SELF, &usage);

    Faults faults;

    faults.minor = usage.ru_minflt;
    faults.major = usage.ru_majflt;

    return faults;
}

Faults proc_faults(void) {
    Faults faults = {-1, -1};

    FILE *file = fopen("/proc/self/stat", "r");

    if (file == NULL) {
        return faults;
    }

    char buffer[4096];

    if (fgets(buffer, sizeof(buffer), file)) {
        char *position = strrchr(buffer, ')');

        if (position != NULL) {
            char *token = strtok(position + 2, " ");
            int field = 3;

            while (token != NULL) {
                if (field == 10) {
                    faults.minor = atol(token);
                }

                if (field == 12) {
                    faults.major = atol(token);
                    break;
                }

                token = strtok(NULL, " ");
                field++;
            }
        }
    }

    fclose(file);

    return faults;
}

void print_results(double start, Faults before,
                   Faults proc_before) {
    double elapsed = get_time() - start;

    Faults after = get_faults();
    Faults proc_after = proc_faults();

    printf("Execution time: %.6f seconds\n", elapsed);

    printf("\ngetrusage results:\n");
    printf("Minor faults: %ld\n", after.minor - before.minor);
    printf("Major faults: %ld\n", after.major - before.major);

    if (proc_before.minor >= 0 && proc_after.minor >= 0 &&
        proc_before.major >= 0 && proc_after.major >= 0) {
        printf("\n/proc/self/stat results:\n");
        printf("Minor faults: %ld\n",
               proc_after.minor - proc_before.minor);
        printf("Major faults: %ld\n",
               proc_after.major - proc_before.major);
    }
}

void page_fault_test(int random_access) {
    size_t page = (size_t)sysconf(_SC_PAGESIZE);
    size_t pages = SIZE / page;

    char *array = mmap(NULL, SIZE,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1, 0);

    if (array == MAP_FAILED) {
        perror("mmap");
        return;
    }

    volatile unsigned char *data = (unsigned char *)array;

    Faults proc_before = proc_faults();
    Faults before = get_faults();

    double start = get_time();

    for (size_t i = 0; i < pages; i++) {
        size_t index;

        if (random_access) {
            index = (i * 7919) % pages;
        } else {
            index = i;
        }

        data[index * page] = 1;
    }

    printf("===== PAGE FAULT TEST =====\n");
    printf("Array size: %d MiB\n", SIZE / 1024 / 1024);
    printf("Access type: %s\n",
           random_access ? "Non-sequential" : "Sequential");

    print_results(start, before, proc_before);

    munmap(array, SIZE);
}

int thrashing_test(void) {
    char filename[] = "./paging_data_XXXXXX";

    int fd = mkstemp(filename);

    if (fd == -1) {
        perror("mkstemp");
        return 1;
    }

    unlink(filename);

    char buffer[65536];

    memset(buffer, 1, sizeof(buffer));

    for (size_t i = 0; i < SIZE; i += sizeof(buffer)) {
        if (write(fd, buffer, sizeof(buffer)) !=
            (ssize_t)sizeof(buffer)) {
            perror("write");
            close(fd);
            return 1;
        }
    }

    if (fsync(fd) == -1) {
        perror("fsync");
        close(fd);
        return 1;
    }

    posix_fadvise(fd, 0, SIZE, POSIX_FADV_DONTNEED);

    char *array = mmap(NULL, SIZE, PROT_READ,
                       MAP_SHARED, fd, 0);

    if (array == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    madvise(array, SIZE, MADV_RANDOM);

    size_t page = (size_t)sysconf(_SC_PAGESIZE);
    size_t pages = SIZE / page;

    volatile unsigned char *data = (unsigned char *)array;
    unsigned long sum = 0;

    printf("===== THRASHING TEST =====\n");
    printf("Working set: %d MiB\n", SIZE / 1024 / 1024);

    for (int r = 0; r < 3; r++) {
        Faults proc_before = proc_faults();
        Faults before = get_faults();

        double start = get_time();

        for (size_t i = 0; i < pages; i++) {
            sum += data[i * page];
        }

        printf("\nPass: %d\n", r + 1);

        print_results(start, before, proc_before);
    }

    printf("Checksum: %lu\n", sum);

    munmap(array, SIZE);
    close(fd);

    return 0;
}

void matrix_test(int column) {
    int *matrix = malloc((size_t)N * N * sizeof(int));

    if (matrix == NULL) {
        printf("Allocation failed\n");
        return;
    }

    for (size_t i = 0; i < (size_t)N * N; i++) {
        matrix[i] = 1;
    }

    volatile int *data = matrix;
    unsigned long sum = 0;

    Faults proc_before = proc_faults();
    Faults before = get_faults();

    double start = get_time();

    if (column) {
        for (int j = 0; j < N; j++) {
            for (int i = 0; i < N; i++) {
                sum += data[(size_t)i * N + j];
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                sum += data[(size_t)i * N + j];
            }
        }
    }

    printf("===== MATRIX TEST =====\n");
    printf("Matrix: %d x %d\n", N, N);
    printf("Traversal: %s\n", column ? "Column" : "Row");
    printf("Checksum: %lu\n", sum);

    print_results(start, before, proc_before);

    free(matrix);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./paging sequential\n");
        printf("Usage: ./paging random\n");
        printf("Usage: ./paging thrashing\n");
        printf("Usage: ./paging row\n");
        printf("Usage: ./paging column\n");

        return 1;
    }

    if (strcmp(argv[1], "sequential") == 0) {
        page_fault_test(0);
    } else if (strcmp(argv[1], "random") == 0) {
        page_fault_test(1);
    } else if (strcmp(argv[1], "thrashing") == 0) {
        return thrashing_test();
    } else if (strcmp(argv[1], "row") == 0) {
        matrix_test(0);
    } else if (strcmp(argv[1], "column") == 0) {
        matrix_test(1);
    } else {
        printf("Invalid option\n");
        return 1;
    }

    return 0;
}
