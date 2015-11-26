#!/bin/sh

VERSION='1.7.0'
INSTALL_DIR='/usr/local'
DOWNLOAD_DIR='/tmp'

wget -P ${DOWNLOAD_DIR} https://github.com/google/googletest/archive/release-${VERSION}.tar.gz
cd ${DOWNLOAD_DIR}
tar xzvf release-${VERSION}.tar.gz
cd googletest-release-${VERSION}
mkdir build
cd build
cmake ..
make

# Install
sudo mkdir -p ${INSTALL_DIR}/include
sudo cp -r ../include/gtest ${INSTALL_DIR}/include
sudo mkdir -p ${INSTALL_DIR}/lib
sudo cp *.a ${INSTALL_DIR}/lib

# Cleanup
cd ${DOWNLOAD_DIR}
rm release-${VERSION}.tar.gz
rm -r googletest-release-${VERSION}
