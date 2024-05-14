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

package de.fhg.ise.IEC61850.client.models;

import com.beanit.iec61850bean.ServiceError;
import de.fhg.ise.IEC61850.client.IEC61850Utility;
import de.fhg.ise.IEC61850.client.scheduling.ScheduleDefinitions;
import de.fhg.ise.IEC61850.client.scheduling.ScheduleType;

import java.io.IOException;

/**
 * Holds schedule names as used in for Allianders ReLevENT IEC 61850 server
 */
public class AllianderDER extends IEC61850Utility {

    public final String host;
    public final int port;

    public AllianderDER(String host, int port) throws ServiceError, IOException {
        super(host, port);
        this.host = host;
        this.port = port;
    }

    public static AllianderDER getWithDefaultSettings() throws ServiceError, IOException {
        return new AllianderDER("127.0.0.1", 102);
    }

    public AllianderDER reconnect() throws ServiceError, IOException {
        return new AllianderDER(host, port);
    }

    public final ScheduleDefinitions<Number> powerSchedules = ScheduleType.ASG.withScheduleDefinitions(this,
            "DER_Scheduler_Control/ActPow_GGIO1",//
            "DER_Scheduler_Control/ActPow_FSCC1",//
            "DER_Scheduler_Control/ActPow_Res_FSCH01",//
            "DER_Scheduler_Control/ActPow_FSCH01", //
            "DER_Scheduler_Control/ActPow_FSCH02", //
            "DER_Scheduler_Control/ActPow_FSCH03", //
            "DER_Scheduler_Control/ActPow_FSCH04", //
            "DER_Scheduler_Control/ActPow_FSCH05", //
            "DER_Scheduler_Control/ActPow_FSCH06", //
            "DER_Scheduler_Control/ActPow_FSCH07", //
            "DER_Scheduler_Control/ActPow_FSCH08", //
            "DER_Scheduler_Control/ActPow_FSCH09", //
            "DER_Scheduler_Control/ActPow_FSCH10");

    public final ScheduleDefinitions<Number> maxPowerSchedules = ScheduleType.ASG.withScheduleDefinitions(this,
            "DER_Scheduler_Control/MaxPow_GGIO1",//
            "DER_Scheduler_Control/MaxPow_FSCC1",//
            "DER_Scheduler_Control/MaxPow_Res_FSCH01",//
            "DER_Scheduler_Control/MaxPow_FSCH01", //
            "DER_Scheduler_Control/MaxPow_FSCH02", //
            "DER_Scheduler_Control/MaxPow_FSCH03", //
            "DER_Scheduler_Control/MaxPow_FSCH04", //
            "DER_Scheduler_Control/MaxPow_FSCH05", //
            "DER_Scheduler_Control/MaxPow_FSCH06", //
            "DER_Scheduler_Control/MaxPow_FSCH07", //
            "DER_Scheduler_Control/MaxPow_FSCH08", //
            "DER_Scheduler_Control/MaxPow_FSCH09", //
            "DER_Scheduler_Control/MaxPow_FSCH10");

    public final ScheduleDefinitions<Boolean> onOffSchedules = ScheduleType.SPG.withScheduleDefinitions(this,
            "DER_Scheduler_Control/OnOff_GGIO1",//
            "DER_Scheduler_Control/OnOff_FSCC1",//
            "DER_Scheduler_Control/OnOff_Res_FSCH01",//
            "DER_Scheduler_Control/OnOff_FSCH01", //
            "DER_Scheduler_Control/OnOff_FSCH02", //
            "DER_Scheduler_Control/OnOff_FSCH03", //
            "DER_Scheduler_Control/OnOff_FSCH04", //
            "DER_Scheduler_Control/OnOff_FSCH05", //
            "DER_Scheduler_Control/OnOff_FSCH06", //
            "DER_Scheduler_Control/OnOff_FSCH07", //
            "DER_Scheduler_Control/OnOff_FSCH08", //
            "DER_Scheduler_Control/OnOff_FSCH09", //
            "DER_Scheduler_Control/OnOff_FSCH10");
}
