#!/bin/sh

ulimit -c unlimited

DIR="$( cd "$( dirname "$0"  )" && pwd  )"

BUBI=$DIR/bin/bubi
BUBI_MINITOR=$DIR/bin/bubi_monitor

($BUBI  &);
($BUBI_MINITOR  &);