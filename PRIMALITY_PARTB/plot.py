import matplotlib.pyplot as plt
import numpy as np

# Data
grain_sizes = [10, 20, 50, 100, 200, 500, 1000, 2000, 20000]
times = [4.979128, 4.842032, 4.863737, 3.108976, 4.376151, 6.093785, 6.240219, 9.078733, 54.887470]

# Calculate 5% of the maximum time
five_percent_line = max(times) * 0.05

# Create the figure and axis
plt.figure(figsize=(12, 7))

# Plot the line graph
plt.plot(grain_sizes[:-1], times[:-1], 'b-o', linewidth=2, markersize=8, label='Execution Time')

# Plot the infinite grain size point separately
plt.plot(grain_sizes[-1], times[-1], 'ro', markersize=10, label='Infinite Grain Size')

# Add horizontal line at 5% of max time
plt.axhline(y=five_percent_line, color='r', linestyle='--', alpha=0.5, 
            label=f'5% of max time ({five_percent_line:.2f}s)')

# Customize the plot
plt.title('Execution Time vs Grain Size\n(Total: 20000, Processors: 20)', fontsize=12, pad=15)
plt.xlabel('Grain Size (log scale)', fontsize=10)
plt.ylabel('Time (seconds)', fontsize=10)

# Add grid for better readability
plt.grid(True, linestyle='--', alpha=0.7)

# Use logarithmic scale for x-axis
plt.xscale('log')

# Add data points labels
for i, (x, y) in enumerate(zip(grain_sizes, times)):
    label = f'{y:.2f}s'
    if i == len(grain_sizes) - 1:  # For the last point
        plt.annotate('∞ grain size\n' + label, 
                    (x, y), 
                    textcoords="offset points", 
                    xytext=(0,15), 
                    ha='center',
                    color='red')
    else:
        plt.annotate(label, 
                    (x, y), 
                    textcoords="offset points", 
                    xytext=(0,10), 
                    ha='center')

# Add legend
plt.legend()

# Adjust layout to prevent label clipping
plt.tight_layout()

# Save the plot with high resolution
plt.savefig('performance_analysis_with_infinite.png', dpi=300, bbox_inches='tight')

# Close the plot to free memory
plt.close()
