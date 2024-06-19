#!/bin/bash

./gradlew checkLicense

echo TODO: start the below as screen sessions, stop after tests!
echo "cd ../deployments/n61850-smqtt-ubuntu2004/ && docker-compose up"
echo "cd ../hedera-61850-gateway/ && docker-compose up"
echo mosquitto_sub -t fledge/south-schedule -i schedule-subscriber
echo mosquitto_sub -t fledge/south-command -i cmd-subscriber

./gradlew test
echo "All tests run. Please see the logs in testResults/ directory for details"
