package de.fhg.ise.gateway.configuration;

public class SettingsException extends Exception {
    public SettingsException(String message, Exception cause) {
        super(message, cause);
    }

    public SettingsException(String message) {
        super(message);
    }
}
