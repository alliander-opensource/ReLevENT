#!/bin/bash

# Skript to check proper licensing setup

./gradlew clean checkLicense

# check that java files have Apache2.0 license header
APACHE_2_LICENSE="Licensed under the Apache License, Version 2.0"
echo "the following files do not have a proper header:"
find ./ -iname "*.java" -print  | xargs grep -L "$APACHE_2_LICENSE"
echo "<end of list>"
