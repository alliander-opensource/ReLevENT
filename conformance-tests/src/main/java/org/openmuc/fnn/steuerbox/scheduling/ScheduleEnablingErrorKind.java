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

/**
 * Representation of error codes ("ScheduleEnablingErrorKind") defined in IEC61850-90-10 ed 2017, Table 9 (page 28)
 */
public enum ScheduleEnablingErrorKind {
    NONE(1),
    MISSING_VALID_NUMENTR(2),
    MISSING_VALID_SCHDINTV(3),
    MISSING_VALID_SCHEDULE_VALUES(4);

    private final int value;

    ScheduleEnablingErrorKind(int value) {
        this.value = value;
    }

    public static ScheduleEnablingErrorKind parse(String valueString) {
        try {
            Integer value = Integer.parseInt(valueString);
            for (ScheduleEnablingErrorKind errorKind : ScheduleEnablingErrorKind.values()) {
                if (errorKind.value == value) {
                    return errorKind;
                }
            }
        } catch (Exception e) {
            return NONE;
        }
        return NONE;
    }
}
