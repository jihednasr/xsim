#!/bin/bash
# Usage: ./generate-sc-exe.sh number_of_nodes number_in_x number_in_y
#
# The node are numeroted first in x and then in y.
# It jumps to the next y-line when a x-line is finished.

OUTPUT_FILE=16p16n_work_variation_CA_REF_file
LIMIT=0
INCR=1000
LOOP=40

echo "# The load of the TG will grows of ${INCR} loop iteration for each new measure." > ${OUTPUT_FILE}
echo "# The first measure is done with LIMIT=${LIMIT}." >> ${OUTPUT_FILE}
echo "# Loop_iteration nb_node nb_iface time" >> ${OUTPUT_FILE}

for i in $(seq 0 ${LOOP});
do
    sed -i "1s/LIMIT_LOOP .*/LIMIT_LOOP ${LIMIT}/g" ./Models/SystemC/TG/soclib_TG.h
    ./generate-sc-exe.sh 16 4 4
    ./simulation_2d_mesh_16nodes_4x_4y 4000 ${LIMIT} ${OUTPUT_FILE}
    LIMIT=$((${LIMIT}+${INCR}))
done


