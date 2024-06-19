package org.openmuc.fnn.steuerbox;

import com.beanit.iec61850bean.ModelNode;
import com.beanit.iec61850bean.ServiceError;
import de.fhg.ise.testtool.utils.annotations.label.Description;
import io.reactivex.Single;
import org.junit.jupiter.api.Test;
import org.openmuc.fnn.steuerbox.mqtt.Command;
import org.openmuc.fnn.steuerbox.mqtt.Schedule;
import org.openmuc.fnn.steuerbox.scheduling.PreparedSchedule;
import org.openmuc.fnn.steuerbox.testutils.MqttUtility;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import static java.time.Duration.ofSeconds;
import static java.time.temporal.ChronoUnit.HOURS;
import static java.time.temporal.ChronoUnit.SECONDS;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;

/**
 * Tests covering the (MQTT) interface to a local EMS to request new schedules, the forwarding of schedules and
 * commands.
 * <p>
 * These tests are tailored and limited to max power schedules, in line with the default FLEDGE settings.
 */
public class EmsInterfacingTests extends AllianderBaseTest {

    private static final Logger log = LoggerFactory.getLogger(EmsInterfacingTests.class);

    final int defaultScheduleValue;
    final MqttUtility util;

    public EmsInterfacingTests() throws ExecutionException, InterruptedException, ServiceError, IOException,
            IEC61850MissconfiguredException {
        defaultScheduleValue = dut.readConstantValueFromSysResScheduleFromModelNode(
                dut.maxPowerSchedules.getValueAccess(), dut.maxPowerSchedules.getReserveSchedule()).intValue();
        util = new MqttUtility();
    }

    @Description("Tests that a schedule transmitted via MQTT will be forwarded to the IEC 61850 server. "
            + "Makes sure that schedule values and interval are maintained. Schedule priority is fixed to 20.")
    @Test
    void schedulesAreForwardedAsIEC61850() throws ServiceError, IOException, InterruptedException {
        final Instant expectedStart = Instant.now().plus(Duration.ofMinutes(5)).truncatedTo(SECONDS);
        final int scheduleNumber = 1;
        List<Number> scheduleValues = Arrays.asList(42, 1337);
        Duration scheduleInterval = Duration.ofMinutes(5);

        PreparedSchedule mqttSchedule = dut.maxPowerSchedules.prepareSchedule(scheduleValues, scheduleNumber,
                scheduleInterval, expectedStart, 200);
        util.writeAndEnableSchedule(mqttSchedule);

        Thread.sleep(1_000);

        assertEquals(expectedStart, dut.getScheduleStart(dut.maxPowerSchedules.getScheduleName(scheduleNumber)));

        ModelNode node = dut.getNodeWithValues(dut.maxPowerSchedules.getScheduleName(scheduleNumber));
        String node1 = node.getChild("ValASG001").getBasicDataAttributes().get(0).getValueString(); // sollte 42 sein
        assertEquals(42, Float.valueOf(node1));
        String node2 = node.getChild("ValASG002").getBasicDataAttributes().get(0).getValueString(); // sollte 1337 sein
        assertEquals(1337, Float.valueOf(node2));
        String numberOfScheduleValues = node.getChild("NumEntr")
                .getBasicDataAttributes()
                .get(0)
                .getValueString(); // sollte 2 sein!
        assertEquals(2, Integer.valueOf(numberOfScheduleValues));

        String schdIntv = node.getChild("SchdIntv")
                .getBasicDataAttributes()
                .get(0)
                .getValueString(); // sollte 5min sein
        assertEquals(scheduleInterval.toSeconds(), Long.valueOf(schdIntv));

        String schdPrio = node.getChild("SchdPrio")
                .getBasicDataAttributes()
                .get(0)
                .getValueString(); // sollte 20 sein, ist im hedera client hard gekodet!
        assertEquals(20, Long.valueOf(schdPrio)); // given prio is ignored when transmitting via mqtt!
    }

