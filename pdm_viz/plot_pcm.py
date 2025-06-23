import matplotlib.pyplot as plt

# Load PCM values from the file
file_path = "pcm_output.txt"

timestamps = []
pcm_values = []

with open(file_path, "r") as f:
    for line in f:
        try:
            parts = line.strip().split(",")
            if len(parts) == 1:  # PCM only
                pcm_values.append(int(parts[0]))
            elif len(parts) == 2:  # Timestamp, PCM
                timestamps.append(int(parts[0]))
                pcm_values.append(int(parts[1]))
        except ValueError:
            continue  # Skip invalid lines

# If timestamps are present, normalize to start at 0
if timestamps:
    timestamps = [t - timestamps[0] for t in timestamps]
else:
    timestamps = list(range(len(pcm_values)))  # Use sample index if no timestamps

# Plot PCM signal
plt.figure(figsize=(12, 5))
plt.plot(timestamps, pcm_values, label="PCM Signal", linewidth=1)

plt.xlabel("Time (ms)" if timestamps else "Sample Index")
plt.ylabel("Amplitude")
plt.title("PCM Audio Signal")
plt.legend()
plt.grid()
# plt.show()
plt.savefig("pcm_plot.png")  
