import csv
import random
from collections import OrderedDict

PAGE_SIZE = 1024
MEMORY_TIME = 100
TLB_TIME = 10

page_table = {
    0: 5,
    1: 2,
    2: 7,
    3: 0
}


class TLB:
    def __init__(self, capacity):
        self.capacity = capacity
        self.cache = OrderedDict()
        self.hits = 0
        self.misses = 0

    def access(self, page):
        if page in self.cache:
            self.hits += 1
            self.cache.move_to_end(page)
            return self.cache[page], "HIT"

        self.misses += 1

        if page not in page_table:
            return None, "MISS"

        if len(self.cache) == self.capacity:
            self.cache.popitem(last=False)

        self.cache[page] = page_table[page]

        return page_table[page], "MISS"

    def hit_rate(self):
        total = self.hits + self.misses

        if total == 0:
            return 0

        return self.hits / total


def effective_time(rate):
    hit_time = TLB_TIME + MEMORY_TIME
    miss_time = TLB_TIME + 2 * MEMORY_TIME

    return rate * hit_time + (1 - rate) * miss_time


def run_test(addresses, capacity, show=False):
    tlb = TLB(capacity)

    for addr in addresses:
        page = addr // PAGE_SIZE
        offset = addr % PAGE_SIZE

        frame, status = tlb.access(page)

        if show:
            if frame is None:
                print(f"{addr}: {status} | Page not present")
            else:
                physical = frame * PAGE_SIZE + offset
                print(f"{addr}: {status} | Physical: {physical}")

    return tlb


with open("refs.txt") as file:
    addresses = [int(line.strip()) for line in file]

print("===== TLB TEST =====")

tlb = run_test(addresses, 2, True)

print(f"Hits: {tlb.hits}")
print(f"Misses: {tlb.misses}")
print(f"Hit rate: {tlb.hit_rate() * 100:.2f}%")

valid_addresses = [
    addr for addr in addresses
    if addr // PAGE_SIZE in page_table
]

valid_tlb = run_test(valid_addresses, 2)

print(f"Valid address hit rate: {valid_tlb.hit_rate() * 100:.2f}%")
print(f"Estimated access time: {effective_time(valid_tlb.hit_rate()):.2f} ns")

print("\n===== EFFECTIVE ACCESS TIME =====")

for rate in [0, 0.25, 0.50, 0.75, 0.90, 1]:
    print(
        f"Hit rate: {rate * 100:.0f}% | "
        f"Time: {effective_time(rate):.2f} ns"
    )

random.seed(42)

long_refs = []

for i in range(10000):
    page = random.choices(
        [0, 1, 2, 3],
        weights=[40, 40, 10, 10]
    )[0]

    offset = random.randint(0, PAGE_SIZE - 1)

    long_refs.append(page * PAGE_SIZE + offset)

with open("long_refs.txt", "w") as file:
    for addr in long_refs:
        file.write(f"{addr}\n")

print("\n===== TLB SIZE COMPARISON =====")

with open("comparison.csv", "w") as file:
    writer = csv.writer(file)
    writer.writerow(["Size", "HitRate"])

    for size in [1, 2, 3, 4, 8, 16]:
        result = run_test(long_refs, size)
        rate = result.hit_rate() * 100

        print(f"TLB size: {size} | Hit rate: {rate:.2f}%")

        writer.writerow([size, rate])

print("\n===== TWO-LEVEL PAGE TABLE =====")

directory = {
    0: {0: 5, 1: 2},
    1: {0: 7, 1: 0}
}

address = 1200

page = address // PAGE_SIZE
offset = address % PAGE_SIZE

level1 = page // 2
level2 = page % 2

frame = directory[level1][level2]
physical = frame * PAGE_SIZE + offset

print(f"Virtual address: {address}")
print(f"Level 1 index: {level1}")
print(f"Level 2 index: {level2}")
print(f"Physical frame: {frame}")
print(f"Physical address: {physical}")
print("Memory accesses for translation: 2")
print("Memory accesses including data: 3")
print("Total access time without TLB: 300 ns")
