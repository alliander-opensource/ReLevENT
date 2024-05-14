package de.fhg.ise.gateway.interfaces.ems;

import com.hivemq.client.mqtt.mqtt3.Mqtt3AsyncClient;
import com.hivemq.client.mqtt.mqtt3.Mqtt3Client;
import de.fhg.ise.gateway.configuration.Settings;
import de.fhg.ise.gateway.interfaces.ems.DTO.ExtensionRequest;
import de.fhg.ise.gateway.interfaces.hedera.HederaRefresh;
import org.ini4j.Ini;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.UUID;
import java.util.concurrent.TimeUnit;

/**
 * An interface to the local Energy Management System to be controlled by HEDERA. Implemented using MQTT communication.
 */
public class MqttEmsInterface implements EmsInterface {

    private final MqttSettings settings;

    private static final Logger log = LoggerFactory.getLogger(MqttEmsInterface.class);

    public MqttEmsInterface(Ini ini) {
        settings = new MqttSettings(ini);
    }

    void onNewMessage(HederaRefresh hederaApi, String message) {
        ExtensionRequest extensionRequest = ExtensionRequest.fromJson(message);
        hederaApi.newRequestFromEms(extensionRequest);
    }

    @Override
    public void start(HederaRefresh hederaApi) {

        log.info("Starting MQTT EMS interface with settings {}", this.settings);

        Mqtt3AsyncClient client = Mqtt3Client.builder()
                .identifier(UUID.randomUUID().toString())
                .automaticReconnect()
                .maxDelay(60, TimeUnit.SECONDS)
                .applyAutomaticReconnect()
                .serverHost(this.settings.host)
                .serverPort(this.settings.port)
                .buildAsync();

        client.connectWith()
                .simpleAuth()
                .username(this.settings.user)
                .password(this.settings.password.getBytes())
                .applySimpleAuth()
                .send()
                .whenComplete((ack, throwable) -> {
                    if (throwable != null) {
                        log.warn("Unable to connect", throwable);
                    }
                    else {
                        log.debug("Connected to broker {} at port {} with user {} successfully", this.settings.host,
                                this.settings.port, this.settings.user);
                    }
                });

        client.subscribeWith().topicFilter(this.settings.topic).callback(payload -> {
            String payloadString = new String(payload.getPayloadAsBytes());
            log.debug("Received payload '{}' on topic '{}'", payloadString, this.settings.topic);
            onNewMessage(hederaApi, payloadString);
        }).send().whenComplete(((mqtt3SubAck, throwable) -> {
            if (throwable != null) {
                log.warn("Unable to subscribe to topic '{}'", this.settings.topic, throwable);
            }
            else {
                log.info("Successfully subscribed to topic '{}' on host '{}'. Awaiting messages.", this.settings.topic,
                        this.settings.host);
            }
        }));
    }

    @Override
    public String toString() {
        return "MqttEmsInterface{" + "settings=" + settings + '}';
    }

    static class MqttSettings {
        final String host;
        final String user;
        final String password;
        final int port;
        final String topic;

        public MqttSettings(Ini ini) {
            try {
                host = Settings.getNonNull(ini, "ems-interface", "host");
                user = Settings.getNonNull(ini, "ems-interface", "user");
                password = Settings.getNonNull(ini, "ems-interface", "password");
                port = Integer.valueOf(Settings.getNonNull(ini, "ems-interface", "port"));
                topic = Settings.getNonNull(ini, "ems-interface", "topic");
            } catch (Exception e) {
                throw new RuntimeException("Unable to parse settings", e);
            }
        }

        @Override
        public String toString() {
            return "MqttSettings{" + "host='" + host + '\'' + ", user='" + user + '\'' + ", port=" + port
                    + ", password=***}";
        }
    }
}
