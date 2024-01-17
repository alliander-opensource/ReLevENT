package de.fhg.ise.gateway.interfaces.hedera;

import io.swagger.client.model.Point;
import io.swagger.client.model.Schedule;
import io.swagger.client.model.ScheduleGetResponse;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.threeten.bp.OffsetDateTime;

import java.time.Instant;
import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;

public class HederaSchedule {

    private static final Logger log = LoggerFactory.getLogger(HederaSchedule.class);

    private final ScheduleGetResponse response;

    HederaSchedule(ScheduleGetResponse response) {
        this.response = response;
    }

    public List<Double> getValues() {
        return response.getSchedule()
                .getRegisteredInterTies()
                .get(0)
                .getTimeSeries()
                .getPoints()
                .stream()
                .map(Point::getQuantity)
                .collect(Collectors.toUnmodifiableList());
    }

    public Schedule.AtTypeEnum getStatus() {
        return response.getSchedule().getAtType();//
    }

    public HederaScheduleInterval getInterval() {
        return HederaScheduleInterval.from(
                response.getSchedule().getRegisteredInterTies().get(0).getTimeSeries().getPeriod().getResolution());
    }

    ScheduleGetResponse getRawResponse() {
        return response;
    }

    public String getStatusMessage() {
        return response.getSchedule().getMessage();
    }

    public Instant getStart() {
        OffsetDateTime start = response.getSchedule()
                .getRegisteredInterTies()
                .get(0)
                .getTimeSeries()
                .getPeriod()
                .getTimeInterval()
                .getStart();
        long epochSecond = start.toEpochSecond();
        long nanos = start.getNano();
        long epochMillis = epochSecond * 1000 + nanos / 1000 / 1000;
        Instant startInstant = Instant.ofEpochMilli(epochMillis);
        log.trace("Transformed {} into {}", start, startInstant);
        return startInstant;
    }

    public UUID getScheduleUuid() {
        // respose.getMRID() holds some other MRID, we do not to go for .getSchedule().getMRID() here!
        return response.getSchedule().getMRID();
    }
}
