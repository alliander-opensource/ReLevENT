#!/bin/bash

# EMS test reuquest: increase limit to 42 and 1337 watts in 10 seconds.

mosquitto_pub -h localhost -p 1883 -t hedera-requests -m "{"direction":"IMPORT","start":{"seconds":$(date -d '+10sec' '+%s' ),"nanos":0},"resolution":"FIFTEEN_MINUTES","values":[42,1337]}" -i "'test-publisher'"
