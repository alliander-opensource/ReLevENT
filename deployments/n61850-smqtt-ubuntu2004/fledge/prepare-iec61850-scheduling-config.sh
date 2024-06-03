#!/usr/bin/env bash

# Transform an IEC61850 server configuration file (model.cid) into a .cfg file that can be used by MZ-Automations IEC61850 server

wget https://github.com/mz-automation/libiec61850/raw/5b350102de108d148ae03fe32e26deb755ddf6e3/tools/model_generator/genconfig.jar -O genconfig.jar
java -jar genconfig.jar model.cid model.cfg
