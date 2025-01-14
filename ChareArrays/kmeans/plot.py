import matplotlib.pyplot as plt

# Read values from results.txt and store as floats
losses = []
with open('results.txt', 'r') as f:
    for line in f:
        line = line.strip()
        if line:
            # Convert line to float and append
            try:
                losses.append(float(line))
            except:
                continue

# Plot the loss curve
plt.plot(range(len(losses)), losses, marker='o')
plt.title('Loss Curve')
plt.xlabel('Iteration')
plt.ylabel('Loss')
plt.grid(True)
plt.savefig('loss_curve.png')
plt.show()