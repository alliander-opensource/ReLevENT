/*
 * Copyright 2023 MZ Automation GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

#include <libiec61850/hal_thread.h>
#include <plugin_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string>
#include <logger.h>
#include <plugin_exception.h>
#include <iostream>
#include <config_category.h>
#include <version.h>
#include <iec61850.hpp>


using namespace std;
using namespace rapidjson;

extern "C" {

#define PLUGIN_NAME "iec61850"
  /**
 * Plugin specific default configuration
 */
  static const char *default_config = QUOTE({
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
            "srv_ip" : "0.0.0.0",
            "port" : 103,
            "use_scheduler" : false
          }
        }
      })
    },
    "modelPath" : {
      "description" : "Path of the model config file to read",
      "type" : "string",
      "default" : "../tests/data/schedulermodel.cfg",
      "order" : "3",
      "displayName" : "Path Of File"
    },
    "exchanged_data" : {
      "description" : "Exchanged datapoints configuration",
      "type" : "JSON",
      "displayName" : "Exchanged datapoints",
      "order" : "4",
      "default" : QUOTE({
        "exchanged_data" : {
          "datapoints" : [
            {
              "label" : "TS1",
              "protocols" : [
                {
                  "name" : "iec61850",
                  "objref" : "DER_Scheduler_Control/ActPow_GGIO1.AnOut1",
                  "cdc" : "ApcTyp"
                }
              ]
            }
          ]
        }
      })
    },
    "scheduler_conf" : {
      "description" : "Scheduler configuration",
      "type" : "JSON",
      "displayName" : "Scheduler configuration",
      "order" : "5",
      "default" : QUOTE({
        "scheduler_conf" : {
          "schedules" : [
            {
              "scheduleRef" : "@Control/ActPow_Res_FSCH01",
              "enableScheduleControl" : false,
              "enabled" : true,
              "parameters" : [
                {
                  "parameter" : "SCHED_PARAM_STR_TM",
                  "enableWriteAccess" : true
                },
                {
                  "parameter" : "SCHED_PARAM_SCHD_PRIO",
                  "enableWriteAccess" : false
                }
              ]
            },
            {
              "scheduleRef" : "@Control/MaxPow_Res_FSCH01",
              "enableScheduleControl" : false,
              "enabled" : true,
              "parameters" : [
                {
                  "parameter" : "SCHED_PARAM_STR_TM",
                  "enableWriteAccess" : false
                },
                {
                  "parameter" : "SCHED_PARAM_SCHD_PRIO",
                  "enableWriteAccess" : false
                }
              ]
            },
            {
              "scheduleRef" : "@Control/OnOff_Res_FSCH01",
              "enableScheduleControl" : false,
              "enabled" : true,
              "parameters" : [
                {
                  "parameter" : "SCHED_PARAM_STR_TM",
                  "enableWriteAccess" : false
                },
                {
                  "parameter" : "SCHED_PARAM_SCHD_PRIO",
                  "enableWriteAccess" : false
                }
              ]
            }
          ],
          "storage" : {
            "databaseUri" : "scheduler-db.json",
            "parameters" : []
          }
        }
      })
    },
    "tls_conf" : {
      "description" : "TLS configuration",
      "type" : "JSON",
      "displayName" : "TLS Configuration",
      "order" : "6",
      "default" : QUOTE({
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
      })
    }
  });
  /**
   * The IEC 61850 North Scheduler plugin interface
   */

  /**
   * The C API plugin information structure
   */
  static PLUGIN_INFORMATION info = {
      PLUGIN_NAME,       // Name
      VERSION,           // Version
      SP_CONTROL,        // Flags
      PLUGIN_TYPE_NORTH, // Type
      "0.0.1",           // Interface version
      default_config     // Configuration
  };

  /**
   * Return the information about this plugin
   */
  PLUGIN_INFORMATION *plugin_info()
  {
    return &info;
}


/**
 * Initialise the plugin with configuration.
 *
 * This function is called to get the plugin handle.
 */
PLUGIN_HANDLE plugin_init(ConfigCategory* configData)
{
  Iec61850Utility::log_info("Initializing the plugin");

	IEC61850Server* iec61850 = new IEC61850Server();

    if (iec61850) {
    	iec61850->configure(configData);
    }

	return (PLUGIN_HANDLE)iec61850;
}

/**
 * Send Readings data to historian server
 */
uint32_t plugin_send(const PLUGIN_HANDLE handle,
		     const vector<Reading *>& readings)
{
	IEC61850Server* iec61850 = (IEC61850Server *)handle;

	return iec61850->send(readings);
}

void plugin_register(PLUGIN_HANDLE handle,
		bool ( *write)(const char *name, const char *value, ControlDestination destination, ...),
		int (* operation)(char *operation, int paramCount, char *names[], char *parameters[], ControlDestination destination, ...))
{
    Iec61850Utility::log_info("plugin_register");

    IEC61850Server* iec61850 = (IEC61850Server*)handle;

    iec61850->registerControl(operation);
}


/**
 * Shutdown the plugin
 *
 * Delete allocated data
 *
 * @param handle    The plugin handle
 */
void plugin_shutdown(PLUGIN_HANDLE handle)
{
	IEC61850Server* iec61850 = (IEC61850Server*)handle;

    delete iec61850;
}
}
