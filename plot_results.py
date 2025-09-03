import pandas as pd
import matplotlib.pyplot as plt

# Read both CSV files
df1 = pd.read_csv("results-one_mutex.csv")   # One Mutex results
df2 = pd.read_csv("results-read-write_lock.csv")  # Read-Write Lock results

# Plot both on the same graph
plt.figure(figsize=(8, 6))

plt.plot(df1["Threads"], df1["Time"], marker="o", linestyle="-", color="b", label="One Mutex")
plt.plot(df2["Threads"], df2["Time"], marker="s", linestyle="--", color="r", label="Read-Write Lock")

# Labels and title
plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (seconds)")
plt.title("Case 3")
plt.grid(True)
plt.legend()

# Show the graph
plt.show()
