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

import com.beanit.iec61850bean.BdaBoolean;
import com.beanit.iec61850bean.BdaFloat32;
import com.beanit.iec61850bean.FcModelNode;
import com.beanit.iec61850bean.ServiceError;
import org.openmuc.fnn.steuerbox.testutils.IEC61850Utility;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Provides access to the Values that are stored inside a ScheduleDefinition. This access is using generics to support
 * differrent {@link ScheduleType}s.
 */
public interface ValueAccess<T> {

    String getValueAccessString(int valueIndex, String scheduleName);

    String getGGIOValueSuffix();

    PreparedSchedule.PreparedScheduleValues<T> prepareWriting(List<T> values, String scheduleName);

    default PreparedSchedule.PreparedScheduleValues prepareWriting(T singleValue, String scheduleName) {
        return prepareWriting(Arrays.asList(singleValue), scheduleName);
    }

    PreparedSchedule prepareSchedule(List<T> values, int scheduleNumber, Duration interval, Instant start, int prio);

    /**
     * Write a default value to this schedule. Useful if several types of schedules are to be processed.
     */
    PreparedSchedule.PreparedScheduleValues activateScheduleWithDefaultValue(String scheduleName);

    T readToTargetValue(FcModelNode node) throws ServiceError, IOException;

    private static PreparedSchedule valueWriterToScheduleWriter(PreparedSchedule.PreparedScheduleValues vw,
            Duration interval, Instant start, int prio) {
        return utility -> utility.writeAndEnableSchedule(vw, interval, start, prio);
    }

    static ValueAccess<? extends Number> asgAccess(IEC61850Utility utility, ScheduleDefinitions schedule) {

        Logger log = LoggerFactory.getLogger(ValueAccess.class.getName() + ".asgAccess");

        return new ValueAccess<Number>() {

            @Override
            public String getValueAccessString(int valueIndex, String scheduleName) {
                return String.format("%s.ValASG%03d.setMag.f", scheduleName, valueIndex);
            }

            @Override
            public String getGGIOValueSuffix() {
                return ".AnOut1.mxVal.f";
            }

            @Override
            public PreparedSchedule prepareSchedule(List<Number> values, int scheduleNumber, Duration interval,
                    Instant start, int prio) {
                return prepareWriting(values, schedule.getScheduleName(scheduleNumber)).asSchedule(interval, start,
                        prio);
            }

            @Override
            public PreparedSchedule.PreparedScheduleValues prepareWriting(List<Number> values, String scheduleName) {
                return new PreparedSchedule.PreparedScheduleValues<Number>() {
                    @Override
                    public void writeValues() throws ServiceError, IOException {
                        int index = 1;
                        for (Number value : values) {
                            String valueBasicDataAttribute = getValueAccessString(index++, scheduleName);
                            log.debug("Writing {} to {}", value, valueBasicDataAttribute);
                            utility.setDataValues(valueBasicDataAttribute, null, value.toString());
                        }
                    }

                    @Override
                    public int size() {
                        return values.size();
                    }

                    @Override
                    public String getScheduleName() {
                        return scheduleName;
                    }

                    @Override
                    public PreparedSchedule asSchedule(Duration interval, Instant start, int prio) {
                        return valueWriterToScheduleWriter(this, interval, start, prio);
                    }

                    @Override
                    public List<Number> getValues() {
                        return Collections.unmodifiableList(values);
                    }
                };
            }

            @Override
            public PreparedSchedule.PreparedScheduleValues activateScheduleWithDefaultValue(String scheduleName) {
                return prepareWriting(Arrays.asList(0), scheduleName);
            }

            @Override
            public Number readToTargetValue(FcModelNode node) throws ServiceError, IOException {
                return ((BdaFloat32) node).getFloat();
            }

        };
    }

    static ValueAccess<Boolean> spgAccess(IEC61850Utility utility, ScheduleDefinitions schedules) {

        Logger log = LoggerFactory.getLogger(ValueAccess.class.getName() + ".spgAccess");

        return new ValueAccess<Boolean>() {
            @Override
            public String getValueAccessString(int valueIndex, String scheduleName) {
                return String.format("%s.ValSPG%03d.setVal", scheduleName, valueIndex);
            }

            @Override
            public String getGGIOValueSuffix() {
                return ".SPCSO1.stVal";
            }

            @Override
            public PreparedSchedule prepareSchedule(List<Boolean> values, int scheduleNumber, Duration interval,
                    Instant start, int prio) {
                return prepareWriting(values, schedules.getScheduleName(scheduleNumber)).asSchedule(interval, start,
                        prio);
            }

            @Override
            public PreparedSchedule.PreparedScheduleValues<Boolean> prepareWriting(List<Boolean> values,
                    String scheduleName) {
                return new PreparedSchedule.PreparedScheduleValues() {
                    @Override
                    public void writeValues() throws ServiceError, IOException {
                        int index = 1;
                        for (Boolean value : values) {
                            String valueBasicDataAttribute = getValueAccessString(index++, scheduleName);
                            log.debug("Writing {} to {}", value, valueBasicDataAttribute);
                            utility.setDataValues(valueBasicDataAttribute, null, value.toString());
                        }
                    }

                    @Override
                    public int size() {
                        return values.size();
                    }

                    @Override
                    public String getScheduleName() {
                        return scheduleName;
                    }

                    @Override
                    public PreparedSchedule asSchedule(Duration interval, Instant start, int prio) {
                        return valueWriterToScheduleWriter(this, interval, start, prio);
                    }

                    @Override
                    public List<Boolean> getValues() {
                        return Collections.unmodifiableList(values);
                    }
                };
            }

            @Override
            public PreparedSchedule.PreparedScheduleValues activateScheduleWithDefaultValue(String scheduleName) {
                return prepareWriting(Arrays.asList(false), scheduleName);
            }

            @Override
            public Boolean readToTargetValue(FcModelNode node) throws ServiceError, IOException {
                return ((BdaBoolean) node).getValue();
            }
        };
    }

}
