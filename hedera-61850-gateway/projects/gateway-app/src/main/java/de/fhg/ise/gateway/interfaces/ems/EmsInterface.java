package de.fhg.ise.gateway.interfaces.ems;

import de.fhg.ise.gateway.interfaces.hedera.HederaRefresh;

/**
 * Basic interface to the Engery Management System (EMS).
 * <p>
 * It takes care of:
 * <p>
 * - forwarding requests to extend power limits at grid connection point from the EMS to HEDERA
 */
public interface EmsInterface {

    void start(HederaRefresh hederaApi);

    /**
     * Forward a new request from the EMS to HEDERA
     *
     * @param hederaApi
     * @param message
     */
    void onNewRequestFromEms(HederaRefresh hederaApi, String message);
}
