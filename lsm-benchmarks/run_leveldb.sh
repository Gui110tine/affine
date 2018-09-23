#!/bin/bash

set -x

DB="/mnt/db/leveldb"
exe=benchmark
log="leveldb`date +%Y-%m-%d.%H:%M:%S`.csv"
keysize=1024
valuesize=$((keysize*4))

if [ ! -f "$exe" ]; then
	echo "Need to make benchmark"
	exit 1
fi

touch "$log"
echo "Write Amp test" | tee -a "$log"
echo "N_over_M, node_size, write_amp_seq, write_amp_rand" | tee -a "$log"

for x in mem4 mem2 mem1 mem0.5; do 
	for i in 268435456 67108864 16777216 4194304 1048576; do
    		if [ -d "$DB" ]; then
	    		rm -rf "$DB"/*
		fi
		echo -n "$x, $i" | tee -a "$log"
		cgexec -g memory:$x ./$exe -l -s $i -k $keysize -v $valuesize | tee -a "$log"
		# rm -rf "$DB"/*
		# ./$exe -r -s $i -k $keysize -v $valuesize | tee -a "$log"
		echo | tee -a "$log"
	done
done

