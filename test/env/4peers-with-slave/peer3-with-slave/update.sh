#!/bin/sh
DIR="$( cd "$( dirname "$0"  )" && pwd  )"

BUBI=$DIR/bin/bubi
BUBI_SLAVE=$DIR/bin/bubi_slave

cp -f $DIR/../../../../src/main/bubi $BUBI;
cp -f $DIR/../../../../src/slave/bubi_slave $BUBI_SLAVE;
chmod 777 $BUBI $BUBI_SLAVE;