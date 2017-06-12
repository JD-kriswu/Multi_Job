#!/bin/sh
# author : kriswu 

DIRNAME=`dirname $0`
cd ${DIRNAME}

PATHNAME=`pwd`
FILENAME=`basename $0`
BASE=`dirname ${PATHNAME} | xargs basename`

PROC=$BASE

isInstalled=`crontab -l | grep "${PATHNAME}/start" | wc -l`
if [ ${isInstalled} -gt 0 ]
then
    CURR=`date +%y%m%d%H%M%S`
    crontab -l > ./crontab.bak${CURR}
    crontab -l | grep -v "${PATHNAME}/start" > ./crontab.tmp
    crontab ./crontab.tmp
fi


PROC_PID=`ps aux | grep "./"${PROC}" ../etc/"${PROC}".xml" | grep -v grep | awk '{print $2}'`
kill -s USR2 ${PROC_PID}

