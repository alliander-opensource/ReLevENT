package de.fhg.ise.gateway;

import de.fhg.ise.IEC61850.client.models.AllianderDER;
import de.fhg.ise.gateway.configuration.Settings;
import de.fhg.ise.gateway.configuration.SettingsException;
import de.fhg.ise.gateway.interfaces.hedera.HederaApi;
import de.fhg.ise.gateway.interfaces.hedera.HederaDirection;
import de.fhg.ise.gateway.interfaces.hedera.HederaSchedule;
import de.fhg.ise.gateway.interfaces.hedera.HederaScheduleInterval;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.net.NoRouteToHostException;
import java.time.Duration;
import java.time.Instant;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import static de.fhg.ise.gateway.interfaces.hedera.HederaScheduleInterval.FIVE_MINUTES;
import static java.time.Duration.ofMinutes;

public class DemoApp {
    private static final Logger log = LoggerFactory.getLogger(DemoApp.class);

    // parameters
    public static final String INI_PATH = "docker-image/hedera-interface.ini"; // TODO: revert or add some meaningfule file here
    static List<Double> values = Stream.iterate(1d, i -> i + 1).limit(20).collect(Collectors.toList());
    static HederaScheduleInterval SCHEDULE_RESOLUTION = FIVE_MINUTES;
    static Instant scheduleStart = Instant.now().plus(ofMinutes(2));

    public static void main(String[] args) throws IOException, SettingsException {

        Settings settings = new Settings(new File(INI_PATH));
        HederaApi hederaApi = new HederaApi(settings);

        AllianderDER der = null;
        HederaSchedule schedule = null;
        try {
            schedule = hederaApi.requestExtensionAwaitCalculation(scheduleStart, SCHEDULE_RESOLUTION, values,
                    HederaDirection.EXPORT, settings);

            der = new AllianderDER(settings.derHost, settings.derPort);
            transmitScheduleToDER(der, schedule, settings);

            log.info("Successfully transmitted limits to DER. Schedule execution will start @{}", scheduleStart);
        } catch (NoRouteToHostException e) {
            throw new RuntimeException(
                    "Unable to connect to DER server with hostname=" + settings.derHost + ": " + e.getMessage());
        } catch (Exception e) {

            throw new RuntimeException("Unable to create or read schedule", e);
        } finally {
            if (schedule != null) {
                //Deleting the created schedule at HEDERA
                try {
                    hederaApi.deleteSchedule(schedule);
                    log.info("Successfully deleted schedule id={}", schedule.getScheduleUuid());
                } catch (Exception e) {
                    throw new RuntimeException("Unable to delete schedule with id=" + schedule.getScheduleUuid(), e);
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

    public static void setProxy(String host, int port) {
        System.setProperty("https.proxyHost", host);
        System.setProperty("https.proxyPort", Integer.valueOf(port).toString());
    }
}
