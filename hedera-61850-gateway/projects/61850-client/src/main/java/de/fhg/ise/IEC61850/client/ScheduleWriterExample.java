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

package de.fhg.ise.IEC61850.client;

import de.fhg.ise.IEC61850.client.models.AllianderDER;
import de.fhg.ise.IEC61850.client.scheduling.PreparedSchedule;
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

public class ScheduleWriterExample {

    public static final String HOST_NAME = "localhost";
    public static final int schedule_number = 1;
    public static final Duration interval = ofSeconds(5);

    private static Logger log = LoggerFactory.getLogger(ScheduleWriterExample.class);

    public static void main(String[] args) {
        final List<String> argList = Arrays.asList(args);
        try {
            log.info("Got args {}", argList);

            if (argList.size() < 3) {
                throw new IllegalArgumentException("Too few arguments");
            }

            final int seconds = parseInt(argList.get(0));
            final Instant start = now().plusSeconds(seconds);
            final int prio = parseInt(argList.get(1));
            final List<Number> values = argList.stream()//
                    .skip(2)//
                    .map(Integer::parseInt)//
                    .collect(toList());

            if (seconds < 1) {
                throw new IllegalArgumentException("Seconds " + seconds + " needs to be at last 1");
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
            log.info("Schedule will start running in {}", Duration.between(now(), start));
        } catch (ScheduleEnableException e) {
            log.error("Execution occured while setting up schedule", e.cause);
            log.error("Unable to activate schedule, exiting");
            System.exit(1);
        } catch (Exception e) {
            String example = "30 10 0 10 20 30 40";
            String explantion = "( 30 = seconds until start > 1) ( 42 = prio >= 10) ( 0 10 20 30 40 = control values of the schedule)";
            log.error("Unable to parse input args '{}', failed with {}: {}", argList, e.getClass(), e.getMessage());
            log.info("Arguments '{}' could not be parsed (see above).", argList);
            log.info("Expecting something like '{}'", example);
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
