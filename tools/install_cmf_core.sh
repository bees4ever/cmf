#!/usr/bin/env bash


CWD=$PWD

CMFDIR=$CWD/$(dirname $0)/..

# Gets and makes the dependencies for the new sparese CVodeIntegrator
export CFLAGS="-fPIC"
LIB_DIR=$CMFDIR/lib

CMF_SRC=$CMFDIR/cmf/cmf_core_src

mkdir -p $CMF_SRC/cmake-build-release
cd $CMF_SRC/cmake-build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
make $MAKE_OPTIONS
make install
