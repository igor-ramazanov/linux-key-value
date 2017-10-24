#!/bin/bash
set -euo pipefail
trap "echo 'error: Script failed: see failed command above'" ERR

make

benchmark_multiple_processes() 
{
PROCESS_COUNT=$1
echo "==================== " $PROCESS_COUNT " processes ===================="
> benchmark.log
for i in `seq $PROCESS_COUNT`
do
	./test r >> benchmark.log &
done

wait
echo "100000 read operations: " `cat benchmark.log | python -c "import sys; print sum((float(l) for l in sys.stdin)) / $PROCESS_COUNT"`

> benchmark.log
for i in `seq $PROCESS_COUNT`
do
	./test w >> benchmark.log &
done

wait
echo "100000 write operations: " `cat benchmark.log | python -c "import sys; print sum((float(l) for l in sys.stdin)) / $PROCESS_COUNT"`

> benchmark.log
for i in `seq $PROCESS_COUNT`
do
	./test rw >> benchmark.log &
done

wait
echo "100000 read and 100000 write operations: " `cat benchmark.log | python -c "import sys; print sum((float(l) for l in sys.stdin)) / $PROCESS_COUNT"`
}

benchmark_multiple_processes 1
benchmark_multiple_processes 2
benchmark_multiple_processes 4
benchmark_multiple_processes 6
benchmark_multiple_processes 8
benchmark_multiple_processes 10