package org.openmuc.fnn.steuerbox.testutils;

import com.beanit.iec61850bean.ServiceError;
import org.openmuc.fnn.steuerbox.scheduling.PreparedSchedule;

import java.io.IOException;
import java.time.Duration;
import java.time.Instant;

/**
 * Common interface for all utilities that are able to write a IEC 61850 schedule to a device
 */
public interface ScheduleWriter {

    void writeAndEnableSchedule(PreparedSchedule.PreparedScheduleValues values, Duration interval, Instant start,
            int prio) throws ServiceError, IOException;

    default <T> void writeAndEnableSchedule(PreparedSchedule<T> preparedSchedule) throws ServiceError, IOException {
        preparedSchedule.writeAndEnable(this);
    }

}
