#!/usr/bin/env bash

cd /tmp
git clone https://github.com/mz-automation/libiec61850.git
cd libiec61850
export LIB_IEC61850=$PWD
mkdir build
cd build
cmake -DBUILD_TESTS=NO -DBUILD_EXAMPLES=NO ..
make
sudo make install
sudo ldconfig

cd /tmp
git clone -b develop https://github.com/alliander-opensource/ReLevENT.git
cd ReLevENT

mkdir -p dependencies
cd dependencies
git clone -b develop_mz https://github.com/alliander-opensource/der-scheduling.git

cd /tmp/ReLevENT
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DFLEDGE_INCLUDE=/usr/local/fledge/include/ -DFLEDGE_LIB=/usr/local/fledge/lib/ ..
apt install --yes libgtest-dev
make iec61850
sudo mkdir -p $FLEDGE_ROOT/plugins/north/iec61850
sudo cp libiec61850.so $FLEDGE_ROOT/plugins/north/iec61850
