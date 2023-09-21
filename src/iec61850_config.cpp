#include <arpa/inet.h>
#include <string>

#include "iec61850_config.hpp"

using namespace rapidjson;

#define JSON_EXCHANGED_DATA "exchanged_data"
#define JSON_DATAPOINTS "datapoints"
#define JSON_PROTOCOLS "protocols"
#define JSON_LABEL "label"

#define PROTOCOL_IEC61850 "iec61850"
#define JSON_PROT_NAME "name"
#define JSON_PROT_ADDR "address"
#define JSON_PROT_TYPEID "typeid"
#define JSON_PROT_GI_GROUPS "gi_groups"

IEC61850Config::IEC61850Config()
{
    m_exchangeConfigComplete = false;
    m_protocolConfigComplete = false;
}

void
IEC61850Config::deleteExchangeDefinitions()
{
  if(m_exchangeDefinitions == nullptr) return;

  for (auto const& exchangeDefintions : *m_exchangeDefinitions) {
      for (auto const& dpPair : exchangeDefintions.second) {
          IEC61850Datapoint* dp = dpPair.second;

          delete dp;
      }
  }

  delete m_exchangeDefinitions;

  m_exchangeDefinitions = nullptr;
}

IEC61850Config::~IEC61850Config()
{
    deleteExchangeDefinitions();
}

bool
IEC61850Config::isValidIPAddress(const std::string& addrStr)
{
    // see https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, addrStr.c_str(), &(sa.sin_addr));

    return (result == 1);
}

void
IEC61850Config::importProtocolConfig(const std::string& protocolConfig)
{
  m_protocolConfigComplete = false;

  Document document;

  if (document.Parse(const_cast<char*>(protocolConfig.c_str())).HasParseError()) {
      Logger::getLogger()->fatal("Parsing error in protocol configuration");
      printf("Parsing error in protocol configuration\n");
      return;
  }

  if (!document.IsObject()) {
      return;
  }

  if (!document.HasMember("protocol_stack") || !document["protocol_stack"].IsObject()) {
      return;
  }

  const Value& protocolStack = document["protocol_stack"];

  if (!protocolStack.HasMember("transport_layer") || !protocolStack["transport_layer"].IsObject()) {
      Logger::getLogger()->fatal("transport layer configuration is missing");
      return;
  }
  
  const Value& transportLayer = protocolStack["transport_layer"];
  
  if (transportLayer.HasMember("port")) {
        if (transportLayer["port"].IsInt()){
            int tcpPort = transportLayer["port"].GetInt();

            if (tcpPort > 0 && tcpPort < 65536) {
                m_tcpPort = tcpPort;
            }
            else {
                Logger::getLogger()->warn("transport_layer.port value out of range-> using default port");
            }
        }
        else {
            printf("transport_layer.port has invalid type -> using default port\n");
            Logger::getLogger()->warn("transport_layer.port has invalid type -> using default port");
        }
    }

  if (transportLayer.HasMember("srv_ip")) {
        if (transportLayer["srv_ip"].IsString()) {

            if (isValidIPAddress(transportLayer["srv_ip"].GetString())) {
                m_ip = transportLayer["srv_ip"].GetString();

                printf("Using local IP address: %s\n", m_ip.c_str());

                m_bindOnIp = true;
            }
            else {
                printf("transport_layer.srv_ip is not a valid IP address -> ignore\n");
                Logger::getLogger()->warn("transport_layer.srv_ip has invalid type -> not using TLS");
            }

        }
  }
}


int
IEC61850Config::TcpPort()
{
    if (m_tcpPort == -1) {
        return 102;
    }
    else {
        return m_tcpPort;
    }
}


std::string
IEC61850Config::ServerIp()
{
  if(m_ip == ""){
      return "0.0.0.0";
  }
  else{
      return m_ip;
  }
}  
