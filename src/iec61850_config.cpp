#include <arpa/inet.h>

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
  if (m_exchangeDefinitions == nullptr) return;

  delete m_exchangeDefinitions;

  m_exchangeDefinitions = nullptr;
}

IEC61850Config::~IEC61850Config() { deleteExchangeDefinitions(); }

bool IEC61850Config::isValidIPAddress(const std::string& addrStr) {
  // see
  // https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, addrStr.c_str(), &(sa.sin_addr));

  return (result == 1);
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
      Logger::getLogger()->warn("tls has invalid type -> not using Scheduler");
    }
  }

}

void IEC61850Config::importExchangeConfig(const std::string& exchangeConfig) {
  m_exchangeConfigComplete = false;

  deleteExchangeDefinitions();

  m_exchangeDefinitions =
      new std::map<std::string, std::shared_ptr<IEC61850Datapoint>>();

  Document document;

  if (document.Parse(const_cast<char*>(exchangeConfig.c_str()))
          .HasParseError()) {
    Logger::getLogger()->fatal("Parsing error in data exchange configuration");

    return;
  }

  if (!document.IsObject()) return;

  if (!document.HasMember(JSON_EXCHANGED_DATA) ||
      !document[JSON_EXCHANGED_DATA].IsObject()) {
    return;
  }

  const Value& exchangeData = document[JSON_EXCHANGED_DATA];

  if (!exchangeData.HasMember(JSON_DATAPOINTS) ||
      !exchangeData[JSON_DATAPOINTS].IsArray()) {
    return;
  }

  const Value& datapoints = exchangeData[JSON_DATAPOINTS];

  for (const Value& datapoint : datapoints.GetArray()) {
    if (!datapoint.IsObject()) return;

    if (!datapoint.HasMember(JSON_LABEL) || !datapoint[JSON_LABEL].IsString())
      return;

    std::string label = datapoint[JSON_LABEL].GetString();

    if (!datapoint.HasMember(JSON_PROTOCOLS) ||
        !datapoint[JSON_PROTOCOLS].IsArray())
      return;

    for (const Value& protocol : datapoint[JSON_PROTOCOLS].GetArray()) {
      if (!protocol.HasMember(JSON_PROT_NAME) ||
          !protocol[JSON_PROT_NAME].IsString())
        return;

      std::string protocolName = protocol[JSON_PROT_NAME].GetString();

      if (protocolName == PROTOCOL_IEC61850) {
        if (!protocol.HasMember(JSON_PROT_OBJ_REF) ||
            !protocol[JSON_PROT_OBJ_REF].IsString())
          return;
        if (!protocol.HasMember(JSON_PROT_CDC) ||
            !protocol[JSON_PROT_CDC].IsString())
          return;

        const std::string objRef = protocol[JSON_PROT_OBJ_REF].GetString();
        const std::string typeIdStr = protocol[JSON_PROT_CDC].GetString();

        Logger::getLogger()->debug("  address: %s type: %s\n", objRef.c_str(), typeIdStr.c_str());
        
        int typeId = IEC61850Datapoint::getCdcTypeFromString(typeIdStr);
        
        if(typeId == -1){
          Logger::getLogger()->error("Invalid CDC type, skip", typeIdStr.c_str());
          continue;
        }
        
        IEC61850Datapoint::CDCTYPE cdcType = static_cast<IEC61850Datapoint::CDCTYPE>(typeId);

        std::shared_ptr<IEC61850Datapoint> newDp = std::make_shared<IEC61850Datapoint>(label, objRef, cdcType);

        m_exchangeDefinitions->insert({objRef,newDp});
     }
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
