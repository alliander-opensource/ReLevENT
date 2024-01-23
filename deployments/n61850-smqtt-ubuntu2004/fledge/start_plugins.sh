#!/usr/bin/env bash

curl -X PUT http://localhost:8081/fledge/schedule/enable -d '{"schedule_name": "mqtt_south_s1"}'
curl -X PUT http://localhost:8081/fledge/schedule/enable -d '{"schedule_name": "mqtt_south_s2"}'

curl -X PUT http://localhost:8081/fledge/schedule/enable -d '{"schedule_name": "iec61850north_c1"}'