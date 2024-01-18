package de.fhg.ise.gateway.interfaces.ems.DTO;

import de.fhg.ise.gateway.Context;
import de.fhg.ise.gateway.HederaException;
import de.fhg.ise.gateway.configuration.Settings;
import de.fhg.ise.gateway.interfaces.hedera.HederaApi;
import de.fhg.ise.gateway.interfaces.hedera.HederaDirection;
import de.fhg.ise.gateway.interfaces.hedera.HederaSchedule;
import de.fhg.ise.gateway.interfaces.hedera.HederaScheduleInterval;

import java.time.Duration;
import java.time.Instant;
import java.util.List;

public class ExtensionRequest {
    HederaDirection direction;
    Instant start;
    HederaScheduleInterval resolution;
    List<Double> values;

    public ExtensionRequest() {
        // start schedule 3 min in the future per default
        this.start = Instant.now().plus(Duration.ofMinutes(3));
    }

    public HederaDirection getDirection() {
        return direction;
    }

    public void setDirection(HederaDirection direction) {
        this.direction = direction;
    }

    public Instant getStart() {
        return start;
    }

    public void setStart(Instant start) {
        this.start = start;
    }

    public HederaScheduleInterval getResolution() {
        return resolution;
    }

    public void setResolution(HederaScheduleInterval resolution) {
        this.resolution = resolution;
    }

    public List<Double> getValues() {
        return values;
    }

    public void setValues(List<Double> values) {
        this.values = values;
    }

    public String toJson() {
        return Context.GSON.toJson(this);
    }

    public static ExtensionRequest fromJson(String json) {
        return Context.GSON.fromJson(json, ExtensionRequest.class);
    }

    @Override
    public String toString() {
        return "ExtensionRequest{" + "direction=" + direction + ", start=" + start + ", resolution=" + resolution
                + ", values=" + values + '}';
    }

    public HederaSchedule requestExtensionAwaitCalculation(HederaApi api, Settings settings) throws HederaException {
        return api.requestExtensionAwaitCalculation(this.getStart(), this.getResolution(), this.getValues(),
                this.getDirection(), settings);
    }
}
