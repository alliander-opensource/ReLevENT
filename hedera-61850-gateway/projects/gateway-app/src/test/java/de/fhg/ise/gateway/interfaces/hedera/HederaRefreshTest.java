package de.fhg.ise.gateway.interfaces.hedera;

import org.junit.jupiter.api.Test;

import java.time.Instant;

import static de.fhg.ise.gateway.interfaces.hedera.HederaScheduleInterval.FIFTEEN_MINUTES;
import static de.fhg.ise.gateway.interfaces.hedera.HederaScheduleInterval.FIVE_MINUTES;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

class HederaRefreshTest {

    @Test
    void alignmentCheck() {
        // 10.1.24 16:00
        assertTrue(HederaRefresh.isAligned(Instant.ofEpochSecond(1704902400), FIVE_MINUTES));
        assertTrue(HederaRefresh.isAligned(Instant.ofEpochSecond(1704902400), FIFTEEN_MINUTES));

        // 10.1.24 16:01
        assertFalse(HederaRefresh.isAligned(Instant.ofEpochSecond(1704902460), FIVE_MINUTES));
        assertFalse(HederaRefresh.isAligned(Instant.ofEpochSecond(1704902460), FIFTEEN_MINUTES));

        // 10.1.24 16:05
        assertTrue(HederaRefresh.isAligned(Instant.ofEpochSecond(1704902700), FIVE_MINUTES));
        assertFalse(HederaRefresh.isAligned(Instant.ofEpochSecond(1704902700), FIFTEEN_MINUTES));
    }

}