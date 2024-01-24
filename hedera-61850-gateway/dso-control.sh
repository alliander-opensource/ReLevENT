#!/bin/bash

# exit on error
set -e

./gradlew shadowJar

java -jar  demo-build/61850-client-0.1-SNAPSHOT-all.jar $@
