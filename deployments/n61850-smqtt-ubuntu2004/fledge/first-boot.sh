#!/bin/bash

FLEDGE_FOLDER=/usr/local/fledge
INITIAL_SETUP_COMPLETED_MARKER=$FLEDGE_FOLDER/instance_init_completed

set -e

if [[ -f "$INITIAL_SETUP_COMPLETED_MARKER" ]]; then
  echo "Skipping init: was run before ($INITIAL_SETUP_COMPLETED_MARKER exists)."
  exit 0
fi
echo "Waiting for fledge changes to take effect"
sleep 5

echo "## Setting MQTT defaults to local broker"
# test-mqtt-server: the mqtt test docker container, referenced by its docker name
MQTT_HOST="test-mqtt-server"
MQTT_CMD_TOPIC=fledge/south-command
MQTT_SCHEDULE_TOPIC=fledge/south-schedule
MQTT_PORT=1883

echo "Using host=$MQTT_HOST, topcis $MQTT_CMD_TOPIC and $MQTT_SCHEDULE_TOPIC and port $MQTT_PORT"
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"brokerHost\":\"$MQTT_HOST\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"brokerPort\":\"$MQTT_PORT\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"username\":\"$MQTT_USER\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"password\":\"$MQTT_PW\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"topic\":\"$MQTT_CMD_TOPIC\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"op_filter\":\"PivotCommand\"}" >/dev/null 2>&1
echo "...set up command publishing"

curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"brokerHost\":\"$MQTT_HOST\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"brokerPort\":\"$MQTT_PORT\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"username\":\"$MQTT_USER\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"password\":\"$MQTT_PW\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"topic\":\"$MQTT_SCHEDULE_TOPIC\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"op_filter\":\"PivotSchedule\"}" >/dev/null 2>&1
echo "...set up schedule publishing"
sleep 1

curl -X PUT http://localhost:8081/fledge/schedule/enable -d '{"schedule_name": "mqtt_south_s1"}' >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/schedule/enable -d '{"schedule_name": "mqtt_south_s2"}' >/dev/null 2>&1
echo "...enabled both schedulers"

echo "--> successfully set up MQTT to connect to test broker and enabled publishing"


echo "## Setting up IEC 61850 north scheduler interface"
bash prepare-iec61850-scheduling-config.sh
echo "....Prepared IEC 61850 cfg and moved in place"
bash configure_scheduler.sh
echo "...configured IEC 61850 north scheduler interface" 
curl -X PUT http://localhost:8081/fledge/schedule/enable -d '{"schedule_name": "iec61850north_c1"}' >/dev/null 2>&1
echo "...enabled IEC 61850 north scheduler interface"
echo "--> successfully set up and enabled ICE 61850 north scheduler interface"

echo "...requesting restart of FLEDGE to apply new settings"
curl -X PUT  http://localhost:8081/fledge/restart >/dev/null 2>&1

echo "==> Initial setup done"

mkdir -p $FLEDGE_FOLDER
touch $INITIAL_SETUP_COMPLETED_MARKER
