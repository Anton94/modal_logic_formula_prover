#!/bin/bash

# Example usage:
# ./app_runner.sh '<path_to_the_tests_executable>/tests.exe' "satisfiable evaluation with constant 7" -d yes

time_to_run_ms=$((10000))
output_file_name="app_runner_last_run_output.txt"

echo "Will re-run it continuously and the output from the last run will be in '$output_file_name'"

start_time_ms=$(($(date +%s%N) / 1000000))
number_of_runs=$((0))
time_passed_ms=$((0))

while [ $((time_passed_ms)) -lt $((time_to_run_ms)) ]
do
	"$1" "${@:2}" > $output_file_name
	number_of_runs=$((number_of_runs + 1))
	time_passed_ms=$(($(date +%s%N) / 1000000 - $start_time_ms))
	echo -en "\rRunning... $(($time_passed_ms / ($time_to_run_ms / 100)))%"
done

echo -en "\r               \n"

echo "Ran it $number_of_runs times in $((time_passed_ms))ms"
echo "Average time per run: $(($time_passed_ms / $number_of_runs))ms"
