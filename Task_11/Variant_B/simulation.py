import random
import csv
import time

from replace import fifo, lru, optimal, reference


def clock(refs, capacity):
    frames = []
    bits = []
    pointer = 0
    faults = 0

    for page in refs:
        if page in frames:
            index = frames.index(page)
            bits[index] = 1

        else:
            faults += 1

            if len(frames) < capacity:
                frames.append(page)
                bits.append(1)

            else:
                while bits[pointer] == 1:
                    bits[pointer] = 0
                    pointer = (pointer + 1) % capacity

                frames[pointer] = page
                bits[pointer] = 1

                pointer = (pointer + 1) % capacity

    return faults


algorithms = [fifo, lru, optimal, clock]

print("===== FRAME COMPARISON =====")

with open("comparison.csv", "w") as file:
    writer = csv.writer(file)

    writer.writerow([
        "Frames", "FIFO", "LRU", "Optimal", "Clock"
    ])

    print("Frames  FIFO  LRU  Optimal  Clock")

    for capacity in range(1, 9):
        results = [
            algorithm(reference, capacity)
            for algorithm in algorithms
        ]

        writer.writerow([capacity] + results)

        print(
            f"{capacity:<8}"
            f"{results[0]:<6}"
            f"{results[1]:<5}"
            f"{results[2]:<9}"
            f"{results[3]}"
        )


belady = [
    1, 2, 3, 4, 1, 2,
    5, 1, 2, 3, 4, 5
]

print("\n===== BELADY'S ANOMALY =====")

for algorithm in algorithms:
    faults3 = algorithm(belady, 3)
    faults4 = algorithm(belady, 4)

    print(f"\nAlgorithm: {algorithm.__name__}")
    print(f"3 frames: {faults3}")
    print(f"4 frames: {faults4}")

    if faults4 > faults3:
        print("Belady's anomaly detected")

    else:
        print("No anomaly detected")


random.seed(42)

local_refs = []
random_refs = []

for i in range(1000):
    if (i // 100) % 2 == 0:
        local_refs.append(random.randint(1, 3))

    else:
        local_refs.append(random.randint(4, 6))

    random_refs.append(random.randint(1, 6))


print("\n===== LOCALITY COMPARISON =====")

for algorithm in algorithms:
    local_faults = algorithm(local_refs, 4)
    random_faults = algorithm(random_refs, 4)

    print(f"\nAlgorithm: {algorithm.__name__}")
    print(f"Local reference faults: {local_faults}")
    print(f"Random reference faults: {random_faults}")


print("\n===== LRU VS CLOCK =====")

for algorithm in [lru, clock]:
    start = time.perf_counter()

    for i in range(100):
        faults = algorithm(random_refs, 4)

    elapsed = time.perf_counter() - start

    print(f"\nAlgorithm: {algorithm.__name__}")
    print(f"Faults: {faults}")
    print(f"Execution time: {elapsed:.6f} seconds")
