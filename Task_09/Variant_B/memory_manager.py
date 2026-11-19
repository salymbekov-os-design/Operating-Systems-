class MemoryManager:
    def __init__(self, size, strategy, coalescing=True):
        self.blocks = [
            {"start": 0, "size": size, "name": None}
        ]

        self.strategy = strategy
        self.coalescing = coalescing
        self.failures = 0

    def allocate(self, name, size):
        suitable = [
            b for b in self.blocks
            if b["name"] is None and b["size"] >= size
        ]

        if not suitable:
            self.failures += 1
            print(f"Allocation failed: {name}")
            return

        if self.strategy == "first_fit":
            block = suitable[0]

        elif self.strategy == "best_fit":
            block = min(suitable, key=lambda b: b["size"])

        else:
            block = max(suitable, key=lambda b: b["size"])

        index = self.blocks.index(block)
        remaining = block["size"] - size

        block["size"] = size
        block["name"] = name

        if remaining > 0:
            self.blocks.insert(index + 1, {
                "start": block["start"] + size,
                "size": remaining,
                "name": None
            })

    def free(self, name):
        for block in self.blocks:
            if block["name"] == name:
                block["name"] = None
                break

        if self.coalescing:
            self.merge()

    def merge(self):
        i = 0

        while i < len(self.blocks) - 1:
            a = self.blocks[i]
            b = self.blocks[i + 1]

            if a["name"] is None and b["name"] is None:
                a["size"] += b["size"]
                self.blocks.pop(i + 1)
                print("Free blocks merged")

            else:
                i += 1

    def fragmentation(self):
        free = [
            b["size"] for b in self.blocks
            if b["name"] is None
        ]

        total = sum(free)
        largest = max(free, default=0)

        external = 0

        if total > 0:
            external = (1 - largest / total) * 100

        return external, 0

    def memory_map(self):
        print(" ".join(
            f"[{b['name'] or 'FREE'}:{b['size']}]"
            for b in self.blocks
        ))


class BuddySystem:
    def __init__(self, size):
        self.blocks = [
            {
                "start": 0,
                "size": size,
                "name": None,
                "requested": 0
            }
        ]

        self.failures = 0

    def allocate(self, name, size):
        required = 1

        while required < size:
            required *= 2

        suitable = [
            b for b in self.blocks
            if b["name"] is None and b["size"] >= required
        ]

        if not suitable:
            self.failures += 1
            print(f"Allocation failed: {name}")
            return

        block = min(suitable, key=lambda b: b["size"])

        while block["size"] > required:
            half = block["size"] // 2
            index = self.blocks.index(block)

            block["size"] = half

            self.blocks.insert(index + 1, {
                "start": block["start"] + half,
                "size": half,
                "name": None,
                "requested": 0
            })

            print(f"Split {half * 2} into {half} + {half}")

        block["name"] = name
        block["requested"] = size

    def free(self, name):
        block = next(
            (b for b in self.blocks if b["name"] == name),
            None
        )

        if block is None:
            return

        block["name"] = None
        block["requested"] = 0

        while True:
            buddy_start = block["start"] ^ block["size"]

            buddy = next(
                (
                    b for b in self.blocks
                    if b["start"] == buddy_start
                    and b["size"] == block["size"]
                    and b["name"] is None
                ),
                None
            )

            if buddy is None:
                break

            block["start"] = min(
                block["start"], buddy["start"]
            )

            block["size"] *= 2

            self.blocks.remove(buddy)

            print(f"Buddies merged into {block['size']}")

        self.blocks.sort(key=lambda b: b["start"])

    def fragmentation(self):
        free = [
            b["size"] for b in self.blocks
            if b["name"] is None
        ]

        used = [
            b for b in self.blocks
            if b["name"] is not None
        ]

        total_free = sum(free)
        largest = max(free, default=0)

        allocated = sum(b["size"] for b in used)

        wasted = sum(
            b["size"] - b["requested"] for b in used
        )

        external = 0
        internal = 0

        if total_free > 0:
            external = (1 - largest / total_free) * 100

        if allocated > 0:
            internal = wasted / allocated * 100

        return external, internal

    def memory_map(self):
        print(" ".join(
            f"[{b['name'] or 'FREE'}:{b['size']}]"
            for b in self.blocks
        ))


ops2 = [
    ("alloc", "A", 20),
    ("alloc", "B", 12),
    ("alloc", "C", 25),
    ("alloc", "D", 10),
    ("free", "A", 0),
    ("free", "B", 0),
    ("alloc", "E", 30),
    ("free", "C", 0),
    ("alloc", "F", 18),
    ("free", "D", 0),
    ("free", "E", 0),
    ("alloc", "G", 40)
]


def run_test(name, manager):
    external_values = []
    internal_values = []

    print(f"\n===== {name} =====")

    for operation, block, size in ops2:
        print(f"\nOperation: {operation} {block} {size}")

        if operation == "alloc":
            manager.allocate(block, size)
        else:
            manager.free(block)

        manager.memory_map()

        external, internal = manager.fragmentation()

        external_values.append(external)
        internal_values.append(internal)

    return (
        name,
        manager.failures,
        sum(external_values) / len(external_values),
        sum(internal_values) / len(internal_values)
    )


results = []

for strategy in ["first_fit", "best_fit", "worst_fit"]:
    manager = MemoryManager(128, strategy)
    results.append(run_test(strategy, manager))

results.append(run_test("buddy", BuddySystem(128)))

without_merge = run_test(
    "First Fit without coalescing",
    MemoryManager(128, "first_fit", False)
)

print("\n===== COMPARISON =====")

for name, failures, external, internal in results:
    print(f"\nStrategy: {name}")
    print(f"Failures: {failures}")
    print(f"Average external fragmentation: {external:.2f}%")
    print(f"Average internal fragmentation: {internal:.2f}%")

print("\n===== WITHOUT COALESCING =====")
print(without_merge)

with open("comparison.csv", "w") as file:
    file.write("Strategy,Failures,External,Internal\n")

    for name, failures, external, internal in results:
        file.write(
            f"{name},{failures},{external:.2f},{internal:.2f}\n"
        )
