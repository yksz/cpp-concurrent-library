#!/bin/sh

cd `dirname "${0}"`
if [ ! -e build ] ; then
    mkdir build
fi
cd build
cmake ../ccl
