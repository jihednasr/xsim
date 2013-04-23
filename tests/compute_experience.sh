#!/bin/bash
# usage computate_speed_up.sh nb_proc_max nb_node_max
LAST_PROC=30

# Parameters
WINDOW=0
FREQUENCE_INFO=1
NB_NODE=2
NB_IFACE=3
TSIMULATED=4
NB_PROC=5

_3D=1

# There is a special case if the time info freq is 0 => move it to 1
NAME=( "time window" "time information period" "number of node" "number of interface" "simulated time" "number of processors" )
PARAMETERS=( 100 100 16 2 20000 32  )
#MIN_VAL=(     0      2  2  1    100          32  )
#MAX_VAL=( 10000  10000 32 10 100000 ${LAST_PROC} )
MIN_VAL=(   100      0  2  1    100          32  )
MAX_VAL=(  1000   1000 32  8 100000 ${LAST_PROC} )
INCR=( 100  50  1 1 1000 -1 )

TRIAL=200
XSIM_INTERMEDIATE_FILE=xsim_measure

# Choice of the variables
VAR1=$1
VAR2=$2
INCR1=${INCR[${VAR1}]}
INCR2=${INCR[${VAR2}]}
if [ ${VAR2} -eq ${NB_PROC} ]; then
    echo "ERROR: the second variable can not be the number of hote processors"
    exit 0
fi


SET_VALUE1=( $(seq ${MIN_VAL[${VAR1}]} ${INCR1} ${MAX_VAL[${VAR1}]}) )
SET_VALUE2=( $(seq ${MIN_VAL[${VAR2}]} ${INCR2} ${MAX_VAL[${VAR2}]}) )
# Change the increment of time window
#if [ ${VAR1} -eq ${WINDOW} ]; then
#    TMP_TABLE=( $(seq 0 10 90) )
#    SET_VALUE1=( ${TMP_TABLE[@]} ${SET_VALUE1[@]} )
#fi
#if [ ${VAR2} -eq ${WINDOW} ]; then
#    TMP_TABLE=( $(seq 0 10 90) )
#    SET_VALUE2=( ${TMP_TABLE[@]} ${SET_VALUE2[@]} )
#fi
# check if the time information frequency is 0
if [ ${VAR1} -eq ${FREQUENCE_INFO} ]; then
    SET_VALUE1[0]=1
fi
if [ ${VAR2} -eq ${FREQUENCE_INFO} ]; then
    SET_VALUE2[0]=1
fi
#echo ${SET_VALUE1[*]}
#echo ${SET_VALUE2[*]}
#exit 0



function read_processing_time 
{
    INPUT_FILE=$1
    MAX_TIME=0

    for file in $(ls ${INPUT_FILE}_node*.perf);
    do
        while read line; 
        do 
            if [ "$line" = "- global_processus_time:" ]; 
            then
                read line;
                read line;
                read line;
                IFS=: read debut fin;
                array=(${fin})
                if [ "${#array[*]}" -eq "2" ];
                then 
                    SEC=$(echo  "${array[0]}" | cut -d"s" -f1); 
                    NSEC=$(echo "${array[0]}" | cut -d"s" -f2); 
                    NSEC=$(echo "${NSEC}" | cut -d"n" -f1); 
                else
                    SEC=$(echo  "${array[0]}" | cut -d"s" -f1); 
                    NSEC=$(echo "${array[1]}" | cut -d"n" -f1); 
                fi

                RESULTAT=$((${SEC}*1000000000 + ${NSEC}))
                if [ "${RESULTAT}" -gt "${MAX_TIME}" ];
                then
                    MAX_TIME=${RESULTAT}
                fi
            fi
        done < ${file}
    done

    echo "${MAX_TIME}"
}

function set_time_window
{
    TW=$1
    sed -i "s/time_window .*/time_window               ${TW}/g" ../include/xsim_topology.h
}

function set_frequence_time_info
{
    FINFO=$1
    sed -i "36s/TIME_PERIOD .*/TIME_PERIOD          ${FINFO}/g" src/test_poisson.c
}


MUST_COMPILE=0

# compile in order to be sure to have the right version
$(set_time_window ${PARAMETERS[${WINDOW}]})
$(set_frequence_time_info ${PARAMETERS[${FREQUENCE_INFO}]})
cat ../include/xsim_topology.h | grep "time_window"
cat src/test_poisson.c | grep "TIME_PERIOD"
make xsim_lib > /dev/null 2>&1

for var1 in ${SET_VALUE1[*]};
do
    # set var1
    if [ ${VAR1} -eq ${WINDOW} ]; then
        $(set_time_window ${var1})
        MUST_COMPILE=1
    fi
    if [ ${VAR1} -eq ${FREQUENCE_INFO} ]; then
        $(set_frequence_time_info ${var1})
        MUST_COMPILE=1
    fi


    for var2 in ${SET_VALUE2[*]}
    do
        # Set variables
        PARAMETERS[${VAR1}]=${var1}
        PARAMETERS[${VAR2}]=${var2}

        if [ -f ../topology/2d_mesh_${PARAMETERS[${NB_NODE}]}nodes* ]; then
            if [ ${PARAMETERS[${WINDOW}]} -lt ${PARAMETERS[${FREQUENCE_INFO}]} ]; then
                if [ ${PARAMETERS[${FREQUENCE_INFO}]} -ne 1 ]; then
                    echo "SKIP: parameters[window]: ${PARAMETERS[${WINDOW}]} ; parameters[freq]: ${PARAMETERS[${FREQUENCE_INFO}]}"
                    continue
                fi
            fi
            # set var2
            if [ ${VAR2} -eq ${WINDOW} ]; then
                $(set_time_window ${var2})
                MUST_COMPILE=1
            fi
            if [ ${VAR2} -eq ${FREQUENCE_INFO} ]; then
                $(set_frequence_time_info ${var2})
                MUST_COMPILE=1
            fi
            # up-date the bin
            if [ ${MUST_COMPILE} -ne 0 ]; then
                cat ../include/xsim_topology.h | grep "time_window"
                cat src/test_poisson.c | grep "TIME_PERIOD"
                make xsim_lib > /dev/null 2>&1
                MUST_COMPILE=0
            fi

            for t in $(seq 1 ${TRIAL});
            do
                echo ""
                echo "!!!!!!!!! ${var1} ${NAME[${VAR1}]} ; ${var2} ${NAME[${VAR2}]} ; ${t}/${TRIAL} trial !!!!!!!!!"
                # compute XSim
                rm -f ${XSIM_INTERMEDIATE_FILE}_node*.perf
                ./test_poisson -time ${PARAMETERS[${TSIMULATED}]} -n ${PARAMETERS[${NB_NODE}]} -i ${PARAMETERS[${NB_IFACE}]} -tp ../topology/2d_mesh_${PARAMETERS[${NB_NODE}]}nodes* -mo ${XSIM_INTERMEDIATE_FILE} > /dev/null
                # read measure
                XSIM_RESULT=$(read_processing_time ${XSIM_INTERMEDIATE_FILE})
                echo "Xsim_result: ${XSIM_RESULT}"

            done
        fi

    done
done


rm -f res_node*
rm -f ${XSIM_INTERMEDIATE_FILE}_node*.perf
