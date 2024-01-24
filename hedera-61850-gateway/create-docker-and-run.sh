#!/bin/bash

# This script is used to create a docker image of the HEDERA 61850 Gateway and run it.
#
# Hint: actually, two docker images are created, there is one plain image that does not contain any secrets
# ("relevent-61850-hedera-gateway-plain"). If a file docker-image/hedera-interface.ini is provided, also the second
# image ("relevent-61850-hedera-gateway") is created, otherwise this script fails.
# This script was tested on Ubuntu 20.04 LTS.

set -e

DOCKER_BUILD_OPTIONS=""
# set your proxy here, if required
PROXY=""

SCRIPT_DIR=$(dirname $(realpath -s $0))
CONFIG_FILE=$SCRIPT_DIR/docker-image/hedera-interface.ini

echo "## Building jar"
./gradlew  shadowJar
mkdir -p "${SCRIPT_DIR}/docker-image/java-build"
cp demo-build/*.jar ${SCRIPT_DIR}/docker-image/java-build
echo "..done building jar"

echo "## Building general docker image"
docker build $DOCKER_BUILD_OPTIONS ${SCRIPT_DIR}/docker-plain-image \
  -t "relevent-61850-hedera-gateway-plain:0.1"
echo "..done building general docker image"

if [ ! -f $CONFIG_FILE ]; then
  echo "Config file $CONFIG_FILE required in order to create a working docker image."
  exit 1
fi


echo "## Building docker image with config $CONFIG_FILE "
  docker build $DOCKER_BUILD_OPTIONS ${SCRIPT_DIR}/docker-image \
               -t "relevent-61850-hedera-gateway:0.1"
  echo "..done building docker image with config"


docker run --network host relevent-61850-hedera-gateway:0.1
