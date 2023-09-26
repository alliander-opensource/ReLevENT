#include "iec61850_config.hpp"
#include "iec61850_datapoint.hpp"
#include "logger.h"
#include <cstdint>
#include <iec61850.hpp>

#include <libiec61850/iec61850_config_file_parser.h>
#include <libiec61850/iec61850_model.h>
#include <libiec61850/iec61850_server.h>
#include <plugin_api.h>
#include <config_category.h>


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <vector>
 
static Datapoint*
getChild(Datapoint* dp, const std::string& name)
{
    Datapoint* childDp = nullptr;

    DatapointValue& dpv = dp->getData();

    if (dpv.getType() == DatapointValue::T_DP_DICT) {
        std::vector<Datapoint*>* datapoints = dpv.getDpVec();

        for (Datapoint* child : *datapoints) {
            if (child->getName() == name) {
                childDp = child;
                break;
            }
        }
    }

    return childDp;
}

static const std::string
getValueStr(Datapoint* dp)
{
    DatapointValue& dpv = dp->getData();

    if (dpv.getType() == DatapointValue::T_STRING) {
        return dpv.toStringValue();
    }
    else {
        throw IEC61850ServerException("datapoint " + dp->getName() + " has mot a std::string value");
    }
}

static const std::string
getChildValueStr(Datapoint* dp, const std::string& name)
{
    Datapoint* childDp = getChild(dp, name);

    if (childDp) {
        return getValueStr(childDp);
    }
    else {
        throw IEC61850ServerException("No such child: " + name);
    }
}

static long
getValueInt(Datapoint* dp)
{
    DatapointValue& dpv = dp->getData();

    if (dpv.getType() == DatapointValue::T_INTEGER) {
        return dpv.toInt();
    }
    else {
        throw IEC61850ServerException("datapoint " + dp->getName() + " has not an int value");
    }
}

static int
getChildValueInt(Datapoint* dp, const std::string& name)
{
    Datapoint* childDp = getChild(dp, name);

    if (childDp) {
        return getValueInt(childDp);
    }
    else {
        throw IEC61850ServerException("No such child: " + name);
    }
}

static float
getValueFloat(Datapoint* dp)
{
    DatapointValue& dpv = dp->getData();

    if (dpv.getType() == DatapointValue::T_FLOAT) {
        return (float) dpv.toDouble();
    }
    else {
        throw IEC61850ServerException("datapoint " + dp->getName() + " has not a float value");
    }
}

static Datapoint*
getCdc(Datapoint* dp)
{
    Datapoint* cdcDp = nullptr;

    DatapointValue& dpv = dp->getData();

    if (dpv.getType() == DatapointValue::T_DP_DICT) {
        std::vector<Datapoint*>* datapoints = dpv.getDpVec();

        for (Datapoint* child : *datapoints) {
            if (child->getName() == "SpsTyp") {
                cdcDp = child;
                break;
            }
            else if (child->getName() == "MvTyp") {
                cdcDp = child;
                break;
            }
            else if (child->getName() == "DpsTyp") {
                cdcDp = child;
                break;
            }
            else if (child->getName() == "SpcTyp") {
                cdcDp = child;
                break;
            }
            else if (child->getName() == "DpcTyp") {
                cdcDp = child;
                break;
            }
            else if (child->getName() == "IncTyp") {
                cdcDp = child;
                break;
            }
            else if (child->getName() == "ApcTyp") {
                cdcDp = child;
                break;
            }
            else if (child->getName() == "BscTyp") {
                cdcDp = child;
                break;
            }
        }
    }

    return cdcDp;
}


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
                              const std::string& tlsConfig,
                              const std::string& schedulerConfig)
{
    m_model = ConfigFileParser_createModelFromConfigFileEx(m_modelPath.c_str());
  
    m_config->importExchangeConfig(dataExchangeConfig, m_model);
    m_config->importProtocolConfig(stackConfig);

    if(m_config->TLSEnabled() && tlsConfig != "")
       m_config->importTlsConfig(tlsConfig);
  
    if(!m_model){
      m_log->error("Invalid Model File Path");
      return;
    }

    m_server = IedServer_create(m_model);

    if(!m_server){
      m_log->error("Server couldn't be created");
      return;
    }
    
    if(m_config->schedulerEnabled() && schedulerConfig != ""){
      m_scheduler = Scheduler_create(m_model,m_server);
      m_config->importSchedulerConfig(schedulerConfig);
      m_log->warn("Scheduler created");
    }

    IedServer_start(m_server,m_config->TcpPort());
    
    if(IedServer_isRunning(m_server)){
      m_log->warn("SERVER RUNNING on port " + std::to_string(m_config->TcpPort()));
    }
    else{
      m_log->warn("SERVER NOT RUNNING");
    }

    m_exchangeDefinitions = *m_config->getExchangeDefinitions();
}

void
IEC61850Server::stop()
{
  if(m_started == true){
    m_started = false;
  }
  if(m_scheduler){
    Scheduler_destroy(m_scheduler);
  }
  if(m_server){
    IedServer_stop(m_server);
    IedServer_destroy(m_server);
    running = false;
  }
}

