package de.fhg.ise.gateway.interfaces.hedera;

import io.swagger.client.model.Period;

import java.time.Duration;
import java.util.Arrays;

@SuppressWarnings("unused")
public enum HederaScheduleInterval {
    FIVE_MINUTES(Period.ResolutionEnum.PT5M, Duration.ofMinutes(5)),
    FIFTEEN_MINUTES(Period.ResolutionEnum.PT15M, Duration.ofMinutes(15));

    private final Period.ResolutionEnum mapping;
    private final Duration duration;

    HederaScheduleInterval(Period.ResolutionEnum mapping, Duration duration) {
        this.mapping = mapping;
        this.duration = duration;
    }

    public static HederaScheduleInterval from(Period.ResolutionEnum resolution) {
        return Arrays.stream(HederaScheduleInterval.values())
                .filter(interval -> interval.mapping.equals(resolution))
                .findFirst()
                .orElseThrow(() -> new RuntimeException("Not a mapped resolution:" + resolution));
    }

    Period.ResolutionEnum getAsResolutionEnum() {
        return this.mapping;
    }

    public Duration getAsDuration() {
        return duration;
    }

}
