package de.fhg.ise.gateway.interfaces.ems;

import de.fhg.ise.gateway.interfaces.hedera.HederaRefresh;

/**
 * Basic interface to the Engery Management System (EMS).
 * <p>
 * It takes care of:
 * <p>
 * - forwarding requests to extend power limits at grid connection point from the EMS to HEDERA
 * <p>
 * - forwarding the response (the new new power limit) from HEDERA to the EMS
 */
// TODO: facilitate the implementation in various communication protocols -> split the code into several methods!
public interface EmsInterface {
    void start(HederaRefresh hederaApi);

    // TODO: better separation of code, add newRequestFromEms(..) and possibly onHederaResponse(..)
}
