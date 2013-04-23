#!/bin/bash
# Usage: ./parse_output_file.sh input_file_preposition output_file  nb_iteration
#

INPUT_FILE=$1
#OUTPUT_FILE=$2
#LIMIT=$3
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
            array=($fin)
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
            #echo "${RESULTAT}"
            if [ "${RESULTAT}" -gt "${MAX_TIME}" ];
            then
                MAX_TIME=${RESULTAT}
            fi
        fi
    done < ${file}
done

echo "${MAX_TIME}"
#echo "${LIMIT}      ${MAX_TIME}" >> ${OUTPUT_FILE}

