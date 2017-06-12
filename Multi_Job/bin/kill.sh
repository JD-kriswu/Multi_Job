#!/bin/sh


PIDS=`pidof Multi_Job`
for PID in $PIDS
do
	kill -9 $PID
done
