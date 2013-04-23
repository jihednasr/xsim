#!/bin/bash
# Ce script génère tous les fichiers .plot pour la configuration du diagramme
# et tous les fichiers .gp pour les données à tracer
# Le tous est placé dans les répertoires
# ../gnuplot/gnuplot_command et ../gnuplot/gnuplot_data

SRC="./"
DEST_D="../gnuplot/gnuplot_data"
DEST_C="../gnuplot/gnuplot_command"

NB_PROC=2
NB_IFACE=1
NB_NODE=2
TIME=100
MODEL=4

while [ "$#" -gt "0" ]
do
    if [ "$1" == "-h" ]; then
        echo for help: $0 -h 
        echo usualy:   $0 \[-np nb_proc\] \[-nn nb_node\] \[-i nb_interface\] \[-m model\]
        exit
    elif [ "$1" == "-np" ]; then
        NB_PROC=$2
    elif [ "$1" == "-nn" ]; then
        NB_IFACE=$2
    elif [ "$1" == "-i" ]; then
        NB_NODE=$2
    elif [ "$1" == "-m" ]; then
        MODEL=$2
    else
        echo error in the arguments given
        exit 
    fi
    shift 2
done

rm $DEST_D/*
echo rm $DEST_D/*
rm $DEST_C/*
echo rm $DEST_C/*

# pour toutes les configurations interessantes:
#   cd ..
#   faire un make avec les bonnes options de config
#   cd tests/
#   make clean ; make
#   cd ../gnuplot/
#   executer ./gnuplot_output -tl $TIME -nbp $NB_PROC -nbn $NB_NODE -nbi $NB_IFACE -m $MODEL
#   cd ../script

# TODO: faire ce qui est écrit au-dessus


echo DONE
