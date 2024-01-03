package de.fhg.ise.gateway;

import de.fhg.ise.IEC61850.client.models.AllianderDER;
import de.fhg.ise.gateway.HederaApi.HederaSchedule;
import io.swagger.client.model.Schedule;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.util.List;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import static de.fhg.ise.gateway.HederaApi.HederaScheduleInterval.FIVE_MINUTES;
import static java.time.Duration.ofMinutes;

public class App {
    private static final Logger log = LoggerFactory.getLogger(App.class);

    // parameters
    public static final String INI_PATH = "hedera-interface.ini";
    static List<Double> values = Stream.iterate(1d, i -> i + 1).limit(20).collect(Collectors.toList());
    static HederaApi.HederaScheduleInterval SCHEDULE_RESOLUTION = FIVE_MINUTES;
    static Instant scheduleStart = Instant.now().plus(ofMinutes(2));

    public static void main(String[] args) throws IOException, Settings.SettingsException {

        Settings settings = new Settings(new File(INI_PATH));
        HederaApi hederaApi = new HederaApi(settings);

        UUID mrid = settings.exportMrid; // export from the grid -> draw from grid

        Optional<UUID> scheduleId = Optional.empty();
        AllianderDER der = null;
        try {
            scheduleId = Optional.ofNullable(
                    hederaApi.createSchedule(mrid, scheduleStart, SCHEDULE_RESOLUTION, values));
            log.info("created new schedule with mrid={}", scheduleId.get());
            log.info("Requesting schedule in kW: {}", values);

            HederaSchedule schedule = awaitScheduleCalculationAtHedera(hederaApi, scheduleId,
                    Duration.ofSeconds(10 * 10));
            log.info("Result schedule in kW:   {}", schedule.getValues());

            der = new AllianderDER(settings.derHost, settings.derPort);
            transmitScheduleToDER(der, schedule, settings);

            log.info("Successfully transmitted limits to DER. Schedule execution will start @{}", scheduleStart);
        } catch (Exception e) {
            throw new RuntimeException("Unable to create or read schedule", e);
        } finally {
            if (scheduleId.isPresent()) {
                //Deleting the created schedule at HEDERA
                try {
                    hederaApi.deleteSchedule(scheduleId.get());

                    log.info("Successfully deleted schedule id={}", scheduleId.get());
                } catch (Exception e) {
                    throw new RuntimeException("Unable to delete schedule with id=" + scheduleId.get(), e);
                }
            }
            else {
                log.debug("It seems no schedule was created, so not deleting any.");
            }
            if (der != null) {
                der.close();
                log.info("Disconnected from DER");
            }
        }
    }

    static class IEC61850ForwardingException extends Exception {
        IEC61850ForwardingException(Exception e) {
            super("Unable to forward schedule via IEC 61850", e);
        }
    }

    private static void transmitScheduleToDER(AllianderDER der, HederaSchedule schedule, Settings settings)
            throws IEC61850ForwardingException {
        try {
            List<Number> resultLimitationsInWatt = schedule.getValues()
                    .stream()
                    .map(limitKilwatt -> limitKilwatt * 1000) // kW to W conversion
                    .map(hederaScheduleLimit -> hederaScheduleLimit + settings.importLimitWatts)
                    .collect(Collectors.toList());
            log.info("Transmitting limits {}W to DER", resultLimitationsInWatt);
            int divisor = 60; // FIXME this is only here for the demo, should be gone in final version!
            if (divisor != 1) {
                log.info("Scaling down schedule interval by factor {} (1min = {}s)", divisor,
                        Duration.ofMinutes(1).dividedBy(divisor).toMillis() / 1000.0);
            }
            der.writeAndEnableSchedule(der.maxPowerSchedules.prepareSchedule(resultLimitationsInWatt, 1,
                    schedule.getInterval().getAsDuration().dividedBy(divisor), schedule.getStart(), 200));
        } catch (Exception e) {
            throw new IEC61850ForwardingException(e);
        }
    }

    static class HederaException extends Exception {
        public HederaException(Exception e) {
            super("Error in communication with HEDERA", e);
        }
    }

    private static HederaSchedule awaitScheduleCalculationAtHedera(HederaApi hederaApi, Optional<UUID> scheduleId,
            Duration durationUntilAbort) throws HederaException {
        Schedule.AtTypeEnum status;
        HederaSchedule schedule;
        int count = 0;
        final int maxCounts = 10;
        final long pollrate = durationUntilAbort.toMillis() / maxCounts;
        try {
            do {
                // give hedera a bit time to calculate the schedules
                Thread.sleep(pollrate);

                //Reading the created schedule at HEDERA
                schedule = hederaApi.readSchedule(scheduleId.get());
                //   log.debug("read schedule result: " + schedule.getRawResponse());
                status = schedule.getStatus();
                log.debug("New schedule at HEDERA, is now in state {} with message {}", status,
                        schedule.getStatusMessage());

                count++;
                log.info("Read HEDERA API " + count + " of " + maxCounts
                        + ". Schedule calculation is currently pending. Will try again in " + pollrate / 1000.0 + "s.");

            } while ((count < maxCounts) && (!Schedule.AtTypeEnum.ACCEPTED.equals(status)));
            return schedule;
        } catch (Exception e) {
            throw new HederaException(e);
        }
    }

    public static void setProxy(String host, int port) {
        System.setProperty("https.proxyHost", host);
        System.setProperty("https.proxyPort", Integer.valueOf(port).toString());
    }
}
