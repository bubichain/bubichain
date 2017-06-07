#!/bin/sh

PEER_DIR="$( cd "$( dirname "$0"  )" && pwd  )"

for i in $PEER_DIR/peer*; 
do 
	rm -rf $i/log/*
	$i/update.sh;
	$i/dropdb.sh;
	$i/start.sh;
done

