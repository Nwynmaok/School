#!/bin/bash
#echo "1 Parallel Process:" > time.txt
> time.txt
/usr/bin/time -a -o time.txt -f "1;%E; %U; %S;" ./parallel_convert 1 ./output_dir ./input_dir >/dev/null
rm -r output_dir
/usr/bin/time -a -o time.txt -f "2;%E; %U; %S;" ./parallel_convert 2 ./output_dir ./input_dir >/dev/null
rm -r output_dir
/usr/bin/time -a -o time.txt -f "3;%E; %U; %S;" ./parallel_convert 3 ./output_dir ./input_dir >/dev/null
rm -r output_dir
/usr/bin/time -a -o time.txt -f "4;%E; %U; %S;" ./parallel_convert 4 ./output_dir ./input_dir >/dev/null
rm -r output_dir
/usr/bin/time -a -o time.txt -f "5;%E; %U; %S;" ./parallel_convert 5 ./output_dir ./input_dir >/dev/null
rm -r output_dir
/usr/bin/time -a -o time.txt -f "6;%E; %U; %S;" ./parallel_convert 6 ./output_dir ./input_dir >/dev/null
rm -r output_dir
/usr/bin/time -a -o time.txt -f "7;%E; %U; %S;" ./parallel_convert 7 ./output_dir ./input_dir >/dev/null
rm -r output_dir
/usr/bin/time -a -o time.txt -f "8;%E; %U; %S;" ./parallel_convert 8 ./output_dir ./input_dir >/dev/null
rm -r output_dir
/usr/bin/time -a -o time.txt -f "9;%E; %U; %S;" ./parallel_convert 9 ./output_dir ./input_dir >/dev/null
rm -r output_dir
/usr/bin/time -a -o time.txt -f "10;%E; %U; %S;" ./parallel_convert 10 ./output_dir ./input_dir >/dev/null
rm -r output_dir

(echo "Parallel Processes;Real;User;Sys" ; cat time.txt) | sed 's/;/\t/g' > time.csv
