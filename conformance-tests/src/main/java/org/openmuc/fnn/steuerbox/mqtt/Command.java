package org.openmuc.fnn.steuerbox.mqtt;

import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.time.Instant;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import static org.openmuc.fnn.steuerbox.mqtt.ParsingUtil.parseMqttMessages;

/**
 * A command payload representing a control signal intended for the EMS.
 * <p>
 * Only relevant parameters will be parsed, most of the JSON fields are FLEDGE defaults and not relevant for the
 * ReLevENT use cases.
 */
public class Command {
    public Command(long epochSecond, double controlValue) {
        this.epochSecond = epochSecond;
        this.controlValue = controlValue;
    }

    public final long epochSecond;
    public final double controlValue;

    public static Command fromJsonString(String command)
            throws ParserConfigurationException, IOException, SAXException {
        List<Map.Entry<Instant, Double>> entries = parseMqttMessages(command);
        if (entries.size() != 1) {
            throw new IllegalArgumentException(
                    "Expected exactly 1 element to be returned. Are you trying to parse a schedule as a command?");
        }
        Map.Entry<Instant, Double> instantDoubleEntry = entries.get(0);
        return new Command(instantDoubleEntry.getKey().getEpochSecond(), instantDoubleEntry.getValue());
    }

    @Override
    public String toString() {
        return "Command{" + "epochSecond=" + epochSecond + "(equivalent to " + Instant.ofEpochSecond(epochSecond)
                + "), controlValue=" + controlValue + '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o)
            return true;
        if (o == null || getClass() != o.getClass())
            return false;
        Command command = (Command) o;
        return epochSecond == command.epochSecond && Double.compare(controlValue, command.controlValue) == 0;
    }

    @Override
    public int hashCode() {
        return Objects.hash(epochSecond, controlValue);
    }
}
