package de.fhg.ise.gateway.configuration;

import de.fhg.ise.gateway.interfaces.ems.EmsInterface;
import de.fhg.ise.gateway.interfaces.ems.MqttEmsInterface;
import org.ini4j.Ini;

import java.util.Arrays;
import java.util.stream.Collectors;

public enum EmsInterfaceSettings {

    MQTT_SETTINGS("mqtt") {
        @Override
        public EmsInterface createInterface(Ini ini) {
            return new MqttEmsInterface(ini);
        }
    };

    private final String iniInterfaceTypeString;

    EmsInterfaceSettings(String iniInterfaceTypeString) {
        this.iniInterfaceTypeString = iniInterfaceTypeString;
    }

    /**
     * Start with the settings parsed from the {@link Ini}
     */
    public abstract EmsInterface createInterface(Ini ini);

    /**
     * Parses the .ini for {@link EmsInterfaceSettings} and creates an {@link EmsInterface} instance from the settings
     * in the {@link Ini}.
     */
    public static EmsInterface parseIniCreateInterface(Ini ini) {
        String interfaceType = ini.get("ems-interface", "type");
        EmsInterfaceSettings emsInterfaceSettings = Arrays.stream(EmsInterfaceSettings.values())
                .filter(eis -> eis.iniInterfaceTypeString.equals(interfaceType))
                .findFirst()
                .orElseThrow(() -> new RuntimeException(
                        "Unable to start EMS interface: interface type '" + interfaceType
                                + "' unknown. Known keys are: '" + Arrays.stream(EmsInterfaceSettings.values())
                                .map(s -> s.iniInterfaceTypeString)
                                .collect(Collectors.joining("', '")) + "'."));
        return emsInterfaceSettings.createInterface(ini);
    }

    @Override
    public String toString() {
        return this.getClass().getSimpleName() + "=" + name();
    }
}
