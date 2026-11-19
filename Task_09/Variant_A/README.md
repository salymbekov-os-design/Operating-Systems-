# Task 09 — Variant A

## Description

In this task, I created a memory allocation simulator using Python.

The program simulates three memory allocation strategies: First Fit, Best Fit and Worst Fit.

## Allocation Strategies

- First Fit selects the first suitable free block.
- Best Fit selects the smallest suitable free block.
- Worst Fit selects the largest suitable free block.

## Testing

I created a custom operation sequence called `ops1` using 100 units of memory.

The program displays the memory map after every operation and calculates the total free memory, largest free block and external fragmentation.

## External Fragmentation

External fragmentation happens when free memory is divided into small separate blocks.

First Fit cannot allocate block F, which requires 30 units. Although there are 40 free units in total, the largest free block has only 20 units.

## Running

```bash
python3 memory_manager.py
```

## Results

The memory maps and the comparison table are saved in `results.txt`.
