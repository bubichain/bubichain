#!/bin/sh

ulimit -c unlimited

DIR="$( cd "$( dirname "$0"  )" && pwd  )"

BUBI=$DIR/bin/bubi
BUBI_SLAVE=$DIR/bin/bubi_slave
BUBI_MINITOR=$DIR/bin/bubi_monitor

$DIR/bin/bubi  &
$DIR/bin/bubi_slave  &
$DIR/bin/bubi_monitor  &

