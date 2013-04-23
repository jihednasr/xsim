#!/bin/bash
# Usage: ./generate-sc-exe.sh number_of_nodes number_in_x number_in_y
#
# The node are numeroted first in x and then in y.
# It jumps to the next y-line when a x-line is finished.

INTERMEDIATE_FILE=work_var
LIMIT=0
INCR=500
LOOP=40
NB_NODE=16
NB_IFACE=1
TRIAL=30
OUTPUT_FILE=32p${NB_NODE}n_work_variation_10TW_file.gp

echo "# The load of the TG will grows of ${INCR} loop iteration for each new measure." > ${OUTPUT_FILE}
echo "# The first measure is done with LIMIT=${LIMIT}." >> ${OUTPUT_FILE}
echo "# nb_node: ${NB_NODE} ; nb_iface: ${NB_IFACE}." >> ${OUTPUT_FILE}
echo "# Loop_iteration time" >> ${OUTPUT_FILE}


function abs {
    echo "${1}" | sed "s/-//"
}


for l in $(seq 0 ${LOOP});
do
    echo "Number of loop added: ${l}"
    sed -i "1s/LIMIT_LOOP .*/LIMIT_LOOP ${LIMIT}/g" ./include/soclib_TG.h
    make 2> /dev/null

    #variable
    TIME=()
    for t in $(seq 1 ${TRIAL}); 
    do
        ./simulation_systemC_3DSPIN_model6 -m 6 -n ${NB_NODE} -i ${NB_IFACE} -tl 4000 -tp ../topology/2d_mesh_${NB_NODE}nod*.topo -om ${INTERMEDIATE_FILE} > /dev/null
        TIME[${t}]=$(./parse_output_file.sh ${INTERMEDIATE_FILE})
        echo "TIME[]= ${TIME[${t}]}"
    done
    # compute the mean and the standard variation
    MEAN=0
    VALUE_SQUARE=0
    for i in ${TIME[*]};
    do
        MEAN=$(echo "${MEAN} + ${i}" | bc -l)
        VALUE_SQUARE=$(echo "${VALUE_SQUARE} + ${i}*${i}" | bc -l)
    done
    MEAN=$(echo "${MEAN} / ${#TIME[*]}" | bc -l)
    STANDARD_VARIATION=$(echo "${VALUE_SQUARE} / (${#TIME[*]}-1) - (${#TIME[*]}/(${#TIME[*]}-1))*${MEAN}*${MEAN}" | bc -l)
    STANDARD_VARIATION=$(abs ${STANDARD_VARIATION})
    STANDARD_VARIATION=$(echo "sqrt(${STANDARD_VARIATION})" | bc -l)

    # Output them
    echo "${LIMIT}      ${MEAN}     ${STANDARD_VARIATION}" >> ${OUTPUT_FILE}

    LIMIT=$((${LIMIT}+${INCR}))
done

