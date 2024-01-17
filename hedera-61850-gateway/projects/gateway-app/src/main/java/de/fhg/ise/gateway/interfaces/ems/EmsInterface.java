package de.fhg.ise.gateway.interfaces.ems;

import de.fhg.ise.gateway.interfaces.hedera.HederaRefresh;

public interface EmsInterface {
    void start(HederaRefresh hederaApi);
}
