#!/usr/bin/env bash

curl -X PUT http://localhost:8081/fledge/category/iec61850north_c1 -d "@./north_iec61850_exchanged_data.json" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/iec61850north_c1 -d "@./north_iec61850_protocol_stack.json" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/iec61850north_c1 -d "@./north_iec61850_scheduler_config.json" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/iec61850north_c1 -d '{"modelPath":"/usr/local/fledge/model.cfg"}' >/dev/null 2>&1
