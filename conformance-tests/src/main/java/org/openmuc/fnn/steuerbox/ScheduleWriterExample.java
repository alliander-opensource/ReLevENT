/*
 * Copyright 2023 Fraunhofer ISE
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

package org.openmuc.fnn.steuerbox;

import org.openmuc.fnn.steuerbox.models.AllianderDER;
import org.openmuc.fnn.steuerbox.scheduling.PreparedSchedule;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.time.Duration;
import java.time.Instant;
import java.util.Arrays;
import java.util.List;

import static java.lang.Integer.parseInt;
import static java.time.Duration.ofSeconds;
import static java.time.Instant.now;
import static java.util.stream.Collectors.toList;

/**
 * Example class to show how modifying schedules works. Not actually relevant for testing, rather intended as an example
 * on how to modify schedules.
 */
public class ScheduleWriterExample {

    public static final String HOST_NAME = "192.168.17.200";

    private static Logger log = LoggerFactory.getLogger(ScheduleWriterExample.class);

    public static void main(String[] args) {
        final Instant setupStart = now();
        final List<String> argList = Arrays.asList(args);
        try {
            final Duration interval = ofSeconds(5);
            log.info("Got args {}", argList);

            if (argList.size() < 4) {
                throw new IllegalArgumentException("Too few arguments");
            }

            final int schedule_number = parseInt(argList.get(0));
            final int seconds = parseInt(argList.get(1));
            final Instant start = now().plusSeconds(seconds);
            final int prio = parseInt(argList.get(2));
            final List<Number> values = argList.stream()//
                    .skip(3)//
                    .map(Integer::parseInt)//
                    .collect(toList());

            if (seconds < 1) {
                throw new IllegalArgumentException("Seconds " + seconds + " needs to be at last 1");
            }

            if (schedule_number < 1 || schedule_number > 10) {
                throw new IllegalArgumentException(
                        "Illegal schedule number " + schedule_number + " needs to be between 1 and 10");
            }

            if (prio < 10) {
                throw new IllegalArgumentException("Prio " + prio + "too low: needs to be equal to or larger than 10");
            }

            log.info("Setting up new schedule with prio {} starting at {} with interval {} and values {}", prio, start,
                    interval, values);

            try (final AllianderDER allianderDER = new AllianderDER(HOST_NAME, 102)) {
                PreparedSchedule preparedSchedule = allianderDER.maxPowerSchedules.prepareSchedule(values,
                        schedule_number, interval, start, prio);
                allianderDER.writeAndEnableSchedule(preparedSchedule);

            } catch (Exception e) {
                throw new ScheduleEnableException(e);
            }
            log.info("SUCCESS! Set up schedule in {}", Duration.between(setupStart, Instant.now()));
            log.info("Schedule will start running in {} (IN CASE THE CLOCK OF THE DEVICE IS SET UP PROPERLY)",
                    Duration.between(now(), start));
        } catch (ScheduleEnableException e) {
            log.error("Execution occured while setting up schedule", e.cause);
            log.error("Unable to activate schedule, exiting");
            System.exit(1);
        } catch (Exception e) {
            String explantion = "(schedule number [1-10]) (seconds until start > 1) (prio > 10) (value1 value2 value3)";
            String example = "1 30 42 0 10 20 30 40";
            log.error("Unable to parse input args '{}', failed with {}: {}", argList, e.getClass(), e.getMessage());
            log.info("Arguments '{}' could not be parsed (see above).", argList);
            log.info("Expecting something like '{}'", example);
            log.info("or at least like '{}'", "1 0 11 42");
            log.info("where '{}'.", explantion);
            System.exit(2);
        }
    }

    static class ScheduleEnableException extends Exception {
        final Exception cause;

        public ScheduleEnableException(Exception cause) {
            this.cause = cause;
        }
    }
}
