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
PARAMETERS=( 100 100 32 1 10000 32  )
#MIN_VAL=(     0      2  2  1    100          32  )
#MAX_VAL=( 10000  10000 32 10 100000 ${LAST_PROC} )
MIN_VAL=(    0     0  2  1    100          32  )
MAX_VAL=(  100   100 32  8 100000 ${LAST_PROC} )
INCR=( 10  10  1 1 1000 -1 )

TRIAL=200
XSIM_INTERMEDIATE_FILE=xsim_measure
SYSC_INTERMEDIATE_FILE=Platform_3DSPIN/systemC_measure
#OUTPUT_FILE must be removed one day or another
OUTPUT_FILE=speedup
GNUPLOT_CMD=gnuplot_command.plot
OUTPUT_DATA_FILE=
if [ ${_3D} -ne 0 ]; then
    OUTPUT_DATA_FILE=measures.gp
fi

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

function output_gnuplot_command_file
{
    echo "# Gnuplot script. Launch with \"gnuplot file.plot\"" > ${GNUPLOT_CMD}
    echo "# Uncomment to generate png file" >> ${GNUPLOT_CMD}
    echo "# set terminal png" >> ${GNUPLOT_CMD}
    echo "# set output 'speed_up.png'" >> ${GNUPLOT_CMD}
    echo "" >> ${GNUPLOT_CMD}

    echo "# Uncomment to generate pdf file" >> ${GNUPLOT_CMD}
    echo "# set terminal postscript enhanced color" >> ${GNUPLOT_CMD}
    echo "# set output '| ps2pdf - speed_up.pdf'" >> ${GNUPLOT_CMD}
    echo "" >> ${GNUPLOT_CMD}

    echo "#set title \"speed-up\"" >> ${GNUPLOT_CMD}

    echo "set style data linespoints" >> ${GNUPLOT_CMD}
    echo "set pointsize 1" >> ${GNUPLOT_CMD}
    echo "set mouse" >> ${GNUPLOT_CMD}
    echo "set xlabel \"${NAME[${VAR1}]}\"" >> ${GNUPLOT_CMD}
    echo "set xtics ${INCR[${VAR1}]}" >> ${GNUPLOT_CMD}
    echo "set ytics ${INCR[${VAR2}]}" >> ${GNUPLOT_CMD}

    if [ ${_3D} -eq 0 ]; then
        echo "set ylabel \"speed-up\"" >> ${GNUPLOT_CMD}
        echo "" >> ${GNUPLOT_CMD}
        echo "" >> ${GNUPLOT_CMD}

        # plot 2D
        ind=0
        ech=0
        for n in $(seq 2 ${PARAMETERS[${NB_NODE}]});
        do
            if [ -f ../topology/2d_mesh_${n}nodes*.topo ]; then
                ind=$((${ind}+1))
                echo "# nb_proc  execution_time  standard_deviation" > ${OUTPUT_FILE}_${n}nodes.gp
                if [ "${ech}" -eq "0" ]; then
                    echo "plot \"${OUTPUT_FILE}_${n}nodes.gp\" using 1:2 title \"number of nodes: ${n}\" with linespoints linetype ${ind} linewidth 3, \\" >> ${GNUPLOT_CMD}
                    echo "\"${OUTPUT_FILE}_${n}nodes.gp\" using 1:2:3 with errorbars ls 1 notitle, \\" >> ${GNUPLOT_CMD}
                    ech=1
                else
                    echo "\"${OUTPUT_FILE}_${n}nodes.gp\" using 1:2 title \"number of nodes: ${n}\" with linespoints linetype ${ind} linewidth 3, \\" >> ${GNUPLOT_CMD}
                    echo "\"${OUTPUT_FILE}_${n}nodes.gp\" using 1:2:3 with errorbars ls 1 notitle, \\" >> ${GNUPLOT_CMD}
                fi
            fi
        done
    else
        echo "set ylabel \"${NAME[${VAR2}]}\"" >> ${GNUPLOT_CMD}
        echo "set zlabel \"speed-up\"" >> ${GNUPLOT_CMD}
        echo "" >> ${GNUPLOT_CMD}
        echo "set grid" >> ${GNUPLOT_CMD}
        echo "set grid ztics" >> ${GNUPLOT_CMD}
        echo "set ztics out" >> ${GNUPLOT_CMD}
        echo "set ticslevel 0" >> ${GNUPLOT_CMD}
        echo "set border 1+2+4+8+16+32+64+256+512" >> ${GNUPLOT_CMD}
        echo "set view 60,30" >> ${GNUPLOT_CMD}
        echo "set pm3d" >> ${GNUPLOT_CMD}
        echo "" >> ${GNUPLOT_CMD}
        echo "" >> ${GNUPLOT_CMD}

        #initialize the data file
        echo "# Data file" > ${OUTPUT_DATA_FILE}
        for i in $(seq 0 5);
        do
            echo "# ${NAME[${i}]}: ${PARAMETERS[${i}]}"  >> ${OUTPUT_DATA_FILE}
        done
        echo "# ${NAME[${VAR1}]} (column 1) ; ${NAME[${VAR2}]} (column 2) ; speed-up (column 3) ; standard deviation (column 4)" >> ${OUTPUT_DATA_FILE}

        # plot 3D
        echo "splot \"${OUTPUT_DATA_FILE}\" using 1:2:3 notitle" >> ${GNUPLOT_CMD}
    fi

    echo "" >> ${GNUPLOT_CMD}
    echo "pause -1" >> ${GNUPLOT_CMD}
}


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

