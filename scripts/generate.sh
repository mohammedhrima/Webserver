#!/bin/bash
file="006.txt"
# Loop from 1 to 100
for i in {1..500000}; do
    num=$(printf "%04d" $i)
    # output="$num:"
    buff=""
    for char in {a..z}; do
        buff="$buff$char"
    done
    # output="$num:not-found not-found not-found not-found not-found not-found"
    echo "$file:$num:$buff-$buff-$buff"
done
