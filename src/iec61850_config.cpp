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

void 
IEC61850Config::deleteExchangeDefinitions() {
  if (m_exchangeDefinitions == nullptr) return;

  delete m_exchangeDefinitions;

  m_exchangeDefinitions = nullptr;
}

IEC61850Config::~IEC61850Config() { deleteExchangeDefinitions(); }

bool 
IEC61850Config::isValidIPAddress(const std::string& addrStr) {
  // see
  // https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, addrStr.c_str(), &(sa.sin_addr));

  return (result == 1);
}

static ModelNode*
getDataObject(ModelNode* modelNode, std::string objRef)
{
  if(modelNode->modelType == DataObjectModelType){
        return modelNode;
  }

  ModelNode* parent = ModelNode_getParent(modelNode);

  if(parent==NULL){
    Logger::getLogger()->error("Invalid node at -> %s", objRef.c_str());
    return nullptr;
  }

  if (parent!=NULL && parent->modelType == DataObjectModelType) {
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
void IEC61850Config::importProtocolConfig(const std::string& protocolConfig) {
  m_protocolConfigComplete = false;

  Document document;

  if (document.Parse(const_cast<char*>(protocolConfig.c_str()))
          .HasParseError()) {
    Logger::getLogger()->fatal("Parsing error in protocol configuration");
    printf("Parsing error in protocol configuration\n");
    return;
  }

  if (!document.IsObject()) {
    return;
  }

  if (!document.HasMember("protocol_stack") ||
      !document["protocol_stack"].IsObject()) {
    return;
  }

  const Value& protocolStack = document["protocol_stack"];

  if (!protocolStack.HasMember("transport_layer") ||
      !protocolStack["transport_layer"].IsObject()) {
    Logger::getLogger()->fatal("transport layer configuration is missing");
    return;
  }

  const Value& transportLayer = protocolStack["transport_layer"];

  if (transportLayer.HasMember("port")) {
    if (transportLayer["port"].IsInt()) {
      int tcpPort = transportLayer["port"].GetInt();

      if (tcpPort > 0 && tcpPort < 65536) {
        m_tcpPort = tcpPort;
      } else {
        Logger::getLogger()->warn(
            "transport_layer.port value out of range-> using default port");
      }
    } else {
      printf("transport_layer.port has invalid type -> using default port\n");
      Logger::getLogger()->warn(
          "transport_layer.port has invalid type -> using default port");
    }
  }

  if (transportLayer.HasMember("srv_ip")) {
    if (transportLayer["srv_ip"].IsString()) {
      if (isValidIPAddress(transportLayer["srv_ip"].GetString())) {
        m_ip = transportLayer["srv_ip"].GetString();

        printf("Using local IP address: %s\n", m_ip.c_str());

        m_bindOnIp = true;
      } else {
        printf("transport_layer.srv_ip is not a valid IP address -> ignore\n");
        Logger::getLogger()->warn(
            "transport_layer.srv_ip has invalid type -> not using TLS");
      }
    }
  }

  if(transportLayer.HasMember("use_scheduler")){
    if (transportLayer["use_scheduler"].IsBool()){
      m_useScheduler = transportLayer["use_scheduler"].GetBool();
    }
    else{
      Logger::getLogger()->warn("use_scheduler has invalid type -> not using Scheduler");
    }
  }

  if(transportLayer.HasMember("tls")){
    if (transportLayer["tls"].IsBool()){
      m_useTLS = transportLayer["tls"].GetBool();
    }
    else{
      Logger::getLogger()->warn("tls has invalid type -> not using TLS");
    }
  }

}

void 
IEC61850Config::importExchangeConfig(const std::string& exchangeConfig, IedModel* model) {
  m_exchangeConfigComplete = false;

  deleteExchangeDefinitions();

  m_exchangeDefinitions = new std::map<std::string, std::shared_ptr<IEC61850Datapoint>>();
  m_exchangeDefinitionsObjRef = new std::map<std::string, std::shared_ptr<IEC61850Datapoint>>();
  
  Document document;

  if (document.Parse(const_cast<char*>(exchangeConfig.c_str()))
          .HasParseError()) {
    Logger::getLogger()->fatal("Parsing error in data exchange configuration");

    return;
  }

  if (!document.IsObject()) {
    Logger::getLogger()->error("NO DOCUMENT OBJECT FOR EXCHANGED DATA");
    return;
  }
  if (!document.HasMember(JSON_EXCHANGED_DATA) ||
      !document[JSON_EXCHANGED_DATA].IsObject()) {
    Logger::getLogger()->error("EXCHANGED DATA NOT AN OBJECT");
    return;
  }

  const Value& exchangeData = document[JSON_EXCHANGED_DATA];

  if (!exchangeData.HasMember(JSON_DATAPOINTS) ||
      !exchangeData[JSON_DATAPOINTS].IsArray()) {
        Logger::getLogger()->error("NO EXCHANGED DATA DATAPOINTS");
    return;
  }

  const Value& datapoints = exchangeData[JSON_DATAPOINTS];

  for (const Value& datapoint : datapoints.GetArray()) {
    if (!datapoint.IsObject()){
      Logger::getLogger()->error("DATAPOINT NOT AN OBJECT");
      return;
    }
    
    if (!datapoint.HasMember(JSON_LABEL) || !datapoint[JSON_LABEL].IsString()){
      Logger::getLogger()->error("DATAPOINT MISSING LABEL");
      return;
    }
    std::string label = datapoint[JSON_LABEL].GetString();

    if (!datapoint.HasMember(JSON_PROTOCOLS) ||
        !datapoint[JSON_PROTOCOLS].IsArray()){
      Logger::getLogger()->error("DATAPOINT MISSING PROTOCOLS ARRAY");
      return;
    }
    for (const Value& protocol : datapoint[JSON_PROTOCOLS].GetArray()) {
      if (!protocol.HasMember(JSON_PROT_NAME) ||
          !protocol[JSON_PROT_NAME].IsString()){
        Logger::getLogger()->error("PROTOCOL MISSING NAME");
        return;
      }
      std::string protocolName = protocol[JSON_PROT_NAME].GetString();

      if (protocolName != PROTOCOL_IEC61850){
            Logger::getLogger()->error("PROTOCOL NOT IEC61850, IT IS %s", protocolName.c_str());
        continue;
      } 
      if (!protocol.HasMember(JSON_PROT_OBJ_REF) ||
          !protocol[JSON_PROT_OBJ_REF].IsString()){
            Logger::getLogger()->error("PROTOCOL HAS NO OBJECT REFERENCE");
        return;
      }
      if (!protocol.HasMember(JSON_PROT_CDC) ||
          !protocol[JSON_PROT_CDC].IsString()){
          Logger::getLogger()->error("PROTOCOL HAS NO CDC");
          return;
      }

      const std::string objRef = protocol[JSON_PROT_OBJ_REF].GetString();
      const std::string typeIdStr = protocol[JSON_PROT_CDC].GetString();

      Logger::getLogger()->info("  address: %s type: %s label: %s \n ", objRef.c_str(), typeIdStr.c_str(), label.c_str());
      
      int typeId = IEC61850Datapoint::getCdcTypeFromString(typeIdStr);
      
      if(typeId == -1){
        Logger::getLogger()->error("Invalid CDC type, skip", typeIdStr.c_str());
        continue;
      }
      
      CDCTYPE cdcType = static_cast<CDCTYPE>(typeId);
      
      std::shared_ptr<DataAttributesDp> newDadp = std::make_shared<DataAttributesDp>();
      
      ModelNode* modelNode = IedModel_getModelNodeByObjectReference(model, objRef.c_str());
      
      if(modelNode == NULL){
        Logger::getLogger()->error("Model node for obj ref : %s not found -> continue", objRef.c_str());
        continue;
      }

      newDadp->value = (DataAttribute*)modelNode;

      ModelNode* dataObject = NULL;
      
      dataObject = getDataObject(modelNode, objRef);

      if (dataObject) {
          switch(cdcType){
            case SPS:
            case DPS:
            case INS:
            case ENS:{
              DataAttribute* stValDp =  (DataAttribute*)ModelNode_getChild(dataObject, "stVal");
              if(!stValDp){
                Logger::getLogger()->warn("%s has no stVal", objRef.c_str());
                continue;
              }
              newDadp->mmsVal = stValDp;
              break;
            }
            case MV:{
              DataAttribute* magDp =  (DataAttribute*)ModelNode_getChild(dataObject, "mag");
              if(!magDp){
                Logger::getLogger()->warn("%s has no mag", objRef.c_str());
                continue;
              }
              DataAttribute* iVal = (DataAttribute*)ModelNode_getChild(dataObject, "mag$i");
              if(iVal){
                newDadp->mmsVal = iVal;
                break;
              }
              DataAttribute* fVal = (DataAttribute*)ModelNode_getChild(dataObject, "mag$f");
              if(fVal){
                newDadp->mmsVal = fVal;
                break;
              }
              Logger::getLogger()->warn("%s has no mag value", objRef.c_str());
              break;
            }
            case SPC:
            case DPC:
            case INC:
            case APC:
            case BSC:
            {
              DataAttribute* ctlValDp =  (DataAttribute*)ModelNode_getChild(dataObject, "ctlVal");
              if(!ctlValDp){
                Logger::getLogger()->warn("%s has no stVal", objRef.c_str());
                continue;
              }
              newDadp->mmsVal = ctlValDp;
              break;
            }
                
          }
          newDadp->q = (DataAttribute*)ModelNode_getChild(dataObject, "q");
          newDadp->t = (DataAttribute*)ModelNode_getChild(dataObject, "t");
      }
     
      std::shared_ptr<IEC61850Datapoint> newDp = std::make_shared<IEC61850Datapoint>(label, objRef, cdcType, newDadp);

      m_exchangeDefinitions->insert({label,newDp});
      m_exchangeDefinitionsObjRef->insert({objRef,newDp});
      Logger::getLogger()->debug("Added datapoint %s %s", label.c_str(), objRef.c_str());
    }
  }

  m_exchangeConfigComplete = true;
}

void
IEC61850Config::importSchedulerConfig(const std::string& schedulerConfig){
  return;
}


void
IEC61850Config::importTlsConfig(const std::string& tlsConfig){
  return;
}


int 
IEC61850Config::TcpPort() {
  if (m_tcpPort == -1) {
    return 102;
  } else {
    return m_tcpPort;
  }
}

std::string 
IEC61850Config::ServerIp() {
  if (m_ip == "") {
    return "0.0.0.0";
  } else {
    return m_ip;
  }
}

std::shared_ptr<IEC61850Datapoint>
IEC61850Config::getDatapointByObjectReference(const std::string& objref){ 
    auto it = m_exchangeDefinitionsObjRef->find(objref);
    if (it != m_exchangeDefinitionsObjRef->end()) {
        return it->second;
    } else {
        return nullptr;
    }
    return nullptr;
}


