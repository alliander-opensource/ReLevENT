#!/bin/bash

# Constants
s1_south_service_name="mqtt_south_s1"
s2_south_service_name="mqtt_south_s2"
s1_south_service_name_advanced="${s1_south_service_name}Advanced"
s2_south_service_name_advanced="${s2_south_service_name}Advanced"

n1_north_service_name="iec61850north_c1"

# Create service south
curl -sX POST http://localhost:8081/fledge/service -d '{"name":"'$s1_south_service_name'","type":"south","plugin":"mqtt-publisher","enabled":false}'
curl -sX POST http://localhost:8081/fledge/service -d '{"name":"'$s2_south_service_name'","type":"south","plugin":"mqtt-publisher","enabled":false}'

# Create service north
curl -sX POST http://localhost:8081/fledge/service -d '{"name":"'$n1_north_service_name'","type":"north","plugin":"iec61850","enabled":false}'

# Create service notification
curl -sX POST http://localhost:8081/fledge/service -d '{"name":"'$name_service_notif'","type":"notification","enabled":true}'

# Param advanced south
sleep 5
#curl -X PUT --data '{"maxSendLatency":"100"}' http://localhost:8081/fledge/category/$s1_south_service_name_advanced
#curl -X PUT --data '{"maxSendLatency":"100"}' http://localhost:8081/fledge/category/$s2_south_service_name_advanced

# Param purge
curl -X PUT --data '{"value":"1"}' http://localhost:8081/fledge/category/PURGE_READ/age
curl -X PUT --data '{"value":"100000"}' http://localhost:8081/fledge/category/PURGE_READ/size
