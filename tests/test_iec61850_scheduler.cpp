#include <gtest/gtest.h>
#include <libiec61850/iec61850_client.h>
#include <reading.h>
#include <reading_set.h>
#include "iec61850.hpp"
#include <filter.h>


using namespace std;

extern "C" {
	PLUGIN_INFORMATION *plugin_info();

    PLUGIN_HANDLE plugin_init(ConfigCategory* config,
                          OUTPUT_HANDLE *outHandle,
                          OUTPUT_STREAM output);

    void plugin_shutdown(PLUGIN_HANDLE handle);

    void plugin_ingest(PLUGIN_HANDLE handle,
                   READINGSET *readingSet);
};

#define TCP_TEST_PORT 10002
#define LOCAL_HOST "127.0.0.1"

static const char* default_config = QUOTE({
	"plugin" : {
		"description" : "IEC 61850 Server Scheduler",
		"type" : "string",
		"default" : PLUGIN_NAME,
		"readonly" : "true"
	},
	"name" : {
		"description" : "The IEC 61850 Server name to advertise",
		"type" : "string",
		"default" : "Fledge IEC 61850 North Scheduler",
		"order" : "1",
		"displayName" : "Server Name"
	},
  "protocol_stack" : {
    "description" : "protocol stack parameters",
    "type" : "JSON",
    "displayName" : "Protocol stack parameters",
    "order" : "2",
    "default" : QUOTE({
        "protocol_stack" : {
            "name" : "iec61850scheduler",
            "version" : "1.0",
            "transport_layer" : {
                  "srv_ip":"0.0.0.0",
                  "port":10002,
                  "use_scheduler":true                            
          }
      }
    })
  },
  "modelPath" : {
    "description" : "Path of the model config file to read",
    "type" : "string",
    "default" : "/home/develop/fledgepower/iec61850/fledge-iec61850-north-scheduler/tests/data/schedulermodel.cfg",
    "order": "3",
    "displayName": "Path Of File"
},
  "exchanged_data" : {
    "description" : "Exchanged datapoints configuration",
    "type" : "JSON",
    "displayName" : "Exchanged datapoints",
    "order" : "4",
    "default" : QUOTE({
    "exchanged_data":{
      "datapoints":[
        {
          "label":"TS1",
          "protocols":[
              {
                "name":"iec61850",
                "objref": "DER_Scheduler_Control/ActPow_GGIO1.AnOut1.Oper",
                "cdc": "ApcTyp"
              }
            ]
          }
        ]
      } 
    })
  },                                      
  "scheduler_conf": {
    "description" : "Scheduler configuration",
    "type" : "JSON",
    "displayName" : "Scheduler configuration",
    "order": "5",
    "default" : QUOTE({
      "scheduler_conf":{
        "datapoints":[
        {
          "label":"TS1",
          "protocols":[
              {
                "name":"iec61850",
                "objref": "DER_Scheduler_Control/ActPow_GGIO1.AnOut1",
                "cdc": "ApcTyp"
              }
            ]
          }
        ]
      }
    })
  },
  "tls_conf": {
    "description" : "TLS configuration",
    "type" : "JSON",
    "displayName" : "TLS Configuration",
    "order": "6",
    "default" : QUOTE({      
            "tls_conf" : {
                "private_key" : "iec104_server.key",
                "own_cert" : "iec104_server.cer",
                "ca_certs" : [
                    {
                        "cert_file": "iec104_ca.cer"
                    },
                    {
                        "cert_file": "iec104_ca2.cer"
                    }
                ],
                "remote_certs" : [
                    {
                        "cert_file": "iec104_client.cer"
                    }
                ]
            }       
        })
  }
});



static int outputHandlerCalled = 0;
static Reading* lastReading = nullptr;

static 
void testOutputStream(OUTPUT_HANDLE * handle, READINGSET* readingSet)
{
    const std::vector<Reading*> readings = readingSet->getAllReadings();

    for (Reading* reading : readings) {
        printf("output: Reading: %s\n", reading->getAssetName().c_str());

        std::vector<Datapoint*>& datapoints = reading->getReadingData();

        for (Datapoint* dp : datapoints) {
            printf("output:   datapoint: %s -> %s\n", dp->getName().c_str(), dp->getData().toString().c_str());
        }

        if (lastReading != nullptr) {
            delete lastReading;
            lastReading = nullptr;
        }

        lastReading = new Reading(*reading);
    }

    outputHandlerCalled++;
}

// Class to be called in each test, contains fixture to be used in
class SchedulerTest : public testing::Test {
protected:
    IedConnection connection;

