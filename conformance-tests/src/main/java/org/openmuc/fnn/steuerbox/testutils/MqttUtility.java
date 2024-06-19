package org.openmuc.fnn.steuerbox.testutils;

import com.beanit.iec61850bean.ServiceError;
import com.hivemq.client.mqtt.mqtt3.Mqtt3Client;
import io.reactivex.Single;
import org.openmuc.fnn.steuerbox.mqtt.Command;
import org.openmuc.fnn.steuerbox.mqtt.Schedule;
import org.openmuc.fnn.steuerbox.scheduling.PreparedSchedule;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.Closeable;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.time.Instant;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;
import java.util.function.Supplier;

import static org.openmuc.fnn.steuerbox.mqtt.ParsingUtil.scheduleToJson;

public class MqttUtility implements Closeable, ScheduleWriter {

    public static final String SCHEDULE_TOPIC = "fledge/south-schedule";
    public static final String CMD_TOPIC = "fledge/south-command";

    private final static Logger log = LoggerFactory.getLogger(MqttUtility.class);
    private final Mqtt3Client client;
    private Consumer<String> commandConsumer = ignore("command");
    private Consumer<String> scheduleConsumer = ignore("schedule");

    private Consumer<String> ignore(String itemname) {
        return cmdPayload -> log.trace("ignoring {} {}", itemname, cmdPayload);
    }

    public MqttUtility() throws ExecutionException, InterruptedException {
        client = Mqtt3Client.builder()
                .identifier("Junit conformance test started @" + Instant.now())
                .automaticReconnect()
                .maxDelay(60, TimeUnit.SECONDS)
                .applyAutomaticReconnect()
                .serverHost("127.0.0.1")
                .serverPort(1883)
                .build();

        client.toBlocking().connect();

        client.toAsync().connectWith().simpleAuth().username("") // no user required in local container
                .password("".getBytes(StandardCharsets.UTF_8)) // no password required
                .applySimpleAuth().send().orTimeout(100, TimeUnit.MILLISECONDS).whenComplete((ack, throwable) -> {
                    if (throwable != null) {
                        log.warn("Unable to connect. Is the mqtt docker container running?", throwable);
                    }
                    else {
                        log.debug("Connected successfully");
                    }
                });

        subscribeTopic(CMD_TOPIC, () -> this.commandConsumer);
        subscribeTopic(SCHEDULE_TOPIC, () -> this.scheduleConsumer);
    }

    private void publishSchedule(String schedule) {
        String s = "hedera-requests";
        client.toBlocking().publishWith().topic(s).payload(schedule.getBytes(StandardCharsets.UTF_8)).send();
    }

    private void subscribeTopic(String topic, Supplier<Consumer<String>> payloadConsumer)
            throws ExecutionException, InterruptedException {
        client.toAsync().subscribeWith().topicFilter(topic).callback(payload -> {
            String payloadString = new String(payload.getPayloadAsBytes());
            log.trace("Received payload '{}' on topic '{}'", payloadString, topic);
            payloadConsumer.get().accept(payloadString);
        }).send().get();
    }

    @Override
    public void close() throws IOException {
        client.toBlocking().disconnect();
    }

    public static void main(String[] args) throws InterruptedException, ExecutionException {
        MqttUtility mqttClient = new MqttUtility();
        while (true) {
            mqttClient.publishSchedule(
                    "{\"skipHedera\":true,\"direction\":\"IMPORT\",\"start\":{\"seconds\":" + Instant.now()
                            .plusSeconds(1)
                            .getEpochSecond()
                            + ",\"nanos\":0},\"resolution\":\"FIFTEEN_MINUTES\",\"values\":[42,1337]}");
            Thread.sleep(60_000);
        }
    }

    @Override
    public void writeAndEnableSchedule(PreparedSchedule.PreparedScheduleValues values, Duration interval, Instant start,
            int prio) throws ServiceError, IOException {
        String json = scheduleToJson(values.getValues(), interval, start);
        this.publishSchedule(json);
        log.debug("Published schedule {}", json);
    }

    public Single<List<Timestamped<Schedule>>> fetchScheduleUpdate(int updatesToObserve, Duration timeout) {
        Instant start = Instant.now();
        List<Timestamped<Schedule>> ret = new LinkedList<>();
        scheduleConsumer = scheduleStr -> {
            try {
                Instant timestamp = Instant.now();
                Schedule schedule = Schedule.parse(scheduleStr);
                ret.add(new Timestamped<>(schedule, timestamp));
                log.debug("added new schedule {} with ts {}", schedule, timestamp);
            } catch (Exception e) {
                log.error("Unable to parse schedule '{}' and add it as a result", scheduleStr, e);
            }
        };
        log.debug("Start fetching schedule updates @ {}", start);
        ExecutorService exec = Executors.newSingleThreadExecutor();
        Future<List<Timestamped<Schedule>>> future = exec.submit(() -> {
            while (ret.size() < updatesToObserve && Duration.between(start, Instant.now())
                    .minus(timeout)
                    .isNegative()) {
                log.trace("Sleeping");
                Thread.sleep(100);
            }
            log.info("Done fetching schedule updates @ {}", Instant.now());
            scheduleConsumer = ignore("schedule after fetch timed out/enough elements collected");
            return ret;
        });
        return Single.fromFuture(future);
    }

    public Single<List<Timestamped<Command>>> fetchCommandUpdate(int updatesToObserve, Duration timeout) {
        Instant start = Instant.now();
        List<Timestamped<Command>> ret = new LinkedList<>();
        commandConsumer = cmdStr -> {
            try {
                Instant timestamp = Instant.now();
                Command command = Command.fromJsonString(cmdStr);
                ret.add(new Timestamped<>(command, timestamp));
                log.debug("added new command {} with ts {}", command, timestamp);
            } catch (Exception e) {
                log.error("Unable to parse command '{}' and add it as a result", cmdStr, e);
            }
        };
        log.debug("Start fetching command updates @ {}", start);
        ExecutorService exec = Executors.newSingleThreadExecutor();
        Future<List<Timestamped<Command>>> future = exec.submit(() -> {
            while (ret.size() < updatesToObserve && Duration.between(start, Instant.now())
                    .minus(timeout)
                    .isNegative()) {
                log.trace("Sleeping");
                Thread.sleep(100);
            }
            log.info("Done fetching command updates @ {}", Instant.now());
            commandConsumer = ignore("command after fetch timed out/enough elements collected");
            return ret;
        });
        return Single.fromFuture(future);
    }

    public static class Timestamped<T> {
        T object;
        Instant timestamp;

        public Timestamped(T object, Instant timestamp) {
            this.object = object;
            this.timestamp = timestamp;
        }

        public T getObject() {
            return object;
        }

        public Instant getTimestamp() {
            return timestamp;
        }

        @Override
        public String toString() {
            return object.toString() + " timestamped @ " + timestamp.toString();
        }
    }
}
