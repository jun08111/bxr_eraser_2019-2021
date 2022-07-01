#!/bin/bash

BXRG_NAME=bxr_parser

echo "-------------------------------------------------"
echo " "`date`
echo " Blue X-ray G Parser Down Start... "
echo "-------------------------------------------------"

BXRG_PID=`ps -ef | grep ${BXRG_NAME} | grep -v grep | grep -v tail | awk '{print $2}'`
if [ "$BXRG_PID" = "" ]
then
echo "There is no processor..."

#echo -e "\n"
echo "-------------------------------------------------"
echo " Blue X-ray G Parser Down E n d..."
echo "-------------------------------------------------"
echo -e "\n"
exit
fi

echo "${BXRG_NAME} processor is stopping... pid:[ $BXRG_PID ]"
#echo "kill -9 $BXRG_PID"
kill -9 $BXRG_PID
#echo -e "\n"
echo "-------------------------------------------------"
echo " Blue X-ray G Parser Down E n d..."
echo "-------------------------------------------------"
echo -e "\n"

