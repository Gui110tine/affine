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
    for i in 268435456 67108864 16777216 4194304 1048576; do
        if [ -d "$DB" ]; then
	    	    rm -rf "$DB"/*
	    fi

	    echo -n "mem1, $j, $i" | tee -a "$log"
	    #./$exe -l -b -s $i -k $keysize -v $valuesize | tee -a "$log"
	    # rm -rf "$DB"/*
	    cgexec -g memory:mem1 ./$exe -r -f $j -s $i -k $keysize -v $valuesize | tee -a "$log"
	    echo | tee -a "$log"
    done
done

