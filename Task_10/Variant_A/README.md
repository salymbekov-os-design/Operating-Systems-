# Task 10 — Variant A

## Description

In this task, I created a Python program that translates virtual addresses into physical addresses using a page table.

The page size is 1024 bytes.

## Page Table

| Page | Frame |
|---|---|
| 0 | 5 |
| 1 | 2 |
| 2 | 7 |
| 3 | 0 |

## Manual Calculations

### Address 1200

Page = 1200 // 1024 = 1

Offset = 1200 % 1024 = 176

Frame = 2

Physical address = 2 × 1024 + 176 = 2224

### Address 3100

Page = 3100 // 1024 = 3

Offset = 3100 % 1024 = 28

Frame = 0

Physical address = 0 × 1024 + 28 = 28

### Address 5000

Page = 5000 // 1024 = 4

Offset = 5000 % 1024 = 904

Page 4 is not present in the page table. Therefore, address translation fails.

## Why Are Page Sizes Powers of Two?

Page sizes are powers of two because computers use binary numbers.

This makes it easier and faster to separate an address into its page number and offset using bit operations.

## Running

```bash
python3 translate.py
```

## Results

A custom address sequence is stored in refs.txt. The program results are saved in results.txt.

The first three addresses can be compared with the manual calculations.
