#include "iec61850_config.hpp"
#include "logger.h"
#include <iec61850.hpp>

#include <libiec61850/iec61850_config_file_parser.h>
#include <libiec61850/iec61850_server.h>
#include <plugin_api.h>
#include <config_category.h>


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string>
 
static bool running = true;

IEC61850Server::IEC61850Server() :
  m_config(new IEC61850Config()),
  m_log   (Logger::getLogger())
{ 
}

IEC61850Server::~IEC61850Server()
{
  stop();
  
  delete m_config;
}

void
IEC61850Server::setJsonConfig(const std::string& stackConfig,
                              const std::string& dataExchangeConfig,
                              const std::string& tlsConfig)
{
    // m_config->importExchangeConfig(dataExchangeConfig);
    m_config->importProtocolConfig(stackConfig);
    // m_config->importTlsConfig(tlsConfig);
    m_model = ConfigFileParser_createModelFromConfigFileEx(m_modelPath.c_str());
    
    if(!m_model){
      Logger::getLogger()->error("Invalid Model File Path");
      return;
    }

    m_server = IedServer_create(m_model);

    if(!m_server){
      Logger::getLogger()->error("Server couldn't be created");
      return;
    }

    IedServer_start(m_server,m_config->TcpPort());


    // m_exchangeDefinitions = *m_config->getExchangeDefinitions();
}

void
IEC61850Server::setJsonSchedulerConfig(const std::string& schedulerConfig)
{
  if(!m_server) return;
  
  // m_scheduler = Scheduler_create(m_model, m_server);

  
}
void
IEC61850Server::stop()
{
  if(m_started == true){
    m_started = false;
  }
  if(m_server){
    IedServer_stop(m_server);
    IedServer_destroy(m_server);
  }
}


void
IEC61850Server::registerControl(int (* operation)(char *operation, int paramCount, char *names[], char *parameters[], ControlDestination destination, ...))
{
    m_oper = operation;

    m_log->warn("RegisterControl is called"); //LCOV_EXCL_LINE
}

uint32_t
IEC61850Server::send(const std::vector<Reading*>& readings)
{
  return 0;
}

void
IEC61850Server::configure(const ConfigCategory* config)
{
    m_log->info("configure called"); //LCOV_EXCL_LINE

    if (config->itemExists("name"))
        m_name = config->getValue("name"); //LCOV_EXCL_LINE
    else
        m_log->error("Missing name in configuration"); //LCOV_EXCL_LINE

    if (config->itemExists("protocol_stack") == false) {
        m_log->error("Missing protocol configuration"); //LCOV_EXCL_LINE
        return;
    }
    //
    // if (config->itemExists("exchanged_data") == false) {
    //     m_log->error("Missing exchange data configuration"); //LCOV_EXCL_LINE
    //     return;
    // }
    //
    const std::string protocolStack = config->getValue("protocol_stack");
    //
    // const std::string dataExchange = config->getValue("exchanged_data");
    //
    // std::string tlsConfig = "";
    //
    // if (config->itemExists("tls_conf") == false) {
    //     m_log->error("Missing TLS configuration"); //LCOV_EXCL_LINE
    // }
    // else {
    //     tlsConfig = config->getValue("tls_conf");
    // }
    //
   //
    const std::string modelPath = config->getValue("modelPath");

    setModelPath(modelPath);
    setJsonConfig(protocolStack, "", "");
}

