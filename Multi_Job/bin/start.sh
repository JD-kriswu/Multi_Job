#!/bin/bash
# author : kriswu 

DIRNAME=`dirname $0`
cd ${DIRNAME}

PATHNAME=`pwd`
FILENAME=`basename $0`
BASE=`dirname ${PATHNAME} | xargs basename`

PROC=$BASE

ulimit -c unlimited

if [ "$1" == "check" ]
then
    NUM=`crontab -l | grep "${PATHNAME}/start" | wc -l`
	if [ $NUM -le 0 ]
	then
		CURR=`date +%y%m%d%H%M%S`
		crontab -l > ./crontab.bak${CURR}
		cat ./crontab.bak${CURR} > ./crontab.tmp
		echo "*/1 * * * * ${PATHNAME}/start check >/dev/null 2>&1 &" >> crontab.tmp
		crontab ./crontab.tmp
	fi
fi
if [ -f ${PROC} ]
then
	NUM=`ps aux | grep "./"${PROC}" ../etc/"${PROC}".xml" | grep -v grep | grep -v ${FILENAME} | wc -l`
	echo $NUM
	if [ $NUM -le 0 ]
	then
			"./"${PROC} ../etc/${PROC}.xml
	fi
fi
