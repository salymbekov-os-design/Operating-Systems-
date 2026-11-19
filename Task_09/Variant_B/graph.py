import csv
import matplotlib.pyplot as plt

names = []
failures = []
external = []
internal = []

with open("comparison.csv") as file:
    for row in csv.DictReader(file):
        names.append(row["Strategy"])
        failures.append(int(row["Failures"]))
        external.append(float(row["External"]))
        internal.append(float(row["Internal"]))

fig, axes = plt.subplots(1, 3, figsize=(15, 5))

axes[0].bar(names, failures, color="steelblue")
axes[0].set_title("Allocation Failures")

axes[1].bar(names, external, color="orange")
axes[1].set_title("External Fragmentation (%)")

axes[2].bar(names, internal, color="green")
axes[2].set_title("Internal Fragmentation (%)")

plt.tight_layout()
plt.savefig("comparison.png")
