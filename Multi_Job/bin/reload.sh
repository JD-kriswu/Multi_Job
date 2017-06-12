#/bin/bash
# author : kriswu 

DIRNAME=`dirname $0`
cd ${DIRNAME}

PATHNAME=`pwd`
FILENAME=`basename $0`
BASE=`dirname ${PATHNAME} | xargs basename`

PROC_PID=`ps aux | grep "./"${BASE}" ../etc/"${BASE}".xml" | grep -v grep | awk '{print $2}'`
kill -s USR1 ${PROC_PID}



