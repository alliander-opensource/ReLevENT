#include <gtest/gtest.h>
#include <libiec61850/iec61850_client.h>
#include "iec61850.hpp"

using namespace std;

#define TCP_TEST_PORT 10002
#define LOCAL_HOST "127.0.0.1"

static string protocol_stack = QUOTE({
    "protocol_stack" : {
        "name" : "iec61850scheduler",
        "version" : "1.0",
        "transport_layer" : {
        "srv_ip":"0.0.0.0",
        "port":10002,
        "use_scheduler":false
    }
}});

    static string protocol_stack_1 = QUOTE({
        "protocol_stack" : {
            "name" : "iec61850scheduler",
            "version" : "1.0",
            "transport_layer" : {
                "srv_ip" : "0.0.0.0",
                "port" : 10002,
                "use_scheduler" : false
            }
        }
    });

static string exchanged_data = QUOTE({
        "exchanged_data":
        {
            "datapoints" : [
                {
                    "label" : "TS1",
                    "pivot_id" : "ID-1-100",
                    "protocols" : [
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
                    "protocols" : [
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
                    "protocols" : [
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
                    "protocols" : [
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
);

static string tls = QUOTE({
    "tls_conf" : {
        "private_key" : "iec104_server.key",
        "own_cert" : "iec104_server.cer",
        "ca_certs" : [
            {
                "cert_file" : "iec104_ca.cer"
            },
            {
                "cert_file" : "iec104_ca2.cer"
            }
        ],
        "remote_certs" : [
            {
                "cert_file" : "iec104_client.cer"
            }
        ]
    }
});

// Class to be called in each test, contains fixture to be used in
class ConnectionHandlerTest : public testing::Test
{
protected:
    IEC61850Server *iec61850Server; // Object on which we call for tests
    IedConnection connection;
    // Setup is ran for every tests, so each variable are reinitialised
    void SetUp() override
    {
        // Init iec104server object
        iec61850Server = new IEC61850Server();
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete iec61850Server;
    }
};

TEST_F(ConnectionHandlerTest, NormalConnection)
{
    iec61850Server->m_modelPath = "../tests/data/controlmodel.cfg";
    iec61850Server->setJsonConfig(protocol_stack, exchanged_data, "", "");

    Thread_sleep(500); /* wait for the server to start */

    IedClientError err;
    // Create connection
    connection = IedConnection_create();

    IedConnection_connect(connection, &err, LOCAL_HOST, TCP_TEST_PORT);
    ASSERT_TRUE(err == IED_ERROR_OK);

    IedConnection_destroy(connection);
}
