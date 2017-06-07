#!/bin/sh

PEER_DIR="$( cd "$( dirname "$0"  )" && pwd  )"

for i in $PEER_DIR/peer*; 
do 
	$i/stop.sh;
done

