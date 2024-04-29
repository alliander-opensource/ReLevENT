#!/usr/bin/env bash

proj_directory=$(pwd)

echo $proj_directory
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


if [ ! -d $proj_directory/dependencies/der-scheduling ]; then
  cd $proj_directory
  mkdir -p dependencies
  cd dependencies
  echo Fetching iec61850 scheduler library
  git clone -b develop_mz https://github.com/alliander-opensource/der-scheduling.git
  cd der-scheduling
fi
