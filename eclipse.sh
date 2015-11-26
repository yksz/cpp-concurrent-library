#!/bin/sh

cd `dirname "${0}"`
if [ ! -e build ] ; then
    mkdir build
fi
cd build
cmake -G "Eclipse CDT4 - Unix Makefiles" \
    -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE \
    -DCMAKE_ECLIPSE_VERSION=4.4 \
    -Dbuild_tests=ON \
    -Dbuild_examples=ON \
    ../ccl
