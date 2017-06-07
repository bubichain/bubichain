#!/bin/sh

PEER_DIR="$( cd "$( dirname "$0"  )" && pwd  )"

for i in $PEER_DIR/peer*; 
do 
	rm -rf $i/log/*
	$i/update.sh;
	$i/dropdb.sh;
done

$PEER_DIR/peer1-without-slave/start.sh
$PEER_DIR/peer2-without-slave/start.sh
$PEER_DIR/peer4-without-slave/start.sh
