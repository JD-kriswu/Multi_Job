#/bin/bash
# author : kriswu 

DIRNAME=`dirname $0`
cd ${DIRNAME}

PATHNAME=`pwd`
FILENAME=`basename $0`
BASE=`dirname ${PATHNAME} | xargs basename`




ps aux | grep "./"${BASE}" ../etc/"${BASE}".xml" | grep -v grep
