#!/bin/bash

for i in $(seq $1 $2); do
/altamaha/home3/fournel/bin/start_stop_cpu $i on
sleep 3
done