function read_systemc_time 
{
    file=$1
    MAX_TIME=0

    res=""
    IFS=" "
    while read first second third last;
    do 
        res=${last}
    done < ${file}

    echo "${res}"
}

function abs {
    echo "${1}" | sed "s/-//"
}

function set_time_window
{
    TW=$1
    sed -i "s/time_window .*/time_window               ${TW}/g" ../include/xsim_topology.h
}

function set_frequence_time_info
{
    FINFO=$1
    sed -i "s/TIME_INFO_PERIOD .*/TIME_INFO_PERIOD          ${FINFO}/g" include/xsim_sc_3dspin_config.h
}

output_gnuplot_command_file

LINETOJUMP=0
MUST_COMPILE=0

# compile in order to be sure to have the right version
$(set_time_window ${PARAMETERS[${WINDOW}]})
$(set_frequence_time_info ${PARAMETERS[${FREQUENCE_INFO}]})
cat ../include/xsim_topology.h | grep "time_window"
cat include/xsim_sc_3dspin_config.h | grep "FREQ_TIME_INFO"
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

        # local variable
        SPEEDUP=()

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
                cat include/xsim_sc_3dspin_config.h | grep "TIME_INFO_PERIOD"
                make xsim_lib > /dev/null 2>&1
                MUST_COMPILE=0
            fi

            LINETOJUMP=1
            for t in $(seq 1 ${TRIAL});
            do
                echo ""
                echo "!!!!!!!!! ${var1} ${NAME[${VAR1}]} ; ${var2} ${NAME[${VAR2}]} ; ${t}/${TRIAL} trial !!!!!!!!!"
                # compute XSim
                rm -f ${XSIM_INTERMEDIATE_FILE}_node*.perf
                ./simulation_systemC_3DSPIN_model6 -time ${PARAMETERS[${TSIMULATED}]} -n ${PARAMETERS[${NB_NODE}]} -i ${PARAMETERS[${NB_IFACE}]} -tp ../topology/2d_mesh_${PARAMETERS[${NB_NODE}]}nodes* -mo ${XSIM_INTERMEDIATE_FILE} > /dev/null
                # read measure
                XSIM_RESULT=$(read_processing_time ${XSIM_INTERMEDIATE_FILE})
                echo "Xsim_result: ${XSIM_RESULT}"

                # compute SystemC
                ./Platform_3DSPIN/simulation_2d_mesh_${PARAMETERS[${NB_NODE}]}nodes* ${PARAMETERS[${TSIMULATED}]} ${PARAMETERS[${NB_PROC}]} ${SYSC_INTERMEDIATE_FILE} > /dev/null
                # read measure
                SYSC_RESULT=$(read_systemc_time ${SYSC_INTERMEDIATE_FILE})
                echo "SysC_result: ${SYSC_RESULT}"

                # compute the Speed-up
                SPEEDUP[${t}]=$(echo "${SYSC_RESULT} / ${XSIM_RESULT}" | bc -l)
            done
            # compute the mean and the standard variation
            MEAN=0
            VALUE_SQUARE=0
            for i in ${SPEEDUP[*]};
            do
                MEAN=$(echo "${MEAN} + ${i}" | bc -l)
                VALUE_SQUARE=$(echo "${VALUE_SQUARE} + ${i}*${i}" | bc -l)
            done
            MEAN=$(echo "${MEAN} / ${#SPEEDUP[*]}" | bc -l)
            STANDARD_VARIATION=$(echo "${VALUE_SQUARE} / (${#SPEEDUP[*]}-1) - (${#SPEEDUP[*]}/(${#SPEEDUP[*]}-1))*${MEAN}*${MEAN}" | bc -l)
            STANDARD_VARIATION=$(abs ${STANDARD_VARIATION})
            STANDARD_VARIATION=$(echo "sqrt(${STANDARD_VARIATION})" | bc -l)

            # Output them
            # TODO: adapte output
            if [ ${_3D} -eq 0 ]; then
                echo "${var1}   ${var2}   ${MEAN}     ${STANDARD_VARIATION}" >> ${OUTPUT_FILE}_${PARAMETERS[${NB_NODE}]}nodes.gp
            else
                echo "${var1}   ${var2}   ${MEAN}     ${STANDARD_VARIATION}" >> ${OUTPUT_DATA_FILE}
            fi
        fi

    done
    # Print one empty line in order to draw some grid
    if [ ${LINETOJUMP} -ne 0 ]; then
        echo "" >> ${OUTPUT_DATA_FILE}
        LINETOJUMP=0
    fi

    # stop one proc if needed
    if [ ${VAR1} -eq ${NB_PROC} ]; then
        if [ "${var1}" -ne "${LAST_PROC}" ];
        then
            ../script/turn_off_n_proc.sh ${var1} ${var1}
            sleep 4
        fi
    fi
done

# wake-up all the proc if needed
if [ ${VAR1} -eq ${NB_PROC} ]; then
    ../script/turn_on_n_proc.sh ${LAST_PROC} ${MAX_VAL[${NB_PROC}]}
fi

rm -f res_node*
rm -f ${XSIM_INTERMEDIATE_FILE}_node*.perf
