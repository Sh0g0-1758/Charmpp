#!/bin/bash

CHARM_PATH="./../../charm-v7.0.0/bin"
PROCESSORS=20
TOTAL_SIZE=200000
OUTPUT_FILE="results.txt"
RUNS=5

GRAIN_SIZES=(10 20 50 100 200 500 1000 2000 10000 20000 50000 200000)

echo "Compiling program..."
$CHARM_PATH/charmc primal.ci
$CHARM_PATH/charmc -o primal primal.cc

echo "total: $TOTAL_SIZE" > $OUTPUT_FILE
echo "Processors: $PROCESSORS" >> $OUTPUT_FILE
echo "RESULTS (GRAIN_SIZE, TIME):" >> $OUTPUT_FILE

extract_time() {
   grep "Time:" | awk '{print $2}'
}

for i in "${!GRAIN_SIZES[@]}"; do
   GRAIN_SIZE=${GRAIN_SIZES[$i]}
   echo "Testing grain size: $GRAIN_SIZE"
   
   total_time=0
   
   for ((run=1; run<=RUNS; run++)); do
       echo "  Run $run/$RUNS"
       OUTPUT=$($CHARM_PATH/charmrun +p$PROCESSORS ./primal $TOTAL_SIZE $GRAIN_SIZE 2>&1)
       TIME=$(echo "$OUTPUT" | extract_time)
       total_time=$(echo "$total_time + $TIME" | bc)
       echo "    Time: $TIME seconds"
   done
   
   average_time=$(echo "scale=6; $total_time / $RUNS" | bc)
   echo "$((i+1)). $GRAIN_SIZE, $average_time seconds" >> $OUTPUT_FILE
   echo "Completed: Grain size $GRAIN_SIZE - Average Time: $average_time seconds"
   
   sleep 1
done

echo "Final Results:"
cat $OUTPUT_FILE