const std::string
IEC61850Server::getObjRefFromID(const std::string& id){
  std::shared_ptr<IEC61850Datapoint> dp = m_exchangeDefinitions.at(id);
  
  if(!dp) return "";
  
  return dp->getObjRef();
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
  int n = 0;

  int readingsSent = 0;

  for (auto reading = readings.cbegin(); reading != readings.cend(); reading++)
  {
    
    std::vector<Datapoint*>& dataPoints = (*reading)->getReadingData();
    std::string assetName = (*reading)->getAssetName();

    for(Datapoint* dp: dataPoints){

      if (dp->getName() == "PIVOT"){
        
        if(!IedServer_isRunning(m_server))
          continue;
        
        DatapointValue dpv = dp->getData();

        std::vector<Datapoint*>* sdp = dpv.getDpVec(); 
        
        Datapoint* value = nullptr;
        
        IEC61850Datapoint::ROOT root;
        
        Datapoint* rootDp;

        for (Datapoint* child : *sdp) {
            if (child->getName() == "GTIS") {
                root = IEC61850Datapoint::GTIS;
                rootDp = child;
                break;
            }
            else if (child->getName() == "GTIM") {
                root = IEC61850Datapoint::GTIM;
                rootDp = child;
                break;
            }
            else if (child->getName() == "GTIC") {
                root = IEC61850Datapoint::GTIC;
                rootDp = child;
                break;
            }
        }
        
        Datapoint* identifierDp = getChild(rootDp,"Identifier");
        
        if(!identifierDp){
          m_log->error("Identifier missing");
          continue;
        }
        
        const std::string objRef = getObjRefFromID(getValueStr(identifierDp));
        
        if(objRef == ""){
          m_log->error("objRef for label %s not found -> continue", getValueStr(identifierDp).c_str());
        }

        // Datapoint* quality = nullptr;
        // 
        // Datapoint* confirmation = nullptr;
        //
        // Datapoint* tmOrg = nullptr;
        //
        // Datapoint* tmValidity = nullptr;


        Datapoint* cdcDp = getCdc(rootDp);
        
        if(!cdcDp){
          m_log->error("No cdc found or cdc type invalid");
        }

        const std::string cdcName = cdcDp->getName();
        
        int cdcTypeInt = IEC61850Datapoint::getCdcTypeFromString(cdcName);

        if(cdcTypeInt == -1){
          m_log->error("Invalid cdc type -> %s ", cdcName.c_str());
          continue;
        }
        
        IEC61850Datapoint::CDCTYPE cdcType = static_cast<IEC61850Datapoint::CDCTYPE>(cdcTypeInt);
        
        if(IEC61850Datapoint::getRootFromCDC(cdcType)!= root){
          m_log->error("CDC type does not Match Root type -> %s %s", cdcName.c_str(), rootDp->getName().c_str());
          continue;
        }

        switch(cdcType){
          case IEC61850Datapoint::SPS:
          case IEC61850Datapoint::DPS:
          {
            value = getChild(cdcDp, "stVal");    
            break;  
          }
          case IEC61850Datapoint::MV:
          {
            Datapoint* magDp = getChild(cdcDp, "mag");  
            
            if(getChild(magDp, "i")){
              value = getChild(magDp, "i");
            }
            else if(getChild(magDp, "f")){
              value = getChild(magDp, "f");
            }
            else{
              m_log->error("Invalid mag value");
              continue;  
            }  
            break;  
          }
          case IEC61850Datapoint::BSC:
          {
            value = getChild(cdcDp, "valWtr");
            break;  
          }
          default:
          {
            m_log->error("Invalid cdc type");
          }
        }
        
        Datapoint* timestamp = getChild(cdcDp, "t");
        
        Datapoint* quality = getChild(cdcDp, "q");
        
        std::shared_ptr<IEC61850Datapoint> dp = m_exchangeDefinitions.at(getValueStr(identifierDp));
        
        dp->updateDatapoint(value,timestamp,quality, false); 
        
        ModelNode* node = IedModel_getModelNodeByObjectReference(m_model, objRef.c_str());
      }
    }
  }
  return n;
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

    if (config->itemExists("exchanged_data") == false) {
        m_log->error("Missing exchange data configuration"); //LCOV_EXCL_LINE
        return;
    }

    if(config->itemExists("modelPath") == false){
        m_log->error("Missing model file path");
        return;
    }

    const std::string protocolStack = config->getValue("protocol_stack");

    const std::string dataExchange = config->getValue("exchanged_data");

    const std::string modelPath = config->getValue("modelPath");

    setModelPath(modelPath);

    std::string schedulerConfig = "";

    if(config->itemExists("scheduler_conf") == false){
      m_log->warn("Missing scheduler config");
    }
    else {
      schedulerConfig = config->getValue("scheduler_conf"); 
    }
        
    std::string tlsConfig = "";

    if (config->itemExists("tls_conf") == false) {
        m_log->error("Missing TLS configuration"); //LCOV_EXCL_LINE
    }
    else {
        tlsConfig = config->getValue("tls_conf");
    }

    setJsonConfig(protocolStack, dataExchange, tlsConfig, schedulerConfig);
}

