#!/bin/bash

# this script builds the project, builds a jar that contains all dependencies and runs this jar

# exit on errors
set -e

./gradlew clean shadowJar

java -jar demo-build/gateway-app-0.1-SNAPSHOT-all.jar
