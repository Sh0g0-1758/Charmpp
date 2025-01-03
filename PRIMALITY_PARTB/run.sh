#!/bin/bash

CHARM_PATH="./../../charm-v7.0.0/bin"
PROCESSORS=20
TOTAL_SIZE=200000
OUTPUT_FILE="results.txt"

GRAIN_SIZES=(10 20 50 100 200 500 1000 2000 5000)

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

    OUTPUT=$($CHARM_PATH/charmrun +p$PROCESSORS ./primal $TOTAL_SIZE $GRAIN_SIZE 2>&1)
    TIME=$(echo "$OUTPUT" | extract_time)

    echo "$((i+1)). $GRAIN_SIZE, $TIME seconds" >> $OUTPUT_FILE
    echo "Completed: Grain size $GRAIN_SIZE - Time: $TIME seconds"
done

cat $OUTPUT_FILE
