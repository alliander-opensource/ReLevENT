package de.fhg.ise.gateway.interfaces.hedera;

import io.swagger.client.model.Schedule;

import java.time.Instant;
import java.util.List;

public interface IHederaSchedule {
    List<Double> getValues();
    HederaScheduleInterval getInterval();
    Instant getStart();
}
