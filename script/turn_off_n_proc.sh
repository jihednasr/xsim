#!/bin/bash

for i in $(seq $2 -1 $1); do
/altamaha/home3/fournel/bin/start_stop_cpu $i off
sleep 5
done
