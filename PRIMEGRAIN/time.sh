#!/bin/bash
grainSizes=(1 2 4 8 16)

for g in "${grainSizes[@]}"; do
echo "Running with grain size: $g"
  for i in {1..3}; do
    ./charmrun ./prime 10 "$g" ++local +p8 ++quiet
  done
done