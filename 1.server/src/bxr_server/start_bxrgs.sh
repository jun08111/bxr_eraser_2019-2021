#!/bin/bash

BXRG_NAME=bxr_server

echo "-------------------------------------------------"
echo " "`date`
echo " Blue X-ray G Server UP Start... " 
echo "-------------------------------------------------"

BXRG_PID=`ps -ef | grep -v tail | grep -v grep | grep ${BXRG_NAME} | awk '{print $2}'`
if [ "$BXRG_PID" = "" ]
then
echo -e "\n"
${BXRG_HOME}/bin/bxr_server
echo "${BXRG_NAME} processor is startting..."
echo -e "\n"
sleep 1
BXRG_PID=`ps -ef | grep -v grep | grep bxr_server | awk '{print $2}'`
echo "-------------------------------------------------"
echo " Blue X-ray G Server UP E n d...  pid:[ $BXRG_PID ]"
echo "-------------------------------------------------"
echo -e "\n"
exit
fi

echo "There exists processor..."
#echo -e "\n"
echo "-------------------------------------------------"
echo " Blue X-ray G Server UP E n d...  pid:[ $BXRG_PID ]"
echo "-------------------------------------------------"
echo -e "\n"

