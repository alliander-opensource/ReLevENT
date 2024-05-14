package de.fhg.ise.gateway;

/**
 * To be thrown when communication with HEDERA API does not work as expected
 */
public class HederaException extends Exception {
    public HederaException(Exception e) {
        super("Error in communication with HEDERA: " + e.getClass() + ":" + e.getMessage(), e);
    }

    public HederaException(String message) {
        super(message);
    }

    public HederaException(String message, Exception e) {
        super(message, e);
    }
}
