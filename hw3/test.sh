#!/bin/bash
START=1
END=10

for i in $(eval echo "{$START..$END}")
do
    echo "Inferencing $i.txt"
    ./submit_files_template/mydisambig -text testdata/$i.txt -map ZhuYin-Big5.map -lm bigram.lm -order 2 > ./submit_files_template/result1/$i.txt 
	echo "Finished"
done



