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

package org.openmuc.fnn.steuerbox.scheduling;

import java.time.Duration;
import java.time.Instant;
import java.util.Collection;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Contains 61850 references to essential nodes of a schedule
 */
public interface ScheduleDefinitions<T> {

    /**
     * Returns access to schedule values such that these can be modified on the  61850 device
     */
    ValueAccess<T> getValueAccess();

    /**
     * Prepare a schedule to be
     */
    default PreparedSchedule prepareSchedule(Collection<T> values, int scheduleNumber, Duration interval, Instant start,
            int prio) {
        ValueAccess<T> valueAccess = getValueAccess();
        return valueAccess.prepareSchedule(values, scheduleNumber, interval, start, prio);
    }

    /**
     * Get the schedule name from the number
     *
     * @throws IllegalArgumentException
     *         if no schedule with that number is known
     */
    String getScheduleName(int scheduleNumber) throws IllegalArgumentException;

    /**
     * Get the number of a schedule from its name.
     *
     * @throws IllegalArgumentException
     *         if no scheudle with that name is known
     */
    int getScheduleNumber(String scheduleName) throws IllegalArgumentException;

    String getControlledGGIO();

    String getGGIOValueReference();

    String getController();

    String getReserveSchedule();

    Collection<String> getAllScheduleNames();

    ScheduleType getScheduleType();

    T getDefaultValue();

    default Collection<T> getDefaultValues(int numberOfValues) {
        if (numberOfValues < 1) {
            throw new IllegalArgumentException(
                    "numberOfValues (the size of the returned collection) needs to be at least 1");
        }
        final T defaultValue = getDefaultValue();
        return Stream.generate(() -> defaultValue).limit(numberOfValues).collect(Collectors.toList());
    }
}
