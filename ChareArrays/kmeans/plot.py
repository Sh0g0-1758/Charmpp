import matplotlib.pyplot as plt
import argparse

# check user input for part
argparser = argparse.ArgumentParser()

argparser.add_argument('--part', type=str, default='a', help='Part number')

part = argparser.parse_args().part

losses = []
## save runs in results.txt  ./charmrun ./kmeans ... > results.txt
with open(f"part{part}/results.txt", 'r') as f:
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
plt.savefig(f"part{part}/loss_curve.png")
plt.show()