package de.fhg.ise.gateway.interfaces.hedera;

import de.fhg.ise.IEC61850.client.models.AllianderDER;
import de.fhg.ise.gateway.HederaException;
import de.fhg.ise.gateway.configuration.Settings;
import de.fhg.ise.gateway.interfaces.GridConnectionExtension;
import de.fhg.ise.gateway.interfaces.ems.DTO.ExtensionRequest;
import io.swagger.client.ApiException;
import io.swagger.client.model.Schedule;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.ConnectException;
import java.net.UnknownHostException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Optional;
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
    // TODO: from settings!
    private final int scheduleNumber = 1;
    private final int prio = 20;
    private final int divisor = 60;

    private Optional<GridConnectionExtension> lastRequest = Optional.empty();

    public HederaRefresh(HederaApi api, AllianderDER der, Settings settings) {
        this.api = api;
        this.settings = settings;
        this.der = der;
    }

    // TODO: make non-blocking!
    public void newRequestFromEms(ExtensionRequest req)  {
        log.info("Got new request {}", req);

        log.debug("Cleaning up old schedules at HEDERA");
        AtomicInteger cnt = new AtomicInteger(0);

        Collection<Schedule.AtTypeEnum> scheduleStatusesThatMayInterfereWithNewSchedules = Arrays.asList(Schedule.AtTypeEnum.ACCEPTED, Schedule.AtTypeEnum.PENDING,
                Schedule.AtTypeEnum.DECLINED);
        try {
            api.getScheduleMRIDsOfAllExistingSchedules().forEach(schedule -> {
                if(scheduleStatusesThatMayInterfereWithNewSchedules.contains(schedule.status)) {

                    try {
                        api.deleteSchedule(schedule.mRID);
                        cnt.incrementAndGet();
                    } catch (Exception e) {
                        log.warn("Unable to delete schedule with mrid={} (state={}). This might lead to issues.",
                                schedule.mRID, schedule.status);
                    }
                }
                else  {
                    log.trace("Ignoring schedule {}: status will not interfere with creation of new schedules",schedule);
                }
            });
        }
        catch (Exception e){
            throw  new RuntimeException(e);
        }
        log.info("Cleaned up {} old schedules at HEDERA", cnt.get());

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
        if (hederaSchedule == null) {
            log.debug("Skipping to connect to DER: schedule calculation failed at HEDERA");
        }
        else {

            List<Number> values = hederaSchedule.getValues()
                    .stream()
                    .collect(Collectors.toList()); // List<Double> -> List<Number> seems to need that
            try {
                this.der.writeAndEnableSchedule(der.maxPowerSchedules.prepareSchedule(values, scheduleNumber,
                        hederaSchedule.getInterval().getAsDuration().dividedBy(divisor), hederaSchedule.getStart(),
                        prio));
                log.info("Transmitted schedule to DER. Schedule will start to run in @ {}", hederaSchedule.getStart());
            } catch (Exception e) {
                log.warn(
                        "Unable to forward schedule to DER @ {}:{}. Reason: {}:{}. Trying to solve the problem by a reconnect.",
                        der.host, der.port, e.getClass(), e.getMessage());

                try {
                    this.der = this.der.reconnect();
                    log.info("Reconnected successfully.");
                    this.der.writeAndEnableSchedule(der.maxPowerSchedules.prepareSchedule(values, scheduleNumber,
                            hederaSchedule.getInterval().getAsDuration().dividedBy(divisor), hederaSchedule.getStart(),
                            prio));
                    log.info("Transmitted schedule to DER. Schedule will start to run in @ {}",
                            hederaSchedule.getStart());
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
}
