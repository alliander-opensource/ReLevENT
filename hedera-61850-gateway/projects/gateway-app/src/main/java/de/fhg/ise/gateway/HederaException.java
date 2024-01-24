package de.fhg.ise.gateway;

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
