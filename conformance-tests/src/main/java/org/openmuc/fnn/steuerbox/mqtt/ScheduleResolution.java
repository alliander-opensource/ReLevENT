package org.openmuc.fnn.steuerbox.mqtt;

import java.time.Duration;
import java.util.Arrays;
import java.util.stream.Collectors;

/**
 * Schedule resolutions as supported by HEDERA API.
 * <p>
 * At the state of implementation, this only covered only 5 minute and 15 minute resolutions.
 */
public enum ScheduleResolution {
    FIVE_MINUTES(Duration.ofMinutes(5)),
    FIFTEEN_MINUTES(Duration.ofMinutes(15));

    private final Duration duration;

    ScheduleResolution(Duration duration) {
        this.duration = duration;
    }

    static ScheduleResolution from(Duration duration) {
        return Arrays.stream(ScheduleResolution.values())
                .filter(rs -> rs.duration.equals(duration))
                .findFirst()
                .orElseThrow(() -> new IllegalArgumentException(
                        "Unable to map " + duration + " to a schedule resolution. Valid values are: " + Arrays.stream(
                                values()).map(ScheduleResolution::name).collect(Collectors.toList())));
    }

    public Duration getAsDuration() {
        return duration;
    }
}
