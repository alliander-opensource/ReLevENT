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


package org.openmuc.fnn.steuerbox.models;

/**
 * These requirements are meant to be linked from the JavaDoc of unit tests covering them. This way, requirements
 * *should* be linked easily to their tests and vice versa.
 * <p>
 * All requirements are taken from the main github at https://github.com/alliander-opensource/der-scheduling/blob/main/REQUIREMENTS.md
 * .
 * <p>
 * Adding the description as JavaDoc helps to quickly identify what a requirement stands for.
 */
public enum Requirement {

    /**
     * Just a placeholder
     */
    NONE,

    // CONFIGURATION

    /**
     * Basic configuration of DER default shall be stored inside a SCL File
     */
    C01,

    /**
     * Reserve schedules shall be stored inside a SCL File
     */
    C01a,

    /**
     * Number of available schedules (maximum 10 schedules per logical Node, less can be configured) shall be stored
     * inside a SCL File
     */
    C01b,

    /**
     * Number of devices to be controlled (limited to 1 for evaluation, extension shall be foreseen) shall be stored
     * inside a SCL File
     */
    C01c,

    /**
     * Parameters of schedules other than the reserve schedule are to be stored in a key-value manner
     */
    C02,

    /**
     * Parameters of schedules other than the reserve schedule are persisted to a separate file (e.g. as JSON)
     */
    C03,

    /**
     * Changes in schedules are persisted into the file as soon as the schedule is validated
     */
    C04,

    /**
     * A interface to CoMPAS is not required.
     */
    C05,

    /**
     * A simple interface to notify about changes in SCL file is to be defined.
     */
    C05a,

    /**
     * A simple interface to notify about changes in key-value
     */
    C05b,

    // LOGICAL NODES

    /**
     * Map Logical Nodes and Common Data Classes to configuration values, allow reading and writing according to
     * 61850-90-10
     */
    LN01,

    /**
     * The control functions of the DER are modeled and implemented independently
     */
    LN02,

    /**
     * Absolute active power is controlled in 10 logical nodes ActPow_FSCH01 .. ActPow_FSCH10, with schedule controller
     * ActPow_FSCC1 and actuator interface ActPow_GGIO1
     */
    LN02a,

    /**
     * Maximum power is controlled in 10 logical nodes MaxPow_FSCH01 .. MaxPow_FSCH10, with schedule controller
     * MaxPow_FSCC1 and actuator interface MaxPow_GGIO1
     */
    LN02b,

    /**
     * On/Off state of the DER is controlled 10 logical nodes OnOff_FSCH01 .. OnOff_FSCH10, with schedule controller
     * OnOff_FSCC1 and actuator interface OnOff_GGIO1
     */
    LN02c,

    /**
     * The scheduling nodes and their children are to be modeled according to 61850-90-10
     */
    LN03,

    // SCHEDULING

    /**
     * A set of 10 schedules per logical node must be supported
     */
    S01,

    /**
     * Support time based schedules
     */
    S02,

    /**
     * Triggers other than time based may be supported
     */
    S03,

    /**
     * Periodical schedules must be supported
     */
    S04,

    /**
     * Support of absolute schedules of absolute power setpoints
     */
    S05a,

    /**
     * Support of absolute schedules of maximum power setpoints
     */
    S05b,

    /**
     * Support of schedules to turn DER on and off
     */
    S05c,

    /**
     * Validation of Schedules according to 61850-90-10
     */
    S08,

    /**
     * Schedules for maximum generation values and absolute power values are modeled in the same way (using the same
     * DTO)
     */
    S09,

    /**
     * Each schedule must store a sequence of max. 100 values
     */
    S10,

    /**
     * There are three Reserve Schedules: Active Power Reserve Schedule, Maximum Power Reserve Schedule and On/Off
     * Reserve Schedule
     */
    S11,

    /**
     * Each Reserve Schedule holds 100 values
     */
    S12,

    /**
     * Reserve Schedules are always active and cannot be deactivated
     */
    S13,

    /**
     * Reserve Schedules have the lowest priority (10). This cannot be changed.
     */
    S14,

    /**
     * Reserve Schedules have a fixed start date of 01.01.1970 00:00:01 (UTC). This cannot be changed.
     */
    S15,

    /**
     * Reserve Schedules are set to cyclic execution. This cannot be changed.
     */
    S16,

    /**
     * If two schedules are configured with the same pirority and start time, the schedule that comes first in the
     * schedule controller's schedule reference is executed.
     */
    S17,

    // SCHEDULE EXECUTION

    /**
     * Execution of schedules must regard schedule priority according to 61850-10
     */
    E01,

    /**
     * Resolution time base: >= 1 second
     */
    E02,

    // INTERFACES

    /**
     * The Active power Actuator Interface contains the current value (a double) and the source schedule priority (an
     * integer)
     */
    I01,

    /**
     * The Maximum Power Actuator Interface contains the current value (a double) and the source schedule priority (an
     * integer)
     */
    I02,

    /**
     * The On/Off Actuator Interface contains the current value (a double) and the source schedule priority (an
     * integer). The mapping is 0 - off / 1 - on.
     */
    I03;
}