    @Description("Tests that a schedule that covers an entire day can be created and transmitted via MQTT interface.")
    @Test
    void oneDayCanBeCovered() throws ServiceError, IOException, InterruptedException {

        final Instant expectedStart = Instant.now().plus(Duration.ofMinutes(5)).truncatedTo(SECONDS);
        final int scheduleNumber = 1;
        List<Number> scheduleValues = Stream.iterate(1, i -> i + 1)
                .limit(100)
                .map(i -> (Number) i)
                .collect(Collectors.toList());
        Duration scheduleInterval = Duration.ofMinutes(15);

        PreparedSchedule mqttSchedule = dut.maxPowerSchedules.prepareSchedule(scheduleValues, scheduleNumber,
                scheduleInterval, expectedStart, 200);
        util.writeAndEnableSchedule(mqttSchedule);

        Thread.sleep(1_000);

        ModelNode node = dut.getNodeWithValues(dut.maxPowerSchedules.getScheduleName(scheduleNumber));

        int valuesChecked = 0;
        for (int i = 1; i <= 100; i++) {
            String valueAccessString = String.format("ValASG%03d", i);
            assertEquals(i,
                    Float.valueOf(node.getChild(valueAccessString).getBasicDataAttributes().get(0).getValueString()));
            log.debug("Value of node {} has expected value {}", valueAccessString, i);
            valuesChecked++;
        }
        assertEquals(100, valuesChecked);

        String numberOfScheduleValues = node.getChild("NumEntr").getBasicDataAttributes().get(0).getValueString();
        assertEquals(100, Integer.valueOf(numberOfScheduleValues));
        log.info("Schedule value size set to {} as expected", numberOfScheduleValues);

        String schdIntv = node.getChild("SchdIntv")
                .getBasicDataAttributes()
                .get(0)
                .getValueString(); // sollte 5min sein
        assertEquals(Duration.ofMinutes(15).toSeconds(), Long.valueOf(schdIntv));

        Duration maximalScheduleDuration = Duration.ofMinutes(15).multipliedBy(100);
        assertFalse(maximalScheduleDuration.minus(Duration.ofDays(1)).isNegative());
        log.info("Maximal schedule duration of {} is larger than 1 day (24h)", maximalScheduleDuration);
    }

    @Description("This test covers: Schedules are published via MQTT. " + "Publishing interval is 5 seconds. "
            + "Format of the MQTT JSON payload is as expected. "
            + "Schedule values and schedule timestamps have the expected values. "
            + "Does not check 'FractionOfSecond' of published schedule.")
    @Test
    void schedulesAreForwardedInExpectedFormat() throws ServiceError, IOException, InterruptedException {

        Instant iec61850ScheduleStart = Instant.now().plus(3, HOURS).truncatedTo(SECONDS);
        List<Number> iec61850ScheduleValues = Arrays.asList(1d, 42d, 1337d);
        Duration iec61850ScheduleInterval = ofSeconds(1);
        PreparedSchedule iec61850schedule = dut.maxPowerSchedules.prepareSchedule(//
                iec61850ScheduleValues,//
                1,// fixed for the test: fledge setup tested and set up for this schedule
                iec61850ScheduleInterval,//
                iec61850ScheduleStart,//
                200);// schedule prio is arbitrary, needs to be something larger than default prio
        dut.writeAndEnableSchedule(iec61850schedule);
        Single<List<MqttUtility.Timestamped<Schedule>>> result = util.fetchScheduleUpdate(1,
                Duration.between(Instant.now(), iec61850ScheduleStart).plus(iec61850ScheduleInterval.multipliedBy(2)));
        log.debug("Schedule written, awaiting publishing of result schedule (happens every 5 seconds))");
        List<MqttUtility.Timestamped<Schedule>> timestampedObjects = result.blockingGet();

        Schedule sched = timestampedObjects.get(0).getObject();

        log.debug("Checking schedule {}", sched);
        assertEquals(defaultScheduleValue, sched.getScheduleEntries().get(0).controlValue);

        assertEquals(1d, sched.getScheduleEntries().get(1).controlValue);
        assertEquals(iec61850ScheduleValues.get(0), sched.getScheduleEntries().get(1).controlValue);
        assertEquals(iec61850ScheduleStart.getEpochSecond(), sched.getScheduleEntries().get(1).startEpochSecond);

        assertEquals(42d, sched.getScheduleEntries().get(2).controlValue);
        assertEquals(iec61850ScheduleValues.get(1), sched.getScheduleEntries().get(2).controlValue);
        assertEquals(iec61850ScheduleStart.plus(iec61850ScheduleInterval).getEpochSecond(),
                sched.getScheduleEntries().get(2).startEpochSecond);

        assertEquals(1337d, sched.getScheduleEntries().get(3).controlValue);
        assertEquals(iec61850ScheduleValues.get(2), sched.getScheduleEntries().get(3).controlValue);
        assertEquals(iec61850ScheduleStart.plus(iec61850ScheduleInterval.multipliedBy(2)).getEpochSecond(),
                sched.getScheduleEntries().get(3).startEpochSecond);

        assertEquals(defaultScheduleValue, sched.getScheduleEntries().get(4).controlValue);
        assertEquals(iec61850ScheduleStart.plus(iec61850ScheduleInterval.multipliedBy(3)).getEpochSecond(),
                sched.getScheduleEntries().get(4).startEpochSecond);
    }

