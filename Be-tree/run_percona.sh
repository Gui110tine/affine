#!/bin/bash
file="/mnt/db/affine_benchmark_data"
dev="/dev/sdc1"
output="betree.out"

keysize=1024
valuesize=$((keysize*4))

echo "Write Amp Test" | tee -a $output
echo "N_over_M, epsilon, node_size, seq_write_amp, rand_write_amp" | tee -a $output

for nodesize in 128 64 32 16 8; do
	for epsilon in "0.5"; do 
	
		echo -n "4, $epsilon, $nodesize" | tee -a $output
		[[ -e $file ]] && rm $file

		./affine_bench_prep_data rand $epsilon $nodesize $keysize $valuesize
		./affine_bench_warm_cache rand $epsilon $nodesize $keysize $valuesize
		
		blktrace -a write -d $dev -o tracefile &
		cgexec -g memory:mem4 ./affine_bench_insert rand $epsilon $nodesize $keysize $valuesize
		kill -15 $(pidof -s blktrace)
		blkparse -a issue -f "%n\\n" -i tracefile -o parsed_blocks.txt
		./interpret | tee -a $output

                blktrace -a write -d $dev -o tracefile &
                cgexec -g memory:mem4 ./affine_bench_insert seq $epsilon $nodesize $keysize $valuesize
                kill -15 $(pidof -s blktrace)
                blkparse -a issue -f "%n\\n" -i tracefile -o parsed_blocks.txt
                ./interpret | tee -a $output

	done
done
