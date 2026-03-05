#!/bin/bash

# Output file for results
OUTPUT_FILE="bench_results.csv"

# Clear/create output file with CSV header
echo "threads,iterations,lock,run,time_sec,ops_per_sec" > $OUTPUT_FILE

# Array of thread counts
threads=(1 2 3 4 8 16 32 64 )

# Array of iteration counts
iterations=(1000000 5000000 10000000)

# Array of lock types
locks=("sem" "mutex" "spin")

# Number of runs per configuration
RUNS=3

# Run benchmarks
for t in "${threads[@]}"; do
    for iter in "${iterations[@]}"; do
        for lock in "${locks[@]}"; do
            for run in $(seq 1 $RUNS); do
                echo "Running: threads=$t iters=$iter lock=$lock (run $run/$RUNS)"
                
                # Run benchmark and capture output
                output=$(./ex1-bench -t $t -iters $iter -lock $lock)
                
                # Parse the output: lock=sem threads=1 iters=1000000 time=0.123 s ops/sec=8130081
                time_sec=$(echo "$output" | grep -oP 'time=\K[0-9.]+')
                ops_sec=$(echo "$output" | grep -oP 'ops/sec=\K[0-9]+')
                
                # Write to CSV
                echo "$t,$iter,$lock,$run,$time_sec,$ops_sec" >> $OUTPUT_FILE
            done
        done
    done
done

echo "Benchmark complete! Results saved to $OUTPUT_FILE"