    @Description("Schedules are published via MQTT. Publishing interval is 5 seconds. Schedule values and schedule timestamps have the expected values which do not change over time. Does not check 'FractionOfSecond' of published schedule.")
    @Test
    void schedulesArePublishedEvery5Seconds_valuesDoNotDiffer() throws ServiceError, IOException, InterruptedException {

        Instant iec61850ScheduleStart = Instant.now().plus(3, HOURS).truncatedTo(SECONDS);
        List<Number> iec61850ScheduleValues = Arrays.asList(1d, 42d, 1337d);
        Duration iec61850ScheduleInterval = ofSeconds(1);
        PreparedSchedule iec61850schedule = dut.maxPowerSchedules.prepareSchedule(//
                iec61850ScheduleValues,//
                1,// fixed for the test: fledge setup tested and set up for this schedule
                iec61850ScheduleInterval,//
                iec61850ScheduleStart,//
                200);// schedule prio is arbitrary, needs to be something larger than default prio
        dut.writeAndEnableSchedule(iec61850schedule);
        Single<List<MqttUtility.Timestamped<Schedule>>> result = util.fetchScheduleUpdate(3,
                Duration.between(Instant.now(), iec61850ScheduleStart).plus(iec61850ScheduleInterval.multipliedBy(4)));
        log.debug("Schedule written, awaiting publishing of result schedule (happens every 5 seconds))");
        List<MqttUtility.Timestamped<Schedule>> timestampedObjects = result.blockingGet();
        assertEquals(3, timestampedObjects.size());

        Instant firstTimestamp = timestampedObjects.get(0).getTimestamp().truncatedTo(SECONDS);
        Schedule firstSchedule = timestampedObjects.get(0).getObject();
        Instant secondTimestamp = timestampedObjects.get(1).getTimestamp().truncatedTo(SECONDS);
        Schedule secondSchedule = timestampedObjects.get(1).getObject();
        Instant thirdTimestamp = timestampedObjects.get(2).getTimestamp().truncatedTo(SECONDS);
        Schedule thirdSchedule = timestampedObjects.get(2).getObject();

        assertEquals(firstTimestamp.plus(5, SECONDS), secondTimestamp);
        assertEquals(firstTimestamp.plus(10, SECONDS), thirdTimestamp);

        for (Schedule sched : Arrays.asList(firstSchedule, secondSchedule, thirdSchedule)) {
            log.debug("Checking schedule {}", sched);
            assertEquals(defaultScheduleValue, sched.getScheduleEntries().get(0).controlValue);

            assertEquals(1d, sched.getScheduleEntries().get(1).controlValue);
            assertEquals(iec61850ScheduleValues.get(0), sched.getScheduleEntries().get(1).controlValue);
            assertEquals(iec61850ScheduleStart.getEpochSecond(), sched.getScheduleEntries().get(1).startEpochSecond);

            assertEquals(42d, sched.getScheduleEntries().get(2).controlValue);
            assertEquals(iec61850ScheduleValues.get(1), sched.getScheduleEntries().get(2).controlValue);
            assertEquals(iec61850ScheduleStart.plus(iec61850ScheduleInterval).getEpochSecond(),
                    sched.getScheduleEntries().get(2).startEpochSecond);

            assertEquals(1337d, sched.getScheduleEntries().get(3).controlValue);
            assertEquals(iec61850ScheduleValues.get(2), sched.getScheduleEntries().get(3).controlValue);
            assertEquals(iec61850ScheduleStart.plus(iec61850ScheduleInterval.multipliedBy(2)).getEpochSecond(),
                    sched.getScheduleEntries().get(3).startEpochSecond);

            assertEquals(defaultScheduleValue, sched.getScheduleEntries().get(4).controlValue);
            assertEquals(iec61850ScheduleStart.plus(iec61850ScheduleInterval.multipliedBy(3)).getEpochSecond(),
                    sched.getScheduleEntries().get(4).startEpochSecond);
        }
    }

