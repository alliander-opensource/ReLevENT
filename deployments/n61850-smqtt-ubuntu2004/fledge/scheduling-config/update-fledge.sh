
#!/bin/bash

java -jar ~/git/libiec61850/tools/model_generator/genconfig.jar model.cid model.cfg
docker cp model.cfg a72a07d00a96:/tmp
/configure_scheduler.sh
cd ../scripts
./fledge_restart.sh
cd -
