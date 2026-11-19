import csv
import matplotlib.pyplot as plt

sizes = []
rates = []

with open("comparison.csv") as file:
    for row in csv.DictReader(file):
        sizes.append(int(row["Size"]))
        rates.append(float(row["HitRate"]))

plt.figure(figsize=(8, 5))
plt.plot(sizes, rates, marker="o", color="blue")
plt.xlabel("TLB Size")
plt.ylabel("Hit Rate (%)")
plt.title("TLB Size vs Hit Rate")
plt.xticks(sizes)
plt.ylim(0, 105)
plt.grid(True)
plt.tight_layout()
plt.savefig("comparison.png")
