#!/usr/bin/env bash

directory=$1
if [ ! -d $directory ]; then
  mkdir -p $directory
else
  directory=~
fi

if [ ! -d $directory/libiec61850 ]; then
  cd $directory
  echo Fetching MZA libiec61850 library
  git clone https://github.com/mz-automation/libiec61850.git
  cd libiec61850
  mkdir build
  cd build
  cmake -DBUILD_TESTS=NO -DBUILD_EXAMPLES=NO ..
  make
  sudo make install
fi

if [ ! -d $directory/der-scheduling ]; then
  cd $directory
  echo Fetching iec61850 scheduler library
  git clone https://github.com/alliander-opensource/der-scheduling.git
  cd der-scheduling
  mkdir build
  cd build
  cmake ..
  make
  sudo make install
fi
