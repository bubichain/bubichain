#!/bin/bash
basepath=$(cd `dirname $0`; pwd)
GCC_VERSION=$(gcc -dumpversion)
export MYSCONS=${basepath}"/scons-2.1.0/"
echo $MYSCONS
export SCONS_LIB_DIR=$MYSCONS/engine
python $MYSCONS/script/scons platform=linux-gcc $*
mv "libs/linux-gcc-${GCC_VERSION}/libjson_linux-gcc-${GCC_VERSION}_libmt.so" libs/libjson.so
mv "libs/linux-gcc-${GCC_VERSION}/libjson_linux-gcc-${GCC_VERSION}_libmt.a" libs/libjson.a

