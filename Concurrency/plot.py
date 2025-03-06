import pandas as pd
import matplotlib.pyplot as plt

# Load the data
data = pd.read_csv("results.csv")

# Plot speedup vs. thread count for each block size
plt.figure(figsize=(10, 6))
for block_size in data['BlockSize'].unique():
    subset = data[data['BlockSize'] == block_size]
    plt.plot(subset['Threads'], subset['Time'].iloc[0] / subset['Time'], label=f"Block Size {block_size}")

plt.xlabel('Number of Threads')
plt.ylabel('Speedup')
plt.title('Speedup vs Number of Threads for Different Block Sizes')
plt.legend()
plt.show()

# Plot time vs block size for different thread counts
plt.figure(figsize=(10, 6))
for threads in data['Threads'].unique():
    subset = data[data['Threads'] == threads]
    plt.plot(subset['BlockSize'], subset['Time'], label=f"{threads} Threads")

plt.xlabel('Block Size')
plt.ylabel('Time (s)')
plt.title('Time vs Block Size for Different Thread Counts')
plt.legend()
plt.show()
