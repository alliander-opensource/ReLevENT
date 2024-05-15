package de.fhg.ise.gateway.interfaces.hedera;

import de.fhg.ise.IEC61850.client.models.AllianderDER;
import de.fhg.ise.gateway.HederaException;
import de.fhg.ise.gateway.configuration.Settings;
import de.fhg.ise.gateway.interfaces.ems.DTO.ExtensionRequest;
import de.fhg.ise.gateway.interfaces.ems.DTO.Schedule;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.ConnectException;
import java.net.UnknownHostException;
import java.time.Instant;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Collectors;

/**
 * Takes care of refreshing schedules at hedera.
 */
public class HederaRefresh {

    private static final Logger log = LoggerFactory.getLogger(HederaRefresh.class);
    private final Settings settings;
    private AllianderDER der;
    private final HederaApi api;
    // TODO: read scheduleNumber and prio from settings!
    private final int scheduleNumber = 1;
    private final int prio = 20;

    public HederaRefresh(HederaApi api, AllianderDER der, Settings settings) {
        this.api = api;
        this.settings = settings;
        this.der = der;
    }

    // TODO: make non-blocking!
    public void newRequestFromEms(ExtensionRequest req) {
        log.info("Got new request {}", req);

        final Schedule schedule;
        if (req.getSkipHedera()) {
            log.warn("Skipping HEDERA. Directly transmitting schedule to DER");
            schedule = new Schedule() {
                @Override
                public List<Double> getValues() {
                    return req.getValues();
                }

                @Override
                public HederaScheduleInterval getInterval() {
                    return req.getResolution();
                }

                @Override
                public Instant getStart() {
                    return req.getStart();
                }
            };
        }
        else {
            schedule = getScheduleConfirmationAtHedera(req);
        }
        if (schedule == null) {
            log.debug("Skipping to connect to DER: schedule calculation failed at HEDERA");
        }
        else {

            List<Number> values = schedule.getValues()
                    .stream()
                    .collect(Collectors.toList()); // List<Double> -> List<Number> seems to need that
            try {
                this.der.writeAndEnableSchedule(der.maxPowerSchedules.prepareSchedule(values, scheduleNumber,
                        schedule.getInterval().getAsDuration(), schedule.getStart(), prio));
                log.info("Transmitted schedule to DER. Schedule will start to run in @ {}", schedule.getStart());
            } catch (Exception e) {
                log.warn(
                        "Unable to forward schedule to DER @ {}:{}. Reason: {}:{}. Trying to solve the problem by a reconnect.",
                        der.host, der.port, e.getClass(), e.getMessage());

                try {
                    this.der = this.der.reconnect();
                    log.info("Reconnected successfully.");
                    this.der.writeAndEnableSchedule(der.maxPowerSchedules.prepareSchedule(values, scheduleNumber,
                            schedule.getInterval().getAsDuration(), schedule.getStart(), prio));
                    log.info("Transmitted schedule to DER. Schedule will start to run in @ {}", schedule.getStart());
                } catch (UnknownHostException | ConnectException ex) {
                    log.error("Unable to reconnect to host '{}': {}:{}. Giving up.", der.host, ex.getClass(),
                            ex.getMessage());
                } catch (Exception ex) {
                    log.error("Unable to reconnect and forward schedule to DER @ {}:{}. Giving up.", der.host, der.port,
                            ex);
                }
            }
        }

        // TODO: start a refresh here
        // TODO make use of recommended refresh time (shall be in response for the requests)
    }

    private HederaSchedule getScheduleConfirmationAtHedera(ExtensionRequest req) {
        log.debug("Cleaning up old schedules at HEDERA");
        AtomicInteger cnt = new AtomicInteger(0);

        cleanUpAllExistingSchedulesAtHedera(cnt);

        HederaSchedule hederaSchedule = null;
        try {
            hederaSchedule = req.requestExtensionAwaitCalculation(this.api, this.settings);
        } catch (HederaException e) {
            log.error("Error in schedule {}. Error message by HEDERA: {}", req, e.getMessage());
        } catch (Exception e) {
            log.warn("Unable to create schedule at HEDERA. Retrying. Reason: {}:{}", e.getClass(), e.getMessage());
            try {
                hederaSchedule = req.requestExtensionAwaitCalculation(this.api, this.settings);
            } catch (Exception e2) {
                log.error("Again unable to create schedule at HEDERA. Input extension request: '{}'. Giving up.", req,
                        e2);
            }
        }
        return hederaSchedule;
    }

    private void cleanUpAllExistingSchedulesAtHedera(AtomicInteger cnt) {
        try {
            api.getScheduleMRIDsOfAllExistingSchedulesThatMayInterfereWithNewSchedules().forEach(schedule -> {
                try {
                    api.deleteSchedule(schedule.mRID);
                    cnt.incrementAndGet();
                } catch (Exception e) {
                    log.warn("Unable to delete schedule with mrid={} (state={}). This might lead to issues.",
                            schedule.mRID, schedule.status);
                }
            });
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        log.info("Cleaned up {} old schedules at HEDERA", cnt.get());
    }
}
