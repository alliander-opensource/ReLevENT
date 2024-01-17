package de.fhg.ise.gateway.interfaces.ems.DTO;

import de.fhg.ise.gateway.interfaces.hedera.HederaDirection;
import de.fhg.ise.gateway.interfaces.hedera.HederaScheduleInterval;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import java.time.Instant;
import java.util.Arrays;

import static de.fhg.ise.gateway.interfaces.hedera.HederaScheduleInterval.FIVE_MINUTES;

class ExtensionRequestTest {

    @Test
    void serializationResultIsAsExpected() {
        ExtensionRequest request = new ExtensionRequest();
        request.setResolution(HederaScheduleInterval.FIFTEEN_MINUTES);
        request.setStart(Instant.ofEpochSecond(1704807420));
        request.setDirection(HederaDirection.IMPORT);
        request.setValues(Arrays.asList(42d, 1337d));

        final String expected = "{\"direction\":\"IMPORT\",\"start\":{\"seconds\":1704807420,\"nanos\":0},\"resolution\":\"FIFTEEN_MINUTES\",\"values\":[42.0,1337.0]}";
        Assertions.assertEquals(expected, request.toJson());
    }

    @Test
    void deserializationResultIsAsExpected() {
        final String json = "{\"direction\":\"EXPORT\",\"start\":{\"seconds\":1704721020,\"nanos\":1337},\"resolution\":\"FIVE_MINUTES\",\"values\":[13.37]}";

        ExtensionRequest actual = ExtensionRequest.fromJson(json);
        Assertions.assertEquals(HederaDirection.EXPORT, actual.getDirection());
        Assertions.assertEquals(FIVE_MINUTES, actual.getResolution());
        Assertions.assertEquals(Instant.ofEpochSecond(1704721020, 1337), actual.getStart());
        Assertions.assertIterableEquals(Arrays.asList(13.37), actual.getValues());
    }
}