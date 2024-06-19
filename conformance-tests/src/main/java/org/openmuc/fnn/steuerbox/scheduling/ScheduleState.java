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
 * Schedule states defined in IEC61850-90-10 ed 2017, Table 8 (page 28)
 */
public enum ScheduleState {
    /**
     * Fallback state for error cases
     */
    UNKNOWN(0),
    NOT_READY(1),
    START_TIME_REQUIRED(2),
    READY(3),
    RUNNING(4);

    private final int value;

    private ScheduleState(int value) {
        this.value = value;
    }

    public static ScheduleState parse(String valueString) {
        try {
            Integer value = Integer.parseInt(valueString);
            for (ScheduleState state : ScheduleState.values()) {
                if (state.value == value) {
                    return state;
                }
            }
        } catch (Exception e) {
            return UNKNOWN;
        }
        return UNKNOWN;
    }
}
