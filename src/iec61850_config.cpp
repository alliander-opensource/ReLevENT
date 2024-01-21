/*
 * Copyright 2023 MZ Automation GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include <arpa/inet.h>

#include <libiec61850/iec61850_model.h>
#include <memory>
#include <string>

#include "iec61850.hpp"
#include "iec61850_config.hpp"
#include "iec61850_datapoint.hpp"

using namespace rapidjson;

#define JSON_EXCHANGED_DATA "exchanged_data"
#define JSON_DATAPOINTS "datapoints"
#define JSON_PROTOCOLS "protocols"
#define JSON_LABEL "label"

#define PROTOCOL_IEC61850 "iec61850"
#define JSON_PROT_NAME "name"
#define JSON_PROT_OBJ_REF "objref"
#define JSON_PROT_CDC "cdc"

IEC61850Config::IEC61850Config() {
  m_exchangeConfigComplete = false;
  m_protocolConfigComplete = false;
}

void IEC61850Config::deleteExchangeDefinitions() {
  if (m_exchangeDefinitions == nullptr)
    return;

  delete m_exchangeDefinitions;

  m_exchangeDefinitions = nullptr;

  if (m_exchangeDefinitionsObjRef == nullptr)
    return;

  delete m_exchangeDefinitionsObjRef;

  m_exchangeDefinitionsObjRef = nullptr;
}

void IEC61850Config::deleteScheduleTargetDefinitions() 
{
  if (m_schedulerTargets == nullptr)
    return;

  delete m_schedulerTargets;

  m_schedulerTargets = nullptr;
}

IEC61850Config::~IEC61850Config()
{ 
    deleteExchangeDefinitions();
    deleteScheduleTargetDefinitions();
}

bool IEC61850Config::isValidIPAddress(const std::string &addrStr) {
  // see
  // https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, addrStr.c_str(), &(sa.sin_addr));

  return (result == 1);
}

std::map<std::string, CDCTYPE> cdcStrMap = {
    {"SpsTyp", SPS}, {"DpsTyp", DPS}, {"BscTyp", BSC}, {"MvTyp", MV},
    {"SpcTyp", SPC}, {"DpcTyp", DPC}, {"ApcTyp", APC}, {"IncTyp", INC}};

int getCdcTypeFromString(const std::string &cdc) {
  Iec61850Utility::log_debug(" O ");
  auto it = cdcStrMap.find(cdc);
  if (it != cdcStrMap.end()) {
    return it->second;
  }
  return -1;
}

static ModelNode *getDataObject(ModelNode *modelNode, std::string objRef) {
  if (modelNode->modelType == DataObjectModelType) {
    return modelNode;
  }

  ModelNode *parent = ModelNode_getParent(modelNode);

  if (parent == NULL) {
    Iec61850Utility::log_error("Invalid node at -> %s", objRef.c_str());
    return nullptr;
  }

  if (parent != NULL && parent->modelType == DataObjectModelType) {
    return parent;
  }

  else {
    parent = ModelNode_getParent(parent);
    if (parent && parent->modelType == DataObjectModelType) {
      return parent;
    }
  }

  return nullptr;
}
void IEC61850Config::importProtocolConfig(const std::string &protocolConfig)
{
  m_protocolConfigComplete = false;

  Document document;

  if (document.Parse(const_cast<char *>(protocolConfig.c_str()))
          .HasParseError()) {
    Iec61850Utility::log_fatal("Parsing error in protocol configuration");
    Iec61850Utility::log_debug("Parsing error in protocol configuration\n");
    return;
  }

  if (!document.IsObject()) {
    return;
  }

  if (!document.HasMember("protocol_stack") ||
      !document["protocol_stack"].IsObject()) {
    return;
  }

  const Value &protocolStack = document["protocol_stack"];

  if (!protocolStack.HasMember("transport_layer") ||
      !protocolStack["transport_layer"].IsObject()) {
    Iec61850Utility::log_fatal("transport layer configuration is missing");
    return;
  }

  const Value &transportLayer = protocolStack["transport_layer"];

  if (transportLayer.HasMember("port")) {
    if (transportLayer["port"].IsInt()) {
      int tcpPort = transportLayer["port"].GetInt();

      if (tcpPort > 0 && tcpPort < 65536) {
        m_tcpPort = tcpPort;
      } else {
        Iec61850Utility::log_warn(
            "transport_layer.port value out of range-> using default port");
      }
    } else {
      Iec61850Utility::log_debug(
          "transport_layer.port has invalid type -> using default port\n");
      Iec61850Utility::log_warn(
          "transport_layer.port has invalid type -> using default port");
    }
  }

  if (transportLayer.HasMember("srv_ip")) {
    if (transportLayer["srv_ip"].IsString()) {
      if (isValidIPAddress(transportLayer["srv_ip"].GetString())) {
        m_ip = transportLayer["srv_ip"].GetString();

        Iec61850Utility::log_debug("Using local IP address: %s\n",
                                   m_ip.c_str());

        m_bindOnIp = true;
      } else {
        printf("transport_layer.srv_ip is not a valid IP address -> ignore\n");
        Iec61850Utility::log_warn(
            "transport_layer.srv_ip has invalid type -> not using TLS");
      }
    }
  }

  if (transportLayer.HasMember("use_scheduler")) {
    if (transportLayer["use_scheduler"].IsBool()) {
      m_useScheduler = transportLayer["use_scheduler"].GetBool();
    } else {
      Iec61850Utility::log_warn(
          "use_scheduler has invalid type -> not using Scheduler");
    }
  }

  if (transportLayer.HasMember("tls")) {
    if (transportLayer["tls"].IsBool()) {
      m_useTLS = transportLayer["tls"].GetBool();
    } else {
      Iec61850Utility::log_warn("tls has invalid type -> not using TLS");
    }
  }
}

void IEC61850Config::importExchangeConfig(const std::string &exchangeConfig,
                                          IedModel *model) {
  m_exchangeConfigComplete = false;

  deleteExchangeDefinitions();

  m_exchangeDefinitions =
      new std::map<std::string, std::shared_ptr<IEC61850Datapoint>>();
  m_exchangeDefinitionsObjRef =
      new std::map<std::string, std::shared_ptr<IEC61850Datapoint>>();

  Document document;

  if (document.Parse(const_cast<char *>(exchangeConfig.c_str()))
          .HasParseError()) {
    Iec61850Utility::log_fatal("Parsing error in data exchange configuration");

    return;
  }

  if (!document.IsObject()) {
    Iec61850Utility::log_error("NO DOCUMENT OBJECT FOR EXCHANGED DATA");
    return;
  }
  if (!document.HasMember(JSON_EXCHANGED_DATA) ||
      !document[JSON_EXCHANGED_DATA].IsObject()) {
    Iec61850Utility::log_error("EXCHANGED DATA NOT AN OBJECT");
    return;
  }

  const Value &exchangeData = document[JSON_EXCHANGED_DATA];

  if (!exchangeData.HasMember(JSON_DATAPOINTS) ||
      !exchangeData[JSON_DATAPOINTS].IsArray()) {
    Iec61850Utility::log_error("NO EXCHANGED DATA DATAPOINTS");
    return;
  }

  const Value &datapoints = exchangeData[JSON_DATAPOINTS];

  for (const Value &datapoint : datapoints.GetArray()) {
    if (!datapoint.IsObject()) {
      Iec61850Utility::log_error("DATAPOINT NOT AN OBJECT");
      return;
    }

    if (!datapoint.HasMember(JSON_LABEL) || !datapoint[JSON_LABEL].IsString()) {
      Iec61850Utility::log_error("DATAPOINT MISSING LABEL");
      return;
    }
    std::string label = datapoint[JSON_LABEL].GetString();

    if (!datapoint.HasMember(JSON_PROTOCOLS) ||
        !datapoint[JSON_PROTOCOLS].IsArray()) {
      Iec61850Utility::log_error("DATAPOINT MISSING PROTOCOLS ARRAY");
      return;
    }
    for (const Value &protocol : datapoint[JSON_PROTOCOLS].GetArray()) {
      if (!protocol.HasMember(JSON_PROT_NAME) ||
          !protocol[JSON_PROT_NAME].IsString()) {
        Iec61850Utility::log_error("PROTOCOL MISSING NAME");
        return;
      }
      std::string protocolName = protocol[JSON_PROT_NAME].GetString();

      if (protocolName != PROTOCOL_IEC61850) {
        Iec61850Utility::log_error("PROTOCOL NOT IEC61850, IT IS %s",
                                   protocolName.c_str());
        continue;
      }
      if (!protocol.HasMember(JSON_PROT_OBJ_REF) ||
          !protocol[JSON_PROT_OBJ_REF].IsString()) {
        Iec61850Utility::log_error("PROTOCOL HAS NO OBJECT REFERENCE");
        return;
      }
      if (!protocol.HasMember(JSON_PROT_CDC) ||
          !protocol[JSON_PROT_CDC].IsString()) {
        Iec61850Utility::log_error("PROTOCOL HAS NO CDC");
        return;
      }

      const std::string objRef = protocol[JSON_PROT_OBJ_REF].GetString();
      const std::string typeIdStr = protocol[JSON_PROT_CDC].GetString();

      Iec61850Utility::log_info("  address: %s type: %s label: %s \n ",
                                objRef.c_str(), typeIdStr.c_str(),
                                label.c_str());
      int typeId = IEC61850Datapoint::getCdcTypeFromString(typeIdStr);

      if (typeId == -1) {
        Iec61850Utility::log_error("Invalid CDC type, skip", typeIdStr.c_str());
        continue;
      }

      CDCTYPE cdcType = static_cast<CDCTYPE>(typeId);

      std::shared_ptr<DataAttributesDp> newDadp =
          std::make_shared<DataAttributesDp>();

      ModelNode *modelNode =
          IedModel_getModelNodeByObjectReference(model, objRef.c_str());

      if (modelNode == NULL) {
        Iec61850Utility::log_error(
            "Model node for obj ref : %s not found -> continue",
            objRef.c_str());
        continue;
      }

      newDadp->value = (DataAttribute *)modelNode;

      ModelNode *dataObject = NULL;

      dataObject = getDataObject(modelNode, objRef);

      if (dataObject) {
        switch (cdcType) {
        case SPS:
        case DPS:
        case INS:
        case ENS: {
          DataAttribute *stValDp =
              (DataAttribute *)ModelNode_getChild(dataObject, "stVal");
          if (!stValDp) {
            Iec61850Utility::log_warn("%s has no stVal", objRef.c_str());
            continue;
          }
          newDadp->mmsVal = stValDp;
          break;
        }
        case MV: {
          DataAttribute *magDp =
              (DataAttribute *)ModelNode_getChild(dataObject, "mag");
          if (!magDp) {
            Iec61850Utility::log_warn("%s has no mag", objRef.c_str());
            continue;
          }
          DataAttribute *iVal =
              (DataAttribute *)ModelNode_getChild(dataObject, "mag$i");
          if (iVal) {
            newDadp->mmsVal = iVal;
            break;
          }
          DataAttribute *fVal =
              (DataAttribute *)ModelNode_getChild(dataObject, "mag$f");
          if (fVal) {
            newDadp->mmsVal = fVal;
            break;
          }
          Iec61850Utility::log_warn("%s has no mag value", objRef.c_str());
          break;
        }
        case DPC:
        case INC:
        case SPC: {
          DataAttribute *stValDp =
              (DataAttribute *)ModelNode_getChild(dataObject, "stVal");
          if (!stValDp) {
            Iec61850Utility::log_warn("%s has no stVal", objRef.c_str());
            continue;
          }
          newDadp->mmsVal = stValDp;
          break;
        }

        case APC: {
          DataAttribute *mxValDp =
              (DataAttribute *)ModelNode_getChild(dataObject, "mxVal");

          if (mxValDp) {
            DataAttribute *iVal =
                (DataAttribute *)ModelNode_getChild(dataObject, "mxVal$i");
            if (iVal) {
              newDadp->mmsVal = iVal;
              break;
            }
            DataAttribute *fVal =
                (DataAttribute *)ModelNode_getChild(dataObject, "mxVal$f");
  
            if (fVal) {
              newDadp->mmsVal = fVal;
              break;
            }
          }

          ModelNode *operationObject = ModelNode_getChild(dataObject, "Oper");

          if (operationObject) {
            DataAttribute *ctlValDp =
                (DataAttribute *)ModelNode_getChild(operationObject, "ctlVal");

            if (!ctlValDp) {
              Iec61850Utility::log_warn("%s has no ctlVal", objRef.c_str());
              continue;
            }

            newDadp->mmsVal = ctlValDp;
          }

          Iec61850Utility::log_warn("%s has no mxVal or Oper value", objRef.c_str());
          break;
        }
        case BSC: {
          ModelNode *operationObject = ModelNode_getChild(dataObject, "Oper");

          if (operationObject) {
            DataAttribute *ctlValDp =
                (DataAttribute *)ModelNode_getChild(operationObject, "ctlVal");

            if (!ctlValDp) {
              Iec61850Utility::log_warn("%s has no ctlVal", objRef.c_str());
              continue;
            }

            newDadp->mmsVal = ctlValDp;
          }
          else {
            Iec61850Utility::log_warn("%s has no Oper", objRef.c_str());
            continue;
          }

          break;
        }
        }
        newDadp->q = (DataAttribute *)ModelNode_getChild(dataObject, "q");
        newDadp->t = (DataAttribute *)ModelNode_getChild(dataObject, "t");
      }

      std::shared_ptr<IEC61850Datapoint> newDp =
          std::make_shared<IEC61850Datapoint>(label, objRef, cdcType, newDadp);

      m_exchangeDefinitions->insert({label, newDp});
      m_exchangeDefinitionsObjRef->insert({objRef, newDp});
      Iec61850Utility::log_debug("Added datapoint %s %s", label.c_str(),
                                 objRef.c_str());
    }
  }

  m_exchangeConfigComplete = true;
}

void IEC61850Config::importSchedulerConfig(const std::string &schedulerConfig,
                                           Scheduler sched)
{
  using namespace rapidjson;

  deleteScheduleTargetDefinitions();

  Document document;
  if (document.Parse(schedulerConfig.c_str()).HasParseError()) {
    Iec61850Utility::log_fatal("Parsing error in scheduler configuration");
    return;
  }

  if (!document.IsObject()) {
    Iec61850Utility::log_fatal(
        "Invalid scheduler configuration: Not an object");
    return;
  }

  if (!document.HasMember("scheduler_conf")) {
    Iec61850Utility::log_fatal(
        "Missing 'scheduler_conf' in scheduler configuration");
    return;
  }

  const Value &schedulerConf = document["scheduler_conf"];
  if (!schedulerConf.IsObject()) {
    Iec61850Utility::log_fatal(
        "'scheduler_conf' should be an object in scheduler configuration");
    return;
  }

  if (schedulerConf.HasMember("targets"))
  {
    const Value &targets = schedulerConf["targets"];

    if (!targets.IsArray()) {
      Iec61850Utility::log_fatal(
          "'targets' should be an array in 'scheduler_conf'");
      return;
    }

    m_schedulerTargets =
        new std::map<std::string, std::shared_ptr<SchedulerTarget>>();

    for (SizeType i = 0; i < targets.Size(); i++) {
      const Value &target = targets[i];

      if (!target.IsObject()) {
        Iec61850Utility::log_fatal("target configuration is not an object");
        continue;
      }

      if (!target.HasMember("schedCtrlRef")) {
        Iec61850Utility::log_fatal(
            "target configuration is missing mandatory member schedCtrlRef");
        continue;
      }

      const char *schedCtrlRef = target["schedCtrlRef"].GetString();


      std::shared_ptr<SchedulerTarget> schedTarget =
          std::make_shared<SchedulerTarget>();

      schedTarget->schedControllerRef = schedCtrlRef;

      m_schedulerTargets->insert({schedTarget->schedControllerRef, schedTarget});

      if (target.HasMember("forwardOutput")) {
        schedTarget->forwardOutput = target["forwardOutput"].GetBool();
      }

      if (target.HasMember("forwardSchedule")) {
        schedTarget->forwardSchedule = target["forwardSchedule"].GetBool();
      }

      if (target.HasMember("forwardScheduleInterval")) {
        schedTarget->forwardScheduleInterval =
            target["forwardScheduleInterval"].GetInt();
      }

      if (target.HasMember("forwardSchedulePeriod")) {
        schedTarget->forwardSchedulePeriod =
            target["forwardSchedulePeriod"].GetInt();
      }
    }
  }

  if (!schedulerConf.HasMember("schedules")) {
    Iec61850Utility::log_fatal("Missing 'schedules' in 'scheduler_conf'");
    return;
  }

  const Value &schedules = schedulerConf["schedules"];
  if (!schedules.IsArray()) {
    Iec61850Utility::log_fatal(
        "'schedules' should be an array in 'scheduler_conf'");
    return;
  }

  for (SizeType i = 0; i < schedules.Size(); i++) {
    const Value &schedule = schedules[i];
    if (!schedule.IsObject()) {
      Iec61850Utility::log_fatal("Schedule configuration is not an object");
      continue;
    }

    if (!schedule.HasMember("scheduleRef") ||
        !schedule.HasMember("enableScheduleControl") ||
        !schedule.HasMember("enabled")) {
      Iec61850Utility::log_fatal(
          "Schedule configuration is missing mandatory members");
      continue;
    }

    const char *scheduleRef = schedule["scheduleRef"].GetString();
    bool enableScheduleControl = schedule["enableScheduleControl"].GetBool();
    Scheduler_enableScheduleControl(sched, scheduleRef, enableScheduleControl);

    if (!schedule.HasMember("parameters")) {
      Iec61850Utility::log_fatal(
          "Missing 'parameters' in schedule configuration");
      continue;
    }

    const Value &parameters = schedule["parameters"];
    if (!parameters.IsArray()) {
      Iec61850Utility::log_fatal(
          "'parameters' should be an array in schedule configuration");
      continue;
    }

    for (SizeType j = 0; j < parameters.Size(); j++) {
      const Value &parameter = parameters[j];
      if (!parameter.IsObject()) {
        Iec61850Utility::log_fatal("Parameter configuration is not an object");
        continue;
      }

      if (!parameter.HasMember("parameter") ||
          !parameter.HasMember("enableWriteAccess")) {
        Iec61850Utility::log_fatal(
            "Parameter configuration is missing mandatory members");
        continue;
      }

      std::string paramRef = parameter["parameter"].GetString();
      bool enableWriteAccess = parameter["enableWriteAccess"].GetBool();

      if (paramRef == "SCHED_PARAM_STR_TM") {
        Scheduler_enableWriteAccessToParameter(
            sched, scheduleRef, SCHED_PARAM_STR_TM, enableWriteAccess);
      } else if (paramRef == "SCHED_PARAM_SCHD_PRIO") {
        Scheduler_enableWriteAccessToParameter(
            sched, scheduleRef, SCHED_PARAM_SCHD_PRIO, enableWriteAccess);
      } else if (paramRef == "SCHED_PARAM_SCHD_REUSE") {
        Scheduler_enableWriteAccessToParameter(
            sched, scheduleRef, SCHED_PARAM_SCHD_REUSE, enableWriteAccess);
      } else {
        Iec61850Utility::log_fatal("Unknown parameter reference: %s", paramRef);
      }
    }

    bool enabled = schedule["enabled"].GetBool();

    Iec61850Utility::log_info("Enable schedule %s (%i)", scheduleRef, enabled);

    if (Scheduler_enableSchedule(sched, scheduleRef, enabled) == false) {
      Iec61850Utility::log_error("Failed to enable schedule %s (%i)", scheduleRef, enabled);
    }
  }

  if (!document.HasMember("storage")) {
    Iec61850Utility::log_warn("'storage' not configured");
    return;
  }

  const Value &storageConf = document["storage"];
  if (!storageConf.IsObject()) {
    Iec61850Utility::log_fatal(
        "'storage' should be an object in scheduler configuration");
    return;
  }

  if (!storageConf.HasMember("databaseUri") ||
      !storageConf["databaseUri"].IsString()) {
    Iec61850Utility::log_fatal(
        "Missing or invalid 'databaseUri' in storage configuration");
    return;
  }

  const char *databaseUri = storageConf["databaseUri"].GetString();

  const char **parameters = NULL;
  int numberOfParameters = 0;

  if (storageConf.HasMember("parameters") &&
      storageConf["parameters"].IsArray()) {
    const Value &parametersArray = storageConf["parameters"];
    numberOfParameters = static_cast<int>(parametersArray.Size());
    std::vector<const char *> parametersVector(numberOfParameters);

    for (SizeType i = 0; i < parametersArray.Size(); i++) {
      if (!parametersArray[i].IsString()) {
        Iec61850Utility::log_fatal("Parameter is not a string");
        return;
      }
      parametersVector[i] = parametersArray[i].GetString();
    }

    parameters = &parametersVector[0];
  }

  Scheduler_initializeStorage(sched, databaseUri, numberOfParameters,
                              parameters);
}

void IEC61850Config::importTlsConfig(const std::string &tlsConfig) { return; }

int IEC61850Config::TcpPort() {
  if (m_tcpPort == -1) {
    return 102;
  } else {
    return m_tcpPort;
  }
}

std::string IEC61850Config::ServerIp() {
  if (m_ip == "") {
    return "0.0.0.0";
  } else {
    return m_ip;
  }
}

std::shared_ptr<IEC61850Datapoint>
IEC61850Config::getDatapointByObjectReference(const std::string &objref) {
  auto it = m_exchangeDefinitionsObjRef->find(objref);
  if (it != m_exchangeDefinitionsObjRef->end()) {
    return it->second;
  } else {
    return nullptr;
  }
  return nullptr;
}
