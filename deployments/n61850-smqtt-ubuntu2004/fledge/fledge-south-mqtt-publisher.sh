#!/usr/bin/env bash

cd /tmp

git clone https://github.com/mz-automation/fledge-mqtt-publisher-south.git
cd fledge-mqtt-publisher-south/
python3 -m pip install -r python/requirements.txt
sh install-plugin.sh



