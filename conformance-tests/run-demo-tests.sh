#!/bin/bash
./gradlew test --tests *ScheduleExecutionTest.test_priorities
echo "Demo tests run. Please see the logs in testResults/ directory for details"
