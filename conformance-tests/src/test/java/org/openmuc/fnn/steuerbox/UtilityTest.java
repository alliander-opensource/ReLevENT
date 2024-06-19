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

import com.beanit.iec61850bean.ServiceError;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.function.Executable;
import org.openmuc.fnn.steuerbox.models.AllianderDER;

import java.io.IOException;

import static org.openmuc.fnn.steuerbox.AllianderTests.extractNumberFromLastNodeName;

public class UtilityTest {

    @Test
    void numberExtractionWorks() {
        Assertions.assertEquals(
                extractNumberFromLastNodeName("device4.schedule76.whatever.in.between.number123").get().intValue(),
                123);
    }

    @Test
    void numberExtractionDoesNotThrow() {
        Assertions.assertTrue(extractNumberFromLastNodeName("").isEmpty());
        Assertions.assertTrue(extractNumberFromLastNodeName("nonumberinhere").isEmpty());
    }

    @Test
    public void testReserveScheduleWorksAsExpected() throws ServiceError, IOException, IEC61850MissconfiguredException {
        float expectedValue = 123f;

        AllianderDER dut = AllianderDER.getWithDefaultSettings();
        String reserveScheduleName = dut.powerSchedules.getReserveSchedule();
        // set number of values to 1
        dut.setDataValues(reserveScheduleName + ".NumEntr.setVal", null, "1");
        // write expected values
        String valueBasicDataAttribute = String.format("%s.ValASG001.setMag.f", reserveScheduleName);
        dut.setDataValues(valueBasicDataAttribute, null, Float.toString(expectedValue));

        Assertions.assertEquals(expectedValue,
                (float) dut.readConstantValueFromSysResScheduleFromModelNode(dut.powerSchedules.getValueAccess(),
                        reserveScheduleName));
    }

    @Test
    void test_ReadingReserveScheduleFails_When2ComposedFromMoreThan1Element() throws ServiceError, IOException {

        float firstValue = 10f;
        float secondValue = 20f;

        AllianderDER dut = AllianderDER.getWithDefaultSettings();
        String reserveScheduleName = dut.powerSchedules.getReserveSchedule();

        //write two values in reserveSchedule
        String valueBasicDataAttribute1 = String.format("%s.ValASG001.setMag.f", reserveScheduleName);
        String valueBasicDataAttribute2 = String.format("%s.ValASG002.setMag.f", reserveScheduleName);

        dut.setDataValues(valueBasicDataAttribute1, null, Float.toString(firstValue));
        dut.setDataValues(valueBasicDataAttribute2, null, Float.toString(secondValue));
        // set number of values (NumEntr) to 2
        dut.setDataValues(reserveScheduleName + ".NumEntr.setVal", null, "2");

        Executable executableThatShouldThrow = () -> dut.readConstantValueFromSysResScheduleFromModelNode(
                dut.powerSchedules.getValueAccess(), reserveScheduleName);
        IEC61850MissconfiguredException caughtException = Assertions.assertThrows(IEC61850MissconfiguredException.class,
                executableThatShouldThrow);
        Assertions.assertEquals(
                "Expected exactly 1 power value in system reserve Schedule but got 2. Please reconfigure the device.",
                caughtException.getMessage());

    }

    @Test
    void removeingNumbersWorks() {
        Assertions.assertEquals("asd", AllianderBaseTest.removeNumbers("asd123"));
        Assertions.assertEquals(null, AllianderBaseTest.removeNumbers(null));
        Assertions.assertEquals("", AllianderBaseTest.removeNumbers("42"));
        Assertions.assertEquals("no number", AllianderBaseTest.removeNumbers("no number"));
    }
}
