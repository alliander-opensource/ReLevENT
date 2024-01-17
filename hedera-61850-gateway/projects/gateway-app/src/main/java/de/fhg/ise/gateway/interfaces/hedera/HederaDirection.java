package de.fhg.ise.gateway.interfaces.hedera;

import de.fhg.ise.gateway.configuration.Settings;
import io.swagger.client.model.RegisteredInterTie;

import java.util.UUID;

public enum HederaDirection {
    IMPORT(RegisteredInterTie.DirectionEnum.I) {
        @Override
        public UUID getMRID(Settings settings) {
            return settings.importMrid;
        }
    },
    EXPORT(RegisteredInterTie.DirectionEnum.E) {
        @Override
        public UUID getMRID(Settings settings) {
            return settings.exportMrid;
        }
    };

    private final RegisteredInterTie.DirectionEnum direction;

    HederaDirection(RegisteredInterTie.DirectionEnum directionEnum) {
        this.direction = directionEnum;
    }

    RegisteredInterTie.DirectionEnum getDirectionAsHederaEnum() {
        return direction;
    }

    public abstract UUID getMRID(Settings settings);
}
