reference = [
    1, 2, 3, 4, 1, 2, 5, 1, 2,
    3, 4, 5, 6, 7, 8, 9, 10, 11
]


def fifo(refs, capacity):
    frames = []
    faults = 0

    for page in refs:
        if page not in frames:
            faults += 1

            if len(frames) == capacity:
                frames.pop(0)

            frames.append(page)

    return faults


def lru(refs, capacity):
    frames = []
    faults = 0

    for page in refs:
        if page in frames:
            frames.remove(page)
            frames.append(page)

        else:
            faults += 1

            if len(frames) == capacity:
                frames.pop(0)

            frames.append(page)

    return faults


def optimal(refs, capacity):
    frames = []
    faults = 0

    for i, page in enumerate(refs):
        if page not in frames:
            faults += 1

            if len(frames) == capacity:
                future = refs[i + 1:]

                victim = max(
                    frames,
                    key=lambda p: future.index(p)
                    if p in future else float("inf")
                )

                frames.remove(victim)

            frames.append(page)

    return faults


if __name__ == "__main__":
    print("Reference string:", reference)

    print("\nAlgorithm     3 Frames     4 Frames")

    for algorithm in [fifo, lru, optimal]:
        print(
            f"{algorithm.__name__:<14}"
            f"{algorithm(reference, 3):<13}"
            f"{algorithm(reference, 4)}"
        )

    print("\nFIFO reference check:")

    if fifo(reference, 3) == 15:
        print("PASSED")

    else:
        print("FAILED")
