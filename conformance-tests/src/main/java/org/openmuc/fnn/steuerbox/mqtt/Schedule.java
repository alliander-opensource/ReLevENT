package org.openmuc.fnn.steuerbox.mqtt;

import java.time.Instant;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import static org.openmuc.fnn.steuerbox.mqtt.ParsingUtil.parseMqttMessages;

/**
 * A command payload representing a result schedule intended for the EMS to facilitate optimization of the energy
 * system.
 * <p>
 * Only relevant parameters will be parsed, most of the JSON fields are FLEDGE defaults and not relevant for the
 * ReLevENT use cases.
 */
public class Schedule {
    private List<ScheduleEntry> scheduleEntries = new LinkedList<>();

    public Schedule(ScheduleEntry... entries) {
        for (ScheduleEntry entry : entries) {
            this.scheduleEntries.add(entry);
        }
    }

    public List<ScheduleEntry> getScheduleEntries() {
        return Collections.unmodifiableList(this.scheduleEntries);
    }

    public static Schedule parse(String scheduleString) {
        List<Map.Entry<Instant, Double>> entries = parseMqttMessages(scheduleString);
        Schedule schedule = new Schedule();
        entries.forEach(
                e -> schedule.scheduleEntries.add(new ScheduleEntry(e.getKey().getEpochSecond(), e.getValue())));
        return schedule;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o)
            return true;
        if (o == null || getClass() != o.getClass())
            return false;
        Schedule schedule = (Schedule) o;
        return Objects.equals(scheduleEntries, schedule.scheduleEntries);
    }

    @Override
    public int hashCode() {
        return Objects.hashCode(scheduleEntries);
    }

    public static class ScheduleEntry {
        public final long startEpochSecond;
        public final double controlValue;

        public ScheduleEntry(long startEpochSecond, double controlValue) {
            this.startEpochSecond = startEpochSecond;
            this.controlValue = controlValue;
        }

        @Override
        public String toString() {
            return "ScheduleEntry{" + "startEpochSecond=" + startEpochSecond + ", controlValue=" + controlValue + '}';
        }

        @Override
        public boolean equals(Object o) {
            if (this == o)
                return true;
            if (o == null || getClass() != o.getClass())
                return false;
            ScheduleEntry that = (ScheduleEntry) o;
            return startEpochSecond == that.startEpochSecond && Double.compare(controlValue, that.controlValue) == 0;
        }

        @Override
        public int hashCode() {
            return Objects.hash(startEpochSecond, controlValue);
        }
    }

    @Override
    public String toString() {
        return "Schedule{" + "scheduleEntries=" + scheduleEntries + '}';
    }
}
