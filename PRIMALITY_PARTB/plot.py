import matplotlib.pyplot as plt
import numpy as np

def parse_results_file(filename):
   grain_sizes = []
   times = []
   total = None
   processors = None
   
   with open(filename, 'r') as f:
       lines = f.readlines()
       
       total = int(lines[0].split(': ')[1])
       processors = int(lines[1].split(': ')[1])
       
       for line in lines[3:]:
           if line.strip():
               parts = line.split(', ')
               grain_size = int(parts[0].split('. ')[1])
               time = float(parts[1].split(' ')[0])
               grain_sizes.append(grain_size)
               times.append(time)
   
   return total, processors, grain_sizes, times

total, processors, grain_sizes, times = parse_results_file('results.txt')
five_percent_line = max(times) * 0.05

plt.figure(figsize=(12, 7))
plt.plot(grain_sizes, times, 'b-o', linewidth=2, markersize=8, label='Execution Time')
plt.axhline(y=five_percent_line, color='r', linestyle='--', alpha=0.5, 
           label=f'5% of max time ({five_percent_line:.2f}s)')

plt.title(f'Execution Time vs Grain Size\n(Total: {total}, Processors: {processors})', fontsize=12, pad=15)
plt.xlabel('Grain Size (log scale)', fontsize=10)
plt.ylabel('Time (seconds)', fontsize=10)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xscale('log')

for i, (x, y) in enumerate(zip(grain_sizes, times)):
   label = f'{y:.2f}s'
   plt.annotate(label, 
               (x, y), 
               textcoords="offset points", 
               xytext=(0,10), 
               ha='center')

plt.legend()
plt.tight_layout()
plt.savefig('analysis.png', dpi=300, bbox_inches='tight')
plt.close()
