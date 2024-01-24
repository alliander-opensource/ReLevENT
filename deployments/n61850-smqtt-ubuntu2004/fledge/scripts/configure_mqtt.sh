#!/usr/bin/env bash

MQTT_PORT=1883
echo "checking env variables MQTT_HOST, MQTT_USER, MQTT_PW or using defaults."

MQTT_HOST="${MQTT_HOST-localhost}"
MQTT_USER="${MQTT_USER-anonymous}"
MQTT_PW="${MQTT_PW-anonymous}"

MQTT_CMD_TOPIC=fledge/south-command
MQTT_SCHEDULE_TOPIC=fledge/south-schedule
MQTT_PORT=1883

echo "Using host=$MQTT_HOST user=$MQTT_USER (not showing password here)"
echo "Using hard topcis $MQTT_CMD_TOPIC and $MQTT_SCHEDULE_TOPIC and hard coded port $MQTT_PORT"

curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"brokerHost\":\"$MQTT_HOST\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"brokerPort\":\"$MQTT_PORT\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"username\":\"$MQTT_USER\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"password\":\"$MQTT_PW\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"topic\":\"$MQTT_CMD_TOPIC\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s1 -d "{\"op_filter\":\"PivotCommand\"}" >/dev/null 2>&1

curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"brokerHost\":\"$MQTT_HOST\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"brokerPort\":\"$MQTT_PORT\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"username\":\"$MQTT_USER\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"password\":\"$MQTT_PW\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"topic\":\"$MQTT_SCHEDULE_TOPIC\"}" >/dev/null 2>&1
curl -X PUT http://localhost:8081/fledge/category/mqtt_south_s2 -d "{\"op_filter\":\"PivotSchedule\"}" >/dev/null 2>&1
