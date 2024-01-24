package de.fhg.ise.gateway.interfaces.hedera;

import java.time.Instant;
import java.util.List;

public interface IHederaSchedule {
    List<Double> getValues();

    HederaScheduleInterval getInterval();

    Instant getStart();
}
