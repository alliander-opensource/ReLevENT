package de.fhg.ise.gateway.interfaces.ems.DTO;

import de.fhg.ise.gateway.interfaces.hedera.HederaScheduleInterval;

import java.time.Instant;
import java.util.List;

/**
 * Holds the bare minimum information of a schedule
 */
public interface Schedule {
    List<Double> getValues();

    HederaScheduleInterval getInterval();

    Instant getStart();
}
