#!/bin/bash

# Output file for results
OUTPUT_FILE="fairness_results.csv"

# Clear/create output file with CSV header
echo "threads,seconds,impl,run,total_acquisitions,jain_fairness" > $OUTPUT_FILE

# Array of thread counts
threads=(1 2 3 4 8 16 32 64)

# Array of seconds
seconds=(3 7 10)

# Array of implementations
impls=("posix" "ticket")

# Number of runs per configuration
RUNS=3

# Run benchmarks
for t in "${threads[@]}"; do
    for sec in "${seconds[@]}"; do
        for impl in "${impls[@]}"; do
            for run in $(seq 1 $RUNS); do
                echo "Running: threads=$t secs=$sec impl=$impl (run $run/$RUNS)"
                
                # Run benchmark and capture output
                output=$(./ex2-fairness -t $t -secs $sec -impl $impl)
                
                # Parse the output: impl=posix threads=50 seconds=5 total=12345
                # and jain=0.9876
                total=$(echo "$output" | grep -oP 'total=\K[0-9]+')
                jain=$(echo "$output" | grep -oP 'jain=\K[0-9.]+')
                
                # Write to CSV
                echo "$t,$sec,$impl,$run,$total,$jain" >> $OUTPUT_FILE
            done
        done
    done
done

echo "Benchmark complete! Results saved to $OUTPUT_FILE"
