#!/bin/bash

set -x

DB="/mnt/db/leveldb"
exe=benchmark
log="rocksdb_write_amp`date +%Y-%m-%d.%H:%M:%S`.csv"
keysize=1024
valuesize=$((keysize*4))

if [ ! -f "$exe" ]; then
	echo "Need to make benchmark"
	exit 1
fi

touch "$log"
echo "Write Amp test" | tee -a "$log"
echo "N_over_M, fanout, node_size, seq_write_amp, rand_write_amp" | tee -a "$log"

for j in 4 8 16; do
    for i in 268435456 134217728 67108864 33554432 16777216 8388608 4194304 2097152 1048576 524288; do
        if [ -d "$DB" ]; then
	    	    rm -rf "$DB"/*
	    fi

	    #./$exe -l -b -s $i -k $keysize -v $valuesize | tee -a "$log"
	    # rm -rf "$DB"/*
	     ./$exe -r -f $j -s $i -k $keysize -v $valuesize | tee -a "$log"
	    echo | tee -a "$log"
    done
done

