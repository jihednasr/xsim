#!/bin/bash
# This script converts all the gnuplot files in the repository ../gnuplot/gnuplot_command in png files.

REP="../gnuplot"
SRC="./gnuplot_command"
DEST="./pdf_gnuplot"
PATTERN=".plot"

cd ${REP}

if [ $(ls $SRC/ | grep ${PATTERN}~ | wc -l) -gt 0 ]; then
	rm ${SRC}/*${PATTERN}~
fi

for f in $(ls $SRC/ | grep ${PATTERN}); do
	tmp=$SRC/${f%.*}
    sed -i '7,+1s/#//g' ${SRC}/$f
    sed -i 's/pause/#pause/g' ${SRC}/$f
    gnuplot ${SRC}/$f
    sed -i '7,+1s/ s/# s/g' ${SRC}/$f
    sed -i 's/#pause/pause/g' ${SRC}/$f
done

cd -
