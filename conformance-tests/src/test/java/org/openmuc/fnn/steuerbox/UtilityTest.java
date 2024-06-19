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
import org.openmuc.fnn.steuerbox.mqtt.Command;
import org.openmuc.fnn.steuerbox.mqtt.Schedule;
import org.openmuc.fnn.steuerbox.mqtt.Schedule.ScheduleEntry;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.util.Arrays;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.openmuc.fnn.steuerbox.AllianderTests.extractNumberFromLastNodeName;
import static org.openmuc.fnn.steuerbox.mqtt.ParsingUtil.scheduleToJson;

public class UtilityTest {

    @Test
    void numberExtractionWorks() {
        assertEquals(extractNumberFromLastNodeName("device4.schedule76.whatever.in.between.number123").get().intValue(),
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

        assertEquals(expectedValue,
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
        assertEquals(
                "Expected exactly 1 power value in system reserve Schedule but got 2. Please reconfigure the device.",
                caughtException.getMessage());

    }

    @Test
    void removeingNumbersWorks() {
        assertEquals("asd", AllianderBaseTest.removeNumbers("asd123"));
        assertEquals(null, AllianderBaseTest.removeNumbers(null));
        assertEquals("", AllianderBaseTest.removeNumbers("42"));
        assertEquals("no number", AllianderBaseTest.removeNumbers("no number"));
    }

    @Test
    void jsonCreationWorksAsExpected() {
        String expectedJson = "{\"skipHedera\":true,\"direction\":\"IMPORT\",\"start\":{\"seconds\":1717600722,\"nanos\":0},\"resolution\":\"FIFTEEN_MINUTES\",\"values\":[42,1337]}";
        Instant start = Instant.ofEpochSecond(1717600722);
        assertEquals(expectedJson, scheduleToJson(Arrays.asList(42, 1337), Duration.ofMinutes(15), start));
    }

    @Test
    void commandIsParsedCorrectly() throws ParserConfigurationException, IOException, SAXException {
        String command = "{\"operation\": \"PivotCommand\", \"parameters\": [{\"name\": \"PIVOTTC\", \"value\": {\"GTIC\": {\"ComingFrom\": \"iec61850\", \"ApcTyp\": {\"q\": {\"test\": 0}, \"t\": {\"SecondSinceEpoch\": 1717603268, \"FractionOfSecond\": 738197}, \"ctlVal\": 123.456}, \"Identifier\": \"TS1\"}}}]}";
        assertEquals(new Command(1717603268, 123.456), Command.fromJsonString(command));
    }

    @Test
    void scheduleIsParsedCorrectly() {
        String schedule = "{\"operation\": \"PivotSchedule\", \"parameters\": [{\"name\": \"PIVOTTC\", \"value\": {\"GTIC\": {\"ComingFrom\": \"iec61850\", \"ApcTyp\": {\"q\": {\"test\": 0}, \"t\": {\"SecondSinceEpoch\": 1718017399, \"FractionOfSecond\": 6459228}, \"ctlVal\": 10.0}, \"Identifier\": \"TS1\"}}}, {\"name\": \"PIVOTTC\", \"value\": {\"GTIC\": {\"ComingFrom\": \"iec61850\", \"ApcTyp\": {\"q\": {\"test\": 0}, \"t\": {\"SecondSinceEpoch\": 1718017401, \"FractionOfSecond\": 10921967}, \"ctlVal\": 20.0}, \"Identifier\": \"TS1\"}}}, {\"name\": \"PIVOTTC\", \"value\": {\"GTIC\": {\"ComingFrom\": \"iec61850\", \"ApcTyp\": {\"q\": {\"test\": 0}, \"t\": {\"SecondSinceEpoch\": 1718017406, \"FractionOfSecond\": 10921967}, \"ctlVal\": 30.0}, \"Identifier\": \"TS1\"}}}, {\"name\": \"PIVOTTC\", \"value\": {\"GTIC\": {\"ComingFrom\": \"iec61850\", \"ApcTyp\": {\"q\": {\"test\": 0}, \"t\": {\"SecondSinceEpoch\": 1718017411, \"FractionOfSecond\": 10921967}, \"ctlVal\": 40.0}, \"Identifier\": \"TS1\"}}}, {\"name\": \"PIVOTTC\", \"value\": {\"GTIC\": {\"ComingFrom\": \"iec61850\", \"ApcTyp\": {\"q\": {\"test\": 0}, \"t\": {\"SecondSinceEpoch\": 1718017416, \"FractionOfSecond\": 10921967}, \"ctlVal\": 0.0}, \"Identifier\": \"TS1\"}}}]}";
        Schedule expectedSchedule = new Schedule( //
                new ScheduleEntry(1718017399, 10), //
                new ScheduleEntry(1718017401, 20), //
                new ScheduleEntry(1718017406, 30), //
                new ScheduleEntry(1718017411, 40), //
                new ScheduleEntry(1718017416, 0)); //
        assertEquals(expectedSchedule, Schedule.parse(schedule));
    }
}
