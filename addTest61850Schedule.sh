#/bin/bash

# Builds and runs a 61850 client that creates IEC 61850 schedule and runs it.

set -e

cd hedera-61850-gateway

./gradlew shadowJar
java -jar  demo-build/61850-client-0.1-SNAPSHOT-all.jar $@

cd ..
