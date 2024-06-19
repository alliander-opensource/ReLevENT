package org.openmuc.fnn.steuerbox;

import com.beanit.iec61850bean.ModelNode;
import com.beanit.iec61850bean.ServiceError;
import de.fhg.ise.testtool.utils.annotations.label.Description;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.openmuc.fnn.steuerbox.models.AllianderDER;

import java.io.IOException;
import java.nio.file.Path;

public class VersionTest {

    public final static String EXPECTED_LIB_IEC_61850_VERSION = "1.5.2";

    @Description("Checks installed library version of libiec61850. Expects v" + EXPECTED_LIB_IEC_61850_VERSION + ".")
    @DisplayName("IEC 61850 Protocol Library version check")
    @Test
    void libIecVersionMatchesExpectedVersion() throws IOException {
        final String libIecLink = "/usr/local/lib/libiec61850.so";
        Path path = Path.of(libIecLink).toRealPath();
        String versionSuffix = path.toString().replaceAll(libIecLink + ".", "");

        Assertions.assertEquals(EXPECTED_LIB_IEC_61850_VERSION, versionSuffix);
    }

    @Description("Reads the IEC 61850 servers name plate and makes sure the correct config version is used")
    @DisplayName("IEC 61850 config check")
    @Test
    void correctSCLFileIsInUse() throws ServiceError, IOException {
        AllianderDER dut = AllianderDER.getWithDefaultSettings();
        ModelNode node = dut.getNodeWithValues("DER_Scheduler_Control/LLN0.NamPlt");
        String vendor = node.getChild("vendor").getBasicDataAttributes().get(0).getValueString();
        String swRev = node.getChild("swRev").getBasicDataAttributes().get(0).getValueString();
        String d = node.getChild("d").getBasicDataAttributes().get(0).getValueString();
        String configRev = node.getChild("configRev").getBasicDataAttributes().get(0).getValueString();

        Assertions.assertEquals("Alliander", vendor);
        Assertions.assertEquals("1.0.0", swRev);
        Assertions.assertEquals("ReLevENT reference DER model", d);
        Assertions.assertEquals("1", configRev);
    }
}
