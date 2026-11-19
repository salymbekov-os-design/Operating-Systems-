import csv
import matplotlib.pyplot as plt

frames = []
fifo_values = []
lru_values = []
optimal_values = []
clock_values = []

with open("comparison.csv") as file:
    for row in csv.DictReader(file):
        frames.append(int(row["Frames"]))
        fifo_values.append(int(row["FIFO"]))
        lru_values.append(int(row["LRU"]))
        optimal_values.append(int(row["Optimal"]))
        clock_values.append(int(row["Clock"]))

plt.figure(figsize=(10, 6))

plt.plot(frames, fifo_values, marker="o", label="FIFO")
plt.plot(frames, lru_values, marker="o", label="LRU")
plt.plot(frames, optimal_values, marker="o", label="Optimal")
plt.plot(frames, clock_values, marker="o", label="Clock")

plt.xlabel("Number of Frames")
plt.ylabel("Page Faults")
plt.title("Page Replacement Algorithms")
plt.xticks(frames)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("comparison.png")
