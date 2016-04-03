#!/bin/sh

DIRNAME="eclipse"

cd `dirname "${0}"`
if [ ! -e ${DIRNAME} ] ; then
    mkdir ${DIRNAME}
fi
cd ${DIRNAME}
cmake -G "Eclipse CDT4 - Unix Makefiles" \
    -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE \
    -DCMAKE_ECLIPSE_VERSION=4.4 \
    -DCMAKE_BUILD_TYPE=Debug \
    -Dbuild_tests=ON \
    -Dbuild_examples=ON \
    ../ccl

# On Eclipse: Import > General > Existing Projects into Workspace
