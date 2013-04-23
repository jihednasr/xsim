#!/bin/bash

MAX=20000

echo "" > log ;
for i in $(seq 1 ${MAX});
do
    echo "essai ${i}/${MAX}" >> log;
    ./simulation_systemC_3DSPIN_model6 -time 10000 -n 32 -i 4 > /dev/null 2>> log;
done
