#!/bin/bash

outputSize=2000000
grainSizes=(10 20 50 100 200 500 1000 2000 5000 $outputSize)
outputFile="averages.txt"
> "$outputFile"

for g in "${grainSizes[@]}"; do
  sum=0
  echo "Grain size: $g"
  for i in {1..5}; do
    output=$(./charmrun ./prime $outputSize "$g" ++local ++quiet +p7)
    timeValue=$(echo "$output" | grep "time" | awk '{print $2}')
    sum=$(echo "$sum + $timeValue" | bc -l)
  done
  average=$(echo "$sum / 3" | bc -l)
  if [ "$g" -eq -1 ]; then
    g=50
  fi
  echo "$g $average" >> "$outputFile"
done