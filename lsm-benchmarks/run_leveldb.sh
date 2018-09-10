#!/bin/bash

set -x

DB="/mnt/db/leveldb"
exe=benchmark
log="output`date +%Y-%m-%d.%H:%M:%S`.csv"
keysize=1024
valuesize=$((keysize*4))

if [ ! -f "$exe" ]; then
	echo "Need to make benchmark"
	exit 1
fi

touch "$log"
echo "Write Amp test" | tee -a "$log"
echo ",,LevelDB-Sequential-Insert,,,LevelDB-Random-Insert,,,RocksDB-Sequential-Insert,,,RocksDB-Random-Insert" | tee -a "$log"
echo "write_cache_size, node_size, insertions, MiB_written, blocks_written_to_disk, written_to_disk_over_written, blocks_written_to_disk, written_to_disk_over_written" | tee -a "$log"

for j in 2097152 4194304 8388608 16777216 33554432 67108864; do
    for i in 268435456 134217728 67108864 33554432 16777216 8388608 4194304 2097152 1048576 524288; do
        if [ -d "$DB" ]; then
	    	    rm -rf "$DB"/*
	    fi

	    ./$exe -l -b -s $i -c $j -k $keysize -v $valuesize | tee -a "$log"
	    # rm -rf "$DB"/*
	    # ./$exe -r -s $i -k $keysize -v $valuesize | tee -a "$log"
	    echo | tee -a "$log"
    done
done