    // TODO: test fails, command publishing works only upon first startup as it seems (MZ: why is that?)
    @Description("Tests that a schedules values are forwarded as a command just in time. Check publishing time, control value and 'SecondsSinceEpoch' payload. Does not check 'FractionOfSecond' of published command.")
    @Test
    void commandsAreForwardedJustOnTime() throws ServiceError, IOException, ExecutionException, InterruptedException {
        Instant iec61850ScheduleStart = Instant.now().plus(5, SECONDS).truncatedTo(SECONDS);
        List<Number> iec61850ScheduleValues = Arrays.asList(1d, 42d, 1337d);
        Duration iec61850ScheduleInterval = ofSeconds(1);

        // set up observation: watch all values from the schedule and changing back to default
        final int commandUpdatesToObserve = iec61850ScheduleValues.size() + 1;
        Single<List<MqttUtility.Timestamped<Command>>> commandUpdates = util.fetchCommandUpdate(commandUpdatesToObserve,
                iec61850ScheduleInterval.multipliedBy(commandUpdatesToObserve)
                        .plus(Duration.between(Instant.now(), iec61850ScheduleStart).plus(ofSeconds(5))));
        // set up execution
        PreparedSchedule iec61850schedule = dut.maxPowerSchedules.prepareSchedule(//
                iec61850ScheduleValues,//
                1,// fixed for the test: fledge setup tested and set up for this schedule
                iec61850ScheduleInterval,//
                iec61850ScheduleStart,//
                200);// schedule prio is arbitrary, needs to be something larger than default prio
        dut.writeAndEnableSchedule(iec61850schedule);

        // wait until command changes are observed
        List<MqttUtility.Timestamped<Command>> commands = commandUpdates.blockingGet();
        // do checks on 4 command updates: check values, publishing time of cmd and "sinceEpoch" of published payload
        assertFalse(commands.isEmpty(),
                "No MQTT commands where published localhost's MQTT broker @ topic " + util.CMD_TOPIC + ".");
        assertEquals(4, commandUpdatesToObserve);
        assertEquals(commandUpdatesToObserve, commands.size());

        MqttUtility.Timestamped<Command> firstCmd = commands.get(0);
        assertEquals(iec61850ScheduleStart, firstCmd.getTimestamp().truncatedTo(SECONDS));
        assertEquals(iec61850ScheduleStart.getEpochSecond(), firstCmd.getObject().epochSecond);
        assertEquals(1d, firstCmd.getObject().controlValue);

        MqttUtility.Timestamped<Command> secondCmd = commands.get(1);
        Instant secondStart = iec61850ScheduleStart.plus(iec61850ScheduleInterval);
        assertEquals(secondStart, secondCmd.getTimestamp().truncatedTo(SECONDS));
        assertEquals(secondStart.getEpochSecond(), secondCmd.getObject().epochSecond);
        assertEquals(42d, secondCmd.getObject().controlValue);

        MqttUtility.Timestamped<Command> thirdCmd = commands.get(2);
        Instant thirdStart = iec61850ScheduleStart.plus(iec61850ScheduleInterval.multipliedBy(2));
        assertEquals(thirdStart, thirdCmd.getTimestamp().truncatedTo(SECONDS));
        assertEquals(thirdStart.getEpochSecond(), thirdCmd.getObject().epochSecond);
        assertEquals(1337d, thirdCmd.getObject().controlValue);

        MqttUtility.Timestamped<Command> fourthCmd = commands.get(3);
        Instant fourthStart = iec61850ScheduleStart.plus(iec61850ScheduleInterval.multipliedBy(3));
        assertEquals(fourthStart, fourthCmd.getTimestamp().truncatedTo(SECONDS));
        assertEquals(fourthStart.getEpochSecond(), fourthCmd.getObject().epochSecond);
        assertEquals(defaultScheduleValue, commands.get(3).getObject().controlValue);
    }
}
