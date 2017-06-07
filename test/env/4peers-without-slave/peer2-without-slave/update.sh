#!/bin/sh
DIR="$( cd "$( dirname "$0"  )" && pwd  )"

BUBI=$DIR/bin/bubi

cp -f $DIR/../../../../src/main/bubi $BUBI
chmod 777 $BUBI