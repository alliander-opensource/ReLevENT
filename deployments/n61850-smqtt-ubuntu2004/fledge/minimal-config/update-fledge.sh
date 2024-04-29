# Converts the supplied IEC 61850 model.cid into a CFG file that can be parsed by the underlying libiec61850 and copy it into a docker container

#!/bin/bash

DOCKER_CONTAINER_ID=a72a07d00a96
# TODO: add genconfig.jar, do not reference local installation
java -jar ~/git/libiec61850/tools/model_generator/genconfig.jar model.cid model.cfg
docker cp model.cfg $DOCKER_CONTAINER_ID:/tmp
./configure_scheduler.sh
cd ../scripts
./fledge_restart.sh
cd -
