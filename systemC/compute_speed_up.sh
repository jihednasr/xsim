#!/bin/bash
# usage computate_speed_up.sh nb_proc_max nb_node_max


GNUPLOT_CMD=gnuplot_speed_up.plot
OUTPUT_FILE=speedup
XSIM_INTERMEDIATE_FILE=xsim_measure
SYSC_INTERMEDIATE_FILE=Platform_3DSPIN/systemC_measure
NB_PROC=$(($1-1))
NB_NODE=$2
NB_IFACE=1
LAST_PROC=0
TL=1000
TRIAL=30

echo "# Gnuplot script. Launch with \"gnuplot file.plot\"" > ${GNUPLOT_CMD}
echo "# Uncomment to generate png file" >> ${GNUPLOT_CMD}
echo "# set terminal png" >> ${GNUPLOT_CMD}
echo "# set output 'speed_up.png'" >> ${GNUPLOT_CMD}
echo "" >> ${GNUPLOT_CMD}
echo "# Uncomment to generate pdf file" >> ${GNUPLOT_CMD}
echo "# set terminal postscript enhanced color" >> ${GNUPLOT_CMD}
echo "# set output '| ps2pdf - speed_up.pdf'" >> ${GNUPLOT_CMD}
echo "" >> ${GNUPLOT_CMD}
echo "set title \"speed-up\"" >> ${GNUPLOT_CMD}
echo "set style data linespoints" >> ${GNUPLOT_CMD}
echo "set pointsize 1" >> ${GNUPLOT_CMD}
echo "set mouse" >> ${GNUPLOT_CMD}
echo "set xlabel \"number of processors\"" >> ${GNUPLOT_CMD}
echo "set xtics 1, 1" >> ${GNUPLOT_CMD}
echo "set ylabel \"speed-up\"" >> ${GNUPLOT_CMD}
echo "" >> ${GNUPLOT_CMD}
echo "" >> ${GNUPLOT_CMD}

ind=0
ech=0
for n in $(seq 1 ${NB_NODE});
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

echo "" >> ${GNUPLOT_CMD}
echo "pause -1" >> ${GNUPLOT_CMD}


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

for p in $(seq ${NB_PROC} -1 ${LAST_PROC});
do
    for n in $(seq 2 ${NB_NODE});
    do
        #variable
        SPEEDUP=()

        if [ -f ../topology/2d_mesh_${n}nodes* ]; then
            for t in $(seq 1 ${TRIAL});
            do
                echo ""
                echo "!!!!!!!!! $((${p} + 1)) proc ; $n nodes ; $t trial !!!!!!!!!"
                # compute XSim
                rm ${XSIM_INTERMEDIATE_FILE}_node*.perf
                ./simulation_systemC_3DSPIN_model6 -m 6 -tl ${TL} -n $n -i ${NB_IFACE} -tp ../topology/2d_mesh_${n}nodes* -om ${XSIM_INTERMEDIATE_FILE} > /dev/null
                # read measure
                XSIM_RESULT=$(read_processing_time ${XSIM_INTERMEDIATE_FILE})
                echo "Xsim_result: ${XSIM_RESULT}"

                # compute SystemC
                ./Platform_3DSPIN/simulation_2d_mesh_${n}nodes* ${TL} $p ${SYSC_INTERMEDIATE_FILE} > /dev/null
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
            echo "$((${p}+1))      ${MEAN}     ${STANDARD_VARIATION}" >> ${OUTPUT_FILE}_${n}nodes.gp
        fi
    done
    # stop one proc
    if [ "${p}" -ne "${LAST_PROC}" ];
    then
        ../script/turn_off_n_proc.sh $p $p
        sleep 4
    fi
done
# wake-up all the proc
../script/turn_on_n_proc.sh ${LAST_PROC} ${NB_PROC}


