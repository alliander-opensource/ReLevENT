#include <libiec61850/iec61850_client.h>
#include <reading.h>
#include <reading_set.h>
#include "iec61850.hpp"
#include <filter.h>


using namespace std;

#define TCP_TEST_PORT 10002
#define LOCAL_HOST "127.0.0.1"

extern "C" {
PLUGIN_INFORMATION *plugin_info();

PLUGIN_HANDLE plugin_init(ConfigCategory* config,
                          OUTPUT_HANDLE *outHandle,
                          OUTPUT_STREAM output);

void plugin_shutdown(PLUGIN_HANDLE handle);

void plugin_ingest(PLUGIN_HANDLE handle,
                   READINGSET *readingSet);

void plugin_register(PLUGIN_HANDLE handle,
                     bool ( *write)(const char *name, const char *value, ControlDestination destination, ...),
                     int (* operation)(char *operation, int paramCount, char *names[], char *parameters[], ControlDestination destination, ...));
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
                                "use_scheduler":false
                    }
                }
                          })
    },
                                              "modelPath" : {
        "description" : "Path of the model config file to read",
        "type" : "string",
        "default" : "../tests/data/controlmodel.cfg",
        "order": "3",
        "displayName": "Path Of File"
    },
                                              "exchanged_data" : {
        "description" : "Exchanged datapoints configuration",
        "type" : "JSON",
        "displayName" : "Exchanged datapoints",
        "order" : "4",
        "default" : QUOTE({
                                  "exchanged_data":
                                  {
                                      "datapoints" :[
                                      {
                                          "label" : "TS1",
                                                  "pivot_id" : "ID-1-100",
                                                  "protocols" :[
                                          {
                                              "name" : "iec61850",
                                                      "objref" : "simpleIOGenericIO/GGIO1.SPCSO1",
                                                      "cdc" : "SpcTyp"
                                          }
                                          ]
                                      },
                                      {
                                          "label" : "TS2",
                                                  "pivot_id" : "ID-1-101",
                                                  "protocols" :[
                                          {
                                              "name" : "iec61850",
                                                      "objref" : "simpleIOGenericIO/GGIO1.SPCSO2",
                                                      "cdc" : "SpcTyp"
                                          }
                                          ]
                                      },
                                      {
                                          "label" : "TS3",
                                                  "pivot_id" : "ID-1-102",
                                                  "protocols" :[
                                          {
                                              "name" : "iec61850",
                                                      "objref" : "simpleIOGenericIO/GGIO1.SPCSO3",
                                                      "cdc" : "SpcTyp"
                                          }
                                          ]
                                      },
                                      {
                                          "label" : "TS4",
                                                  "pivot_id" : "ID-1-103",
                                                  "protocols" :[
                                          {
                                              "name" : "iec61850",
                                                      "objref" : "simpleIOGenericIO/GGIO1.SPCSO4",
                                                      "cdc" : "SpcTyp"
                                          }
                                          ]
                                      }
                                      ]
                                  }
                          }
        )
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


static Datapoint*
getChild(Datapoint* dp, const std::string& name)
{
    Datapoint* childDp = nullptr;

    DatapointValue& dpv = dp->getData();


    if(dpv.getType() != DatapointValue::T_DP_DICT){
        Iec61850Utility::log_warn("Datapoint not a dictionary");
        return nullptr;
    }

    std::vector<Datapoint*>* datapoints = dpv.getDpVec();

    if (!datapoints) {
        Iec61850Utility::log_warn("datapoints is nullptr");
        return nullptr;
    }

    for (auto child : *datapoints) {
        if (child->getName() == name) {
            childDp = child;
            break;
        }
    }

    return childDp;
}

static Datapoint*
getCdc(Datapoint* dp)
{
    if(!dp) return nullptr;

    DatapointValue& dpv = dp->getData();

    if(dpv.getType() != DatapointValue::T_DP_DICT) return nullptr;

    std::vector<Datapoint*>* datapoints = dpv.getDpVec();

    for (Datapoint* child : *datapoints) {
        if(IEC61850Datapoint::getCdcTypeFromString(child->getName()) != -1){
            return child;
        }
    }

    return nullptr;
}

static Datapoint* jsonParser;
static int operateHandlerCalled = 0;
static std::shared_ptr<Datapoint> lastDatapoint = nullptr;

static int operateHandler(char *operation, int paramCount, char* names[], char *parameters[], ControlDestination destination, ...)
{
    operateHandlerCalled++;
    std::vector<Datapoint*>* parametersDp = jsonParser->parseJson(parameters[0]);
    lastDatapoint = make_shared<Datapoint>(parametersDp->at(0)->getName(), parametersDp->at(0)->getData());
    Iec61850Utility::log_info("Operate handler called %s", lastDatapoint->toJSONProperty().c_str());

    for (auto dp : *parametersDp) {
        delete dp;
    }
    delete parametersDp;

    return 1;
}


// Class to be called in each test, contains fixture to be used in
class ControlTest : public testing::Test {
protected:
    IedConnection connection;

    // Setup is ran for every tests, so each variable are reinitialised
    void SetUp() override {
        // Init iec104server object
        operateHandlerCalled = 0;
        lastDatapoint = nullptr;
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override {
        lastDatapoint = nullptr;
    }
};



TEST_F(ControlTest, SendSpcCommand) {

    Thread_sleep(500); /* wait for the server to start */

    IedClientError err;

    // Create connection
    connection = IedConnection_create();

    operateHandlerCalled = 0;

    vector<Datapoint*> dataobjects;

    ConfigCategory config("iec61850Config", default_config);
    PLUGIN_HANDLE handle = plugin_init(&config, NULL, NULL);
    plugin_register(handle, NULL, operateHandler);

    IedConnection_connect(connection, &err, LOCAL_HOST, TCP_TEST_PORT);

    ASSERT_EQ(err,IED_ERROR_OK);

    ControlObjectClient coc = ControlObjectClient_create("simpleIOGenericIO/GGIO1.SPCSO1",connection);
    MmsValue* mmsVal = MmsValue_newBoolean(true);
    uint64_t currentTime = PivotTimestamp::GetCurrentTimeInMs();
    ASSERT_TRUE(ControlObjectClient_operate(coc,mmsVal,0));
    MmsValue_delete(mmsVal);
    ControlObjectClient_destroy(coc);
    IedConnection_destroy(connection);
    Datapoint* cdc = getCdc(lastDatapoint.get());
    ASSERT_NE(cdc,nullptr);
    ASSERT_EQ(cdc->getName(),"SpcTyp");

    Datapoint* t = getChild(cdc, "t");
    ASSERT_NE(t,nullptr);
    ASSERT_EQ(t->getName(),"t");
    PivotTimestamp* ts = new PivotTimestamp(t);
    ASSERT_NEAR(ts->getTimeInMs(),currentTime,100);
    delete ts;

    Datapoint* ctlVal = getChild(cdc,"ctlVal");
    ASSERT_NE(ctlVal,nullptr);
    ASSERT_EQ(ctlVal->getName(),"ctlVal");
    ASSERT_EQ(ctlVal->getData().toInt(),1);

    Datapoint* comingFrom = getChild(lastDatapoint.get(),"ComingFrom");
    ASSERT_NE(comingFrom,nullptr);
    ASSERT_EQ(comingFrom->getName(),"ComingFrom");
    ASSERT_EQ(comingFrom->getData().toStringValue(),"iec61850");

    Datapoint* identifier = getChild(lastDatapoint.get(), "Identifier");
    ASSERT_NE(identifier,nullptr);
    ASSERT_EQ(identifier->getName(),"Identifier");
    ASSERT_EQ(identifier->getData().toStringValue(),"TS1");

    Datapoint* select = getChild(lastDatapoint.get(),"Select");
    ASSERT_NE(select,nullptr);
    ASSERT_EQ(select->getName(),"Select");
    Datapoint* stVal = getChild(select,"stVal");
    ASSERT_NE(stVal,nullptr);
    ASSERT_EQ(stVal->getName(),"stVal");
    ASSERT_EQ(stVal->getData().toInt(),0);

    plugin_shutdown(handle);
}

TEST_F(ControlTest, SendSpcCommandSelect) {

    Thread_sleep(500); /* wait for the server to start */

    IedClientError err;

    // Create connection
    connection = IedConnection_create();

    operateHandlerCalled = 0;

    vector<Datapoint*> dataobjects;

    ConfigCategory config("iec61850Config", default_config);
    PLUGIN_HANDLE handle = plugin_init(&config, NULL, NULL);
    plugin_register(handle, NULL, operateHandler);

    IedConnection_connect(connection, &err, LOCAL_HOST, TCP_TEST_PORT);

    ASSERT_EQ(err,IED_ERROR_OK);

    ControlObjectClient coc = ControlObjectClient_create("simpleIOGenericIO/GGIO1.SPCSO4",connection);
    MmsValue* mmsVal = MmsValue_newBoolean(true);
    uint64_t currentTime = PivotTimestamp::GetCurrentTimeInMs();
    ASSERT_TRUE(ControlObjectClient_selectWithValue(coc,mmsVal));
    MmsValue_delete(mmsVal);
    ControlObjectClient_destroy(coc);
    IedConnection_destroy(connection);
    Datapoint* cdc = getCdc(lastDatapoint.get());
    ASSERT_NE(cdc,nullptr);
    ASSERT_EQ(cdc->getName(),"SpcTyp");

    Datapoint* t = getChild(cdc, "t");
    ASSERT_NE(t,nullptr);
    ASSERT_EQ(t->getName(),"t");
    PivotTimestamp* ts = new PivotTimestamp(t);
    ASSERT_NEAR(ts->getTimeInMs(),currentTime,100);
    delete ts;


    Datapoint* comingFrom = getChild(lastDatapoint.get(),"ComingFrom");
    ASSERT_NE(comingFrom,nullptr);
    ASSERT_EQ(comingFrom->getName(),"ComingFrom");
    ASSERT_EQ(comingFrom->getData().toStringValue(),"iec61850");

    Datapoint* identifier = getChild(lastDatapoint.get(), "Identifier");
    ASSERT_NE(identifier,nullptr);
    ASSERT_EQ(identifier->getName(),"Identifier");
    ASSERT_EQ(identifier->getData().toStringValue(),"TS4");

    Datapoint* select = getChild(lastDatapoint.get(),"Select");
    ASSERT_NE(select,nullptr);
    ASSERT_EQ(select->getName(),"Select");
    Datapoint* stVal = getChild(select,"stVal");
    ASSERT_NE(stVal,nullptr);
    ASSERT_EQ(stVal->getName(),"stVal");
    ASSERT_EQ(stVal->getData().toInt(),1);

    plugin_shutdown(handle);
}
