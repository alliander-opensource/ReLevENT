package org.openmuc.fnn.steuerbox.mqtt;

import com.fasterxml.jackson.databind.JsonNode;
import org.openmuc.fnn.steuerbox.Context;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.time.Duration;
import java.time.Instant;
import java.util.AbstractMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Utilities to parse JSON messages
 */
public class ParsingUtil {

    private final static Logger log = LoggerFactory.getLogger(ParsingUtil.class);

    private ParsingUtil() {
        // prevent calling constructor: utility class with no members
    }

    /**
     * Deserialize a schedule for communication with HEDERA-61850-interface.
     * <p>
     * Will use a switch to not communicate the request to HEDERA but instead immediately forward it to the IEC 61850
     * FLEDGE scheduling server
     */
    public static String scheduleToJson(List values, Duration interval, Instant start) {
        ScheduleResolution scheduleResolution = ScheduleResolution.from(interval);
        return String.format("{\"skipHedera\":true," //
                        + "\"direction\":\"IMPORT\","//
                        + "\"start\":{\"seconds\":%s,\"nanos\":%s},"//
                        + "\"resolution\":\"%s\"," //
                        + "\"values\":[%s]}", start.getEpochSecond(), start.getNano(), scheduleResolution.name(),
                values.stream().map(Object::toString).collect(Collectors.joining(",")));
    }

    /**
     * Utility to parse command or schedule parameters from a JSON message (command and schedules are very alike: a
     * schedule is like an array of commands)
     */
    static List<Map.Entry<Instant, Double>> parseMqttMessages(String commandOrScheduleJson) {
        List<Map.Entry<Instant, Double>> entities = new LinkedList<>();
        try {
            Iterator<JsonNode> parameters = Context.getObjectMapper()
                    .readTree(commandOrScheduleJson)
                    .get("parameters")
                    .elements();
            while (parameters.hasNext()) {
                JsonNode parameter = parameters.next();
                JsonNode apcTypNode = parameter.get("value").get("GTIC").get("ApcTyp");
                JsonNode tNode = apcTypNode.get("t");
                long epochSeconds = tNode.get("SecondSinceEpoch").longValue();
                double controlValue = apcTypNode.get("ctlVal").asDouble();
                long nanos = 0; // could be created from FractionOfSecond, but testing does not make sense on that base..
                log.trace("Ignoring FractionOfSecond, testing on that base does not make sense");
                entities.add(new AbstractMap.SimpleEntry<>(Instant.ofEpochSecond(epochSeconds, nanos), controlValue));
            }
            return entities;
        } catch (Exception e) {
            throw new RuntimeException(
                    "Unable deserialize command from '" + commandOrScheduleJson + "': Required elements not found.", e);
        }
    }
}
