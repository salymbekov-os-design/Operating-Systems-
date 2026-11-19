class MemoryManager:
    def __init__(self, size):
        self.blocks = [
            {"start": 0, "size": size, "name": None}
        ]

    def first_fit(self, size):
        for block in self.blocks:
            if block["name"] is None and block["size"] >= size:
                return block
        return None

    def best_fit(self, size):
        suitable = [
            block for block in self.blocks
            if block["name"] is None and block["size"] >= size
        ]

        return min(suitable, key=lambda b: b["size"], default=None)

    def worst_fit(self, size):
        suitable = [
            block for block in self.blocks
            if block["name"] is None and block["size"] >= size
        ]

        return max(suitable, key=lambda b: b["size"], default=None)

    def allocate(self, name, size, strategy):
        block = getattr(self, strategy)(size)

        if block is None:
            print(f"Allocation failed: {name} ({size})")
            return

        index = self.blocks.index(block)

        remaining = block["size"] - size

        block["name"] = name
        block["size"] = size

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
                return

    def memory_map(self):
        for block in self.blocks:
            name = block["name"] or "FREE"

            print(
                f"[{name}: {block['start']}, "
                f"size={block['size']}]",
                end=" "
            )

        print()

    def fragmentation(self):
        free_blocks = [
            block["size"] for block in self.blocks
            if block["name"] is None
        ]

        total = sum(free_blocks)
        largest = max(free_blocks, default=0)

        percentage = 0

        if total > 0:
            percentage = (1 - largest / total) * 100

        return total, largest, percentage


ops1 = [
    ("alloc", "A", 20),
    ("alloc", "B", 30),
    ("alloc", "C", 20),
    ("free", "B", 0),
    ("alloc", "D", 15),
    ("alloc", "E", 25),
    ("free", "A", 0),
    ("alloc", "F", 30)
]

results = []

for strategy in ["first_fit", "best_fit", "worst_fit"]:
    print(f"\n===== {strategy.upper()} =====")

    manager = MemoryManager(100)

    for operation, name, size in ops1:
        print(f"\nOperation: {operation} {name} {size}")

        if operation == "alloc":
            manager.allocate(name, size, strategy)
        else:
            manager.free(name)

        manager.memory_map()

    total, largest, fragmentation = manager.fragmentation()

    results.append((strategy, total, largest, fragmentation))

print("\n===== FINAL RESULTS =====")

print(f"{'Strategy':<15} {'Free':<10} {'Largest':<10} {'Fragmentation'}")

for strategy, total, largest, fragmentation in results:
    print(
        f"{strategy:<15} {total:<10} "
        f"{largest:<10} {fragmentation:.2f}%"
    )
