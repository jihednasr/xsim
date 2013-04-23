#!/bin/bash

EXPERIENCE_DIR=test
NUM=1
while [ -d ${EXPERIENCE_DIR}${NUM} ]; 
do
    NUM=$((${NUM}+1))
done
EXPERIENCE_DIR=${EXPERIENCE_DIR}${NUM}
mkdir ${EXPERIENCE_DIR}

# experience with Window et time info freq
#echo "*********************************************"
#echo "**** experience with Window et time info freq"
#echo "*********************************************"
#echo ""
#echo "*********************************************" > log
#echo "**** experience with Window et time info freq" >> log
#echo "*********************************************" >> log
#echo "" >> log
#./compute_experience.sh 0 1 >> log 2>&1
#mkdir ${EXPERIENCE_DIR}/window+info_freq
#mv measures.gp          ${EXPERIENCE_DIR}/window+info_freq
#mv gnuplot_command.plot ${EXPERIENCE_DIR}/window+info_freq
#mv log                  ${EXPERIENCE_DIR}/window+info_freq



## experience with nb node et nb iface
echo "****************************************"
echo "**** experience with nb node et nb iface"
echo "****************************************"
echo ""
echo "*********************************************" > log
echo "**** experience with Window et time info freq" >> log
echo "*********************************************" >> log
echo "" >> log
./compute_experience.sh 2 3 >> log 2>&1
mkdir ${EXPERIENCE_DIR}/node+iface
mv log                  ${EXPERIENCE_DIR}/node+iface



## experience with Window et iface
#echo "************************************"
#echo "**** experience with Window et iface"
#echo "************************************"
#echo ""
#echo "*********************************************" > log
#echo "**** experience with Window et time info freq" >> log
#echo "*********************************************" >> log
#echo "" >> log
#./compute_experience.sh 0 3 >> log 2>&1
#mkdir ${EXPERIENCE_DIR}/window+iface
#mv measures.gp          ${EXPERIENCE_DIR}/window+iface
#mv gnuplot_command.plot ${EXPERIENCE_DIR}/window+iface
#mv log                  ${EXPERIENCE_DIR}/window+iface
#
#
#
## experience with Window et node
#echo "***********************************"
#echo "**** experience with Window et node"
#echo "***********************************"
#echo ""
#echo "*********************************************" > log
#echo "**** experience with Window et time info freq" >> log
#echo "*********************************************" >> log
#echo "" >> log
#./compute_experience.sh 0 2 >> log 2>&1
#mkdir ${EXPERIENCE_DIR}/window+node
#mv measures.gp          ${EXPERIENCE_DIR}/window+node
#mv gnuplot_command.plot ${EXPERIENCE_DIR}/window+node
#mv log                  ${EXPERIENCE_DIR}/window+node