    // Setup is ran for every tests, so each variable are reinitialised
    void SetUp() override {
        
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override {
    }
};

TEST_F(SchedulerTest, RunSimpleSchedule) {
    ConfigCategory config("iec61850", default_config);
    PLUGIN_HANDLE handle = plugin_init(&config, NULL, testOutputStream);

    Thread_sleep(500); /* wait for the server to start */

    IedClientError err;
// Create connection
    connection = IedConnection_create();

    IedConnection_connect(connection, &err, LOCAL_HOST, TCP_TEST_PORT);
    ASSERT_TRUE(err == IED_ERROR_OK);

    /* configure reserve schedule */
    IedConnection_writeInt32Value(connection, &err, "DER_Scheduler_Control/ActPow_Res_FSCH01.SchdPrio.setVal", IEC61850_FC_SP, 0);
    IedConnection_writeInt32Value(connection, &err, "DER_Scheduler_Control/ActPow_Res_FSCH01.NumEntr.setVal", IEC61850_FC_SP, 96);
    IedConnection_writeInt32Value(connection, &err, "DER_Scheduler_Control/ActPow_Res_FSCH01.SchdIntv.setVal", IEC61850_FC_SP, 60 * 15);
    
    for (int i = 0; i < 96; i++) {
        char objRefBuf[130];
        sprintf(objRefBuf, "DER_Scheduler_Control/ActPow_Res_FSCH01.ValASG%03i.setMag.f", i + 1);
    
        IedConnection_writeFloatValue(connection, &err, objRefBuf, IEC61850_FC_SP, (float)10);
    
        if (err != IED_ERROR_OK) {
            printf("ERROR: failed to set %s\n", objRefBuf);
        }
    }

    IedConnection_writeUnsigned32Value(connection, &err, "DER_Scheduler_Control/ActPow_Res_FSCH01.StrTm01.setCal.occ", IEC61850_FC_SP, 0); /* occ = Time(0) */
    IedConnection_writeInt32Value(connection, &err, "DER_Scheduler_Control/ActPow_Res_FSCH01.StrTm01.setCal.occType", IEC61850_FC_SP, 1);  /* occPer = Day(1) */
    IedConnection_writeUnsigned32Value(connection, &err, "DER_Scheduler_Control/ActPow_Res_FSCH01.StrTm01.setCal.hr", IEC61850_FC_SP, 0); /* hr = 0 */
    
    //TODO enable reserve schedule
    
    /* configure schedule */
    
    IedConnection_writeInt32Value(connection, &err, "DER_Scheduler_Control/ActPow_FSCH01.SchdPrio.setVal", IEC61850_FC_SP, 10);
    
    if (err != IED_ERROR_OK) {
        printf("ERROR: failed to set schedule priority\n");
    }
    
    IedConnection_writeInt32Value(connection, &err, "DER_Scheduler_Control/ActPow_FSCH01.NumEntr.setVal", IEC61850_FC_SP, 4);
    
    if (err != IED_ERROR_OK) {
        printf("ERROR: failed to set schedule number of entries\n");
    }
    
    IedConnection_writeInt32Value(connection, &err, "DER_Scheduler_Control/ActPow_FSCH01.SchdIntv.setVal", IEC61850_FC_SP, 2);
    
    if (err != IED_ERROR_OK) {
        printf("ERROR: failed to set SchdIntv.setVal\n");
    }
    
    for (int i = 0; i < 100; i++) {
        char objRefBuf[130];
        sprintf(objRefBuf, "DER_Scheduler_Control/ActPow_FSCH01.ValASG%03i.setMag.f", i + 1);
    
        IedConnection_writeFloatValue(connection, &err, objRefBuf, IEC61850_FC_SP, (float)i);
    
        if (err != IED_ERROR_OK) {
            printf("ERROR: failed to set %s\n", objRefBuf);
        }
    }
    
    MmsValue* strTmVal = MmsValue_newUtcTimeByMsTime(Hal_getTimeInMs() + 3000);
    
    IedConnection_writeObject(connection, &err, "DER_Scheduler_Control/ActPow_FSCH01.StrTm01.setTm", IEC61850_FC_SP, strTmVal);
    
    if (err != IED_ERROR_OK) {
        printf("ERROR: failed to set DER_Scheduler_Control/ActPow_FSCH01.StrTm01.setTm\n");
    }
    
    MmsValue_delete(strTmVal);
    
    strTmVal = MmsValue_newUtcTimeByMsTime(Hal_getTimeInMs() + 15000);
    
    IedConnection_writeObject(connection, &err, "DER_Scheduler_Control/ActPow_FSCH01.StrTm02.setTm", IEC61850_FC_SP, strTmVal);
    
    if (err != IED_ERROR_OK) {
        printf("ERROR: failed to set DER_Scheduler_Control/ActPow_FSCH01.StrTm01.setTm\n");
    }
    
    MmsValue_delete(strTmVal);
    
    strTmVal = MmsValue_newUtcTimeByMsTime(Hal_getTimeInMs() + 25000);
    
    IedConnection_writeObject(connection, &err, "DER_Scheduler_Control/ActPow_FSCH01.StrTm03.setTm", IEC61850_FC_SP, strTmVal);
    
    if (err != IED_ERROR_OK) {
        printf("ERROR: failed to set DER_Scheduler_Control/ActPow_FSCH01.StrTm01.setTm\n");
    }
    
    MmsValue_delete(strTmVal);
    
    //TODO enable schedule
    ControlObjectClient control
        = ControlObjectClient_create("DER_Scheduler_Control/ActPow_FSCH01.EnaReq", connection);
    
    MmsValue* ctlVal = MmsValue_newBoolean(true);
    
    ControlObjectClient_setOrigin(control, NULL, 3);
    
    if (ControlObjectClient_operate(control, ctlVal, 0 /* operate now */)) {
        printf("DER_Scheduler_Control/ActPow_FSCH01.EnaReq operated successfully\n");
    }
    else {
        printf("failed to operate DER_Scheduler_Control/ActPow_FSCH01.EnaReq\n");
    }
    
    MmsValue_delete(ctlVal);
    
    ControlObjectClient_destroy(control);
    
    IedConnection_close(connection);
    
    IedConnection_destroy(connection);

    Thread_sleep(10000);

    Iec61850Utility::log_info("%d\n", outputHandlerCalled);

    plugin_shutdown(handle);
}
