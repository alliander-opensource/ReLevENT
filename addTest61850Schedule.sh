#!/bin/bash
#
# DSO-like schedule transmitter for remote control
#
# Transmits a schedule via 61850 to a server running at localhost, port 102. The schedule name is hard coded to 'DER_Scheduler_Control/MaxPow_FSCH01'.
# The command expects at least 3 parameters:
# - 1st parameter is the number of seconds until the schedule starts
# - 2nd parameter is the priority of the schedule. It must be larger than 10.
# - 3rd and following parameters are the schedule values.

# exit on error
set -e

cd hedera-61850-gateway

./gradlew shadowJar

java -jar  demo-build/61850-client-0.1-SNAPSHOT-all.jar $@
