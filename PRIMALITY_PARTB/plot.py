import matplotlib.pyplot as plt

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

plt.style.use('seaborn-darkgrid')
fig, ax = plt.subplots(figsize=(12, 7))

plt.plot(grain_sizes, times, 'b-o', linewidth=2, markersize=8, label='Execution Time')
ax.fill_between(grain_sizes, times, alpha=0.2)

plt.title(f'Execution Time vs Grain Size Analysis\n(Total Elements: {total:,}, Processors: {processors})', 
         fontsize=14, pad=20, fontweight='bold')
plt.xlabel('Grain Size (log scale)', fontsize=12, fontweight='bold')
plt.ylabel('Execution Time (seconds)', fontsize=12, fontweight='bold')

plt.grid(True, linestyle='--', alpha=0.7)
plt.xscale('log')

for i, (x, y) in enumerate(zip(grain_sizes, times)):
   label = f'{y:.3f}s'
   plt.annotate(label, (x, y), textcoords="offset points", xytext=(0,10), 
               ha='center', fontweight='bold', fontsize=9)

optimal_grain = grain_sizes[times.index(min(times))]
plt.axvline(x=optimal_grain, color='r', linestyle='--', alpha=0.5)
plt.annotate(f'Optimal Grain Size: {optimal_grain}', 
           (optimal_grain, max(times)), 
           textcoords="offset points", 
           xytext=(10, 10), 
           fontweight='bold',
           color='red')

plt.legend(frameon=True, fancybox=True, shadow=True)
plt.tight_layout()
plt.savefig('analysis.png', dpi=300, bbox_inches='tight', facecolor='white')
plt.close()
