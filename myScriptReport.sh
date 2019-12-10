#!/bin/bash
clear
make clean
rm -rf report
make all
mkdir report
#validation 256KB, 512KB, 1MB, 2MB while keeping the cache associativity at 8 and block size at 64B
echo size variation
ASSOC=8
BLOCK=64
PROC=4 
INPUT=./trace/canneal.04t.longTrace
#validation  1
for TYPE in 0 1 2
do
	mkdir ./report/${TYPE}_size_variation
	for SIZE in 262144 524288 1048576 2097152‬
	do
		FILE="./report/${TYPE}_size_variation/output_${SIZE}_${ASSOC}_${BLOCK}_${PROC}_${TYPE}.txt"	
		./smp_cache $SIZE $ASSOC $BLOCK $PROC $TYPE $INPUT > $FILE
	done
done
#Cache associativity: vary from 4 way, 8 way, and 16 way while keeping the cache size at 1MB and block size at 64B
echo assoc variation
ASSOC=4
BLOCK=64
PROC=4
SIZE=1024 
INPUT=./trace/canneal.04t.longTrace
for TYPE in 0 1 2
do
	mkdir ./report/${TYPE}_assoc_variation
	for ASSOC in 4 8 16
	do
		FILE="./report/${TYPE}_assoc_variation/output_${SIZE}_${ASSOC}_${BLOCK}_${PROC}_${TYPE}.txt"	
		./smp_cache $SIZE $ASSOC $BLOCK $PROC $TYPE $INPUT > $FILE
	done
done

#Cache block size: vary from 64B, 128B, and 256B, while keeping the cache size at 1MB and cache associativity at 8 way
echo block variation
ASSOC=8
BLOCK=64
PROC=4
SIZE=1024 
INPUT=./trace/canneal.04t.longTrace
for TYPE in 0 1 2
do
	mkdir ./report/${TYPE}_block_variation
	for BLOCK in 64 128 256
	do
		FILE="./report/${TYPE}_block_variation/output_${SIZE}_${ASSOC}_${BLOCK}_${PROC}_${TYPE}.txt"	
		./smp_cache $SIZE $ASSOC $BLOCK $PROC $TYPE $INPUT > $FILE
	done
done

for subFolder in ./report/*
do
	echo subfolder = $subFolder
	cd $subFolder
	rm -rf data.txt
	touch data.txt
	for file in *
	do	
		echo $file >> data.txt
		echo -n " " >> data.txt
		awk '/01. number of reads:/{print $NF >> "./data.txt"}' $file
	done
	cd ../..
done
echo "Check data.txt file"




