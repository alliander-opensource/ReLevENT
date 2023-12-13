package de.fhg.ise.gateway;

import org.ini4j.Ini;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.util.UUID;

/**
 * Settings, parsed from a ini file
 */
public class Settings {

    private static final Logger log = LoggerFactory.getLogger(App.class);

    public final String clientId;
    public final String clientSecret;
    public final UUID importMrid;
    public final double exportLimitWatts;
    public final UUID exportMrid;
    public final double importLimitWatts;

    public Settings(File file) throws SettingsException {

        if (!file.exists()) {
            throw new SettingsException("No ini file available at " + file.toPath().toAbsolutePath() + ". Aborting.");
        }

        log.debug("start parsing config in {}", file.toPath().toAbsolutePath());
        try {
            Ini ini = new Ini(file);
            clientId = getNonNull(ini, "hedera-secrets", "clientId");
            clientSecret = ini.get("hedera-secrets", "clientSecret");

            importMrid = UUID.fromString(getNonNull(ini, "import", "mrid"));
            importLimitWatts = Double.valueOf(getNonNull(ini, "import", "limitWatts"));
            exportMrid = UUID.fromString(getNonNull(ini, "export", "mrid"));
            exportLimitWatts = Double.valueOf(getNonNull(ini, "export", "limitWatts"));
        } catch (Exception e) {
            throw new SettingsException(
                    "Unable to parse required settings from ini file " + file.toPath().toAbsolutePath(), e);
        }
    }

    private static String getNonNull(Ini ini, String section, String option) throws IOException {
        String ret = ini.get(section, option);
        if (ret == null) {
            throw new IOException("No value found for section='" + section + "' and option='" + option + "'.");
        }
        return ret;
    }

    public static class SettingsException extends Exception {
        public SettingsException(String message, Exception cause) {
            super(message, cause);
        }

        public SettingsException(String message) {
            super(message);
        }
    }
}
