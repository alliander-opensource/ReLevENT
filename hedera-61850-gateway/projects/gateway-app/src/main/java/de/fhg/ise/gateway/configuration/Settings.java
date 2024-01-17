package de.fhg.ise.gateway.configuration;

import de.fhg.ise.gateway.DemoApp;
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

    private static final Logger log = LoggerFactory.getLogger(DemoApp.class);

    public final String clientId;
    public final String clientSecret;
    public final UUID importMrid;
    public final double exportLimitWatts;
    public final UUID exportMrid;
    public final double importLimitWatts;
    public final String derHost;
    public final int derPort;
    public final Ini ini;

    public Settings(File file) throws SettingsException {

        if (!file.exists()) {
            throw new SettingsException("No ini file available at " + file.toPath().toAbsolutePath() + ". Aborting.");
        }

        log.debug("start parsing config in {}", file.toPath().toAbsolutePath());
        try {
            ini = new Ini(file);
            clientId = getNonNull(ini, "hedera-secrets", "clientId");
            clientSecret = ini.get("hedera-secrets", "clientSecret");

            importMrid = UUID.fromString(getNonNull(ini, "hedera-import", "mrid"));
            importLimitWatts = Double.valueOf(getNonNull(ini, "hedera-import", "limitWatts"));
            exportMrid = UUID.fromString(getNonNull(ini, "hedera-export", "mrid"));
            exportLimitWatts = Double.valueOf(getNonNull(ini, "hedera-export", "limitWatts"));
            derHost = getNonNull(ini, "der", "host");
            derPort = Integer.valueOf(getNonNull(ini, "der", "port"));
        } catch (Exception e) {
            throw new SettingsException(
                    "Unable to parse required settings from ini file " + file.toPath().toAbsolutePath(), e);
        }
    }

    public static String getNonNull(Ini ini, String section, String option) throws IOException {
        String ret = ini.get(section, option);
        if (ret == null) {
            throw new IOException("No value found for section='" + section + "' and option='" + option + "'.");
        }
        return ret;
    }

}
