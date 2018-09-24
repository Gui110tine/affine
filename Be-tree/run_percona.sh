#!/bin/bash
file="/mnt/db/affine_benchmark_data"
dev="/dev/sdc1"
output="betree.out"

keysize=1024
valuesize=$((keysize*4))

echo "Write Amp Test" | tee -a $output
echo "N_over_M, fanout, node_size_MB, seq_write_amp, rand_write_amp" | tee -a $output

for nodesize in 256 64 16 4 1; do
	for fanout in 16; do 
	
		echo -n "4, $fanout, $nodesize" | tee -a $output
		[[ -e $file ]] && rm $file

		./affine_bench_prep_data rand $fanout $nodesize $keysize $valuesize
		
		blktrace -a write -d $dev -o tracefile &
		cgexec -g memory:mem4 ./affine_bench_insert seq $fanout $nodesize $keysize $valuesize
		kill -15 $(pidof -s blktrace)
		blkparse -a issue -f "%n\\n" -i tracefile -o parsed_blocks.txt
		./interpret | tee -a $output

		[[ -e $file ]] && rm $file
		./affine_bench_prep_data rand $fanout $nodesize $keysize $valuesize

                blktrace -a write -d $dev -o tracefile &
                cgexec -g memory:mem4 ./affine_bench_insert rand $fanout $nodesize $keysize $valuesize
                kill -15 $(pidof -s blktrace)
                blkparse -a issue -f "%n\\n" -i tracefile -o parsed_blocks.txt
                ./interpret | tee -a $output

		echo "" | tee -a $output

	done
done
