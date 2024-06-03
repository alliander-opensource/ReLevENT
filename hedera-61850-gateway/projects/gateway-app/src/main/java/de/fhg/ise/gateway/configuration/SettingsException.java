package de.fhg.ise.gateway.configuration;

/**
 * To be thrown when a settings ini-file cannot be processed.
 */
public class SettingsException extends Exception {
    public SettingsException(String message, Exception cause) {
        super(message, cause);
    }

    public SettingsException(String message) {
        super(message);
    }
}
