package de.fhg.ise.gateway.interfaces.hedera;

import com.beanit.iec61850bean.ServiceError;
import de.fhg.ise.IEC61850.client.models.AllianderDER;
import de.fhg.ise.gateway.HederaException;
import de.fhg.ise.gateway.configuration.Settings;
import de.fhg.ise.gateway.interfaces.GridConnectionExtension;
import de.fhg.ise.gateway.interfaces.ems.DTO.ExtensionRequest;
import io.swagger.client.ApiException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Takes care of refreshing schedules at hedera.
 */
public class HederaRefresh {

    private static final Logger log = LoggerFactory.getLogger(HederaRefresh.class);
    private final Settings settings;
    private AllianderDER der;

    private final HederaApi api;

    private Optional<GridConnectionExtension> lastRequest = Optional.empty();
    private Runnable hederaScheduleDeleteJob = () -> {
        log.info("No schedule listed to be cleaned up");
    };

    public HederaRefresh(HederaApi api, AllianderDER der, Settings settings) {
        this.api = api;
        this.settings = settings;
        this.der = der;

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            shutdown();
            log.info("Done shutting down");
        }));
        log.trace("Shutdown hook added");
    }

    public void newRequestFromEms(ExtensionRequest req) {
        log.info("Got new request {}", req);

        log.debug("Cleaning up old schedules at HEDERA");
        hederaScheduleDeleteJob.run();
        log.info("Cleaned up old schedules at HEDERA");

        HederaSchedule hederaSchedule = null;
        try {
            hederaSchedule = req.requestExtensionAwaitCalculation(this.api, this.settings);
        } catch (HederaException e) {
            log.error("Error in schedule {}. Error message by HEDERA: {}", req, e.getMessage());
            log.debug("Error details", e);
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

            final HederaSchedule finalHederaSchedule = hederaSchedule;
            hederaScheduleDeleteJob = () -> {
                try {
                    api.deleteSchedule(finalHederaSchedule);
                } catch (ApiException e) {
                    throw new RuntimeException("Unable to delete schedule at HEDERA", e);
                }
            };

            // TODO: from settings!
            int scheduleNumber = 1;
            int prio = 200;
            int divisor = 60;

            List<Number> values = hederaSchedule.getValues()
                    .stream()
                    .collect(Collectors.toList()); // List<Double> -> List<Number> seems to need that
            try {
                this.der.writeAndEnableSchedule(der.maxPowerSchedules.prepareSchedule(values, scheduleNumber,
                        hederaSchedule.getInterval().getAsDuration().dividedBy(divisor), hederaSchedule.getStart(),
                        prio));
            } catch (ServiceError | IOException e) {
                // TODO: maybe retry?
                log.error("Unable to forward schedule to DER", e);
            }
        }

        // TODO: start a refresh here
        // TODO make use of recommended refresh time (shall be in response for the requests)

        // TODO: this should be covered by the delete job but is not for some reason
        hederaScheduleDeleteJob.run();
    }

    public void shutdown() {
        log.debug("Starting cleanup job");
        try {
            this.hederaScheduleDeleteJob.run();
            log.info("Shut down successfully");
        } catch (Exception e) {
            log.warn("Was unable to delete schedules at hedera", e);
        }
    }
}
