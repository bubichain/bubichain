#!/bin/sh

DIR="$( cd "$( dirname "$0"  )" && pwd  )"

BUBI=$DIR/bin/bubi

$BUBI --dropdb;
