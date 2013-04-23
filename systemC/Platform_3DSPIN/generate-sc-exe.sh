#!/bin/bash
# Usage: ./generate-sc-exe.sh number_of_nodes number_in_x number_in_y
#
# The node are numeroted first in x and then in y.
# It jumps to the next y-line when a x-line is finished.

OUTPUT_FILE=simulation_2d_mesh_$1nodes_$2x_$3y

#variable
NBNODE=$(($1-1))
XMAX=$2
YMAX=$3
ZMAX=1

sed -i "9s/NUM_X .*/NUM_X ${XMAX}/g" system.cpp
sed -i "10s/NUM_Y .*/NUM_Y ${YMAX}/g" system.cpp
sed -i "1s/PROG=.*/PROG=${OUTPUT_FILE}/g" Makefile
make


