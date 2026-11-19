# Task 11 — Variant A

## Description

In this task, I implemented three page replacement algorithms using Python: FIFO, LRU and Optimal.

## Algorithms

- FIFO removes the page that entered memory first.
- LRU removes the page that has not been used for the longest time.
- Optimal removes the page that will be needed furthest in the future.

## Testing

I created a custom reference string and tested all three algorithms using 3 and 4 memory frames.

The FIFO algorithm with 3 frames produces 15 page faults.

## Optimal Algorithm

Optimal gives the minimum possible number of page faults for a given reference string and number of frames.

However, it cannot be used directly in a real operating system because the operating system does not know which pages will be requested in the future.

## Running

```bash
python3 replace.py
```

## Results

The comparison table is saved in results.txt.
