package de.fhg.ise.gateway.interfaces;

import de.fhg.ise.gateway.configuration.Settings;
import de.fhg.ise.gateway.configuration.SettingsException;
import de.fhg.ise.gateway.interfaces.hedera.HederaApi;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.time.Instant;
import java.util.List;
import java.util.UUID;

// TODO: better name
public abstract class GridConnectionExtension {

    private static final Logger log = LoggerFactory.getLogger(GridConnectionExtension.class);

    public static final String INI_PATH = "hedera-interface.ini";

    final HederaApi hederaApi;
    final Settings settings;

    protected GridConnectionExtension() throws IOException, SettingsException {
        this.settings = new Settings(new File(INI_PATH));
        this.hederaApi = new HederaApi(settings);
    }

    enum Direction {
        IMPORT("import") {
            @Override
            public UUID getUuid(Settings settings) {
                return settings.importMrid;
            }
        },
        EXPORT("export") {
            @Override
            public UUID getUuid(Settings settings) {
                return settings.exportMrid;
            }
        };

        public final String jsonRepresentation;

        Direction(String jsonRepresentation) {
            this.jsonRepresentation = jsonRepresentation;
        }

        public abstract UUID getUuid(Settings settings);
    }

    static class PowerLimit {
        public final Instant start;
        public final double quantity;

        PowerLimit(Instant start, double quantity) {
            this.start = start;
            this.quantity = quantity;
        }
    }

    /**
     * TODO To be implemented using the given protocol.
     */
    public abstract List<PowerLimit> requestExtendedLimit(List<PowerLimit> extendedLimit);
}
