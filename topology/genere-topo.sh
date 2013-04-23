#!/bin/bash
# Usage: genere-topo number_of_nodes number_in_x number_in_y
#
# The node are numeroted first in x and then in y.
# It jumps to the next y-line when a x-line is finished.

FILE_BASENAME=2d_mesh
FILE_ENDNAME=topo

OUTPUT_FILE=${FILE_BASENAME}_$1nodes_$2x_$3y.${FILE_ENDNAME}

#variable for the time between 2 nodes
DELAY=2
NBNODE=$(($1-1))
XMAX=$2
YMAX=$3
MULT=1

echo "# nb_nodes: $1 ; for x: $2 ; for y: $3" > $OUTPUT_FILE

# source variable
for s in $(seq 0 $NBNODE)
do
    YS=$(($s/$XMAX))
    XS=$(($s-$YS*$XMAX))
    #destination variable
    for d in $(seq 0 $NBNODE)
    do
        YD=$(($d/$XMAX))
        XD=$(($d-$YD*$XMAX))

        if [ $XD -eq $XS ] && [ $YD -eq $YS ] ; then
                DELAY=${MULT}
        else
            XDISTANCE=$(($XS - $XD))
            YDISTANCE=$(($YS - $YD))
            if [ $XDISTANCE -lt 0 ] ; then
                XDISTANCE=$((-$XDISTANCE))
            fi
            if [ $YDISTANCE -lt 0 ] ; then
                YDISTANCE=$((-$YDISTANCE))
            fi

            DELAY=$((${MULT}*($XDISTANCE + $YDISTANCE)))
        fi
        echo "$s->$d : $DELAY" >> ${OUTPUT_FILE}
    done

    echo "" >> ${OUTPUT_FILE}
done


