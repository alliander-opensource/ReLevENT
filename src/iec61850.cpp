#include "iec61850_config.hpp"
#include "iec61850_datapoint.hpp"
#include "logger.h"
#include <iec61850.hpp>

#include <libiec61850/iec61850_common.h>
#include <libiec61850/iec61850_config_file_parser.h>
#include <libiec61850/iec61850_model.h>
#include <libiec61850/iec61850_server.h>
#include <memory>
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
       Logger::getLogger()->error("datapoint " + dp->getName() + " has mot a std::string value");
    }
    
   return nullptr;
}

static const std::string
getChildValueStr(Datapoint* dp, const std::string& name)
{
    Datapoint* childDp = getChild(dp, name);

    if (childDp) {
        return getValueStr(childDp);
    }
    else {
       Logger::getLogger()->error("No such child: " + name);
    }

  return nullptr;
}

static long
getValueInt(Datapoint* dp)
{
    DatapointValue& dpv = dp->getData();

    if (dpv.getType() == DatapointValue::T_INTEGER) {
        return dpv.toInt();
    }
    else {
       Logger::getLogger()->error("datapoint " + dp->getName() + " has not an int value");
    }
  return NULL;
}

static int
getChildValueInt(Datapoint* dp, const std::string& name)
{
    Datapoint* childDp = getChild(dp, name);

    if (childDp) {
        return getValueInt(childDp);
    }
    else {
       Logger::getLogger()->error("No such child: " + name);
    }
    return (int)-1;
}

static float
getValueFloat(Datapoint* dp)
{
    DatapointValue& dpv = dp->getData();

    if (dpv.getType() == DatapointValue::T_FLOAT) {
        return (float) dpv.toDouble();
    }
    else {
       Logger::getLogger()->error("datapoint " + dp->getName() + " has not a float value");
    }
  return (float) -1;
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
  
    if(!m_model){
      m_log->fatal("Invalid Model File Path");
      return;
    }

    m_config->importExchangeConfig(dataExchangeConfig, m_model);
    m_config->importProtocolConfig(stackConfig);

    if(m_config->TLSEnabled() && tlsConfig != "")
       m_config->importTlsConfig(tlsConfig);
  
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
  auto it = m_exchangeDefinitions.find(id);
  if(it != m_exchangeDefinitions.end()) {
    return it->second->getObjRef();
  }
  return "";
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

  if(!m_server){
    m_log->fatal("NO SERVER");
    return 0;
  }

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
        
        PIVOTROOT root;
        
        Datapoint* rootDp;

        for (Datapoint* child : *sdp) {
            if (child->getName() == "GTIS") {
                root = GTIS;
                rootDp = child;
                break;
            }
            else if (child->getName() == "GTIM") {
                root = GTIM;
                rootDp = child;
                break;
            }
            else if (child->getName() == "GTIC") {
                root = GTIC;
                rootDp = child;
                break;
            }
        }
        
        m_log->warn("%s \n", rootDp->toJSONProperty().c_str());

        Datapoint* identifierDp = getChild(rootDp,"Identifier");
        
        if(!identifierDp){
          m_log->error("Identifier missing");
          continue;
        }
        
        const std::string objRef = getObjRefFromID(getValueStr(identifierDp));
        
        if(objRef == ""){
          m_log->error("objRef for label %s not found -> continue", getValueStr(identifierDp).c_str());
          continue;
        }

        Datapoint* cdcDp = getCdc(rootDp);
        
        if(!cdcDp){
          m_log->error("No cdc found or cdc type invalid");
          continue;
        }

        const std::string cdcName = cdcDp->getName();
        
        int cdcTypeInt = IEC61850Datapoint::getCdcTypeFromString(cdcName);

        if(cdcTypeInt == -1){
          m_log->error("Invalid cdc type -> %s ", cdcName.c_str());
          continue;
        }
        
        CDCTYPE cdcType = static_cast<CDCTYPE>(cdcTypeInt);
        
        if(IEC61850Datapoint::getRootFromCDC(cdcType)!= root){
          m_log->error("CDC type does not Match Root type -> %s %s", cdcName.c_str(), rootDp->getName().c_str());
          continue;
        }

        switch(cdcType){
          case SPS:
          case DPS:
          {
            Datapoint* stValDp = getChild(cdcDp, "stVal");  
            if(!stValDp){
                m_log->error("No stValDp found %s -> continue", getValueStr(identifierDp).c_str());
                continue;
            }
            value = stValDp;  
            break;  
          }
          case MV:
          {
            Datapoint* magDp = getChild(cdcDp, "mag");  
            if(!magDp){
              m_log->error("No mag datapoint found %s -> continue", getValueStr(identifierDp).c_str());
              continue;  
            } 
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
          case BSC:
          {
            Datapoint* valWtrDp = getChild(cdcDp, "valWtr");  
            if(!valWtrDp){
                m_log->error("No valWtr found %s -> continue", getValueStr(identifierDp).c_str());
                continue;
            }  
            value = valWtrDp;  
            break;  
          }
          default:
          {
            m_log->error("Invalid cdc type");
          }
        }

        if(!value){
          m_log->error("No value found -> %s", getValueStr(identifierDp).c_str());
        }
        
        Datapoint* timestamp = getChild(cdcDp, "t");
        
        Datapoint* quality = getChild(cdcDp, "q");
        

        std::shared_ptr<IEC61850Datapoint> dp;

        auto it = m_exchangeDefinitions.find(getValueStr(identifierDp));
        if(it != m_exchangeDefinitions.end()) {
          dp = it->second;
        }
        else{
          m_log->error("Datapoint with identifier : %s  not found", getValueStr(identifierDp).c_str());
          continue;
        }

        dp->updateDatapoint(value,timestamp,quality); 
        
        updateDatapointInServer(dp, false);
      }
    }
    n++;
  }
  return n;
}

void
IEC61850Server::updateDatapointInServer(std::shared_ptr<IEC61850Datapoint> dp, bool timeSynced){
  std::shared_ptr<DataAttributesDp> dadp= dp->getDadp();

  if(dadp->q){
    IedServer_updateQuality(m_server, dadp->q, dp->getQuality());
  }
  if(dadp->t){
    Timestamp tp;
    Timestamp_clearFlags(&tp);
    Timestamp_setTimeInMilliseconds(&tp, dp->getMsTimestamp());
    Timestamp_setSubsecondPrecision(&tp, 10);
    
    if (timeSynced == false) {
        Timestamp_setClockNotSynchronized(&tp, true);
    } 
    IedServer_updateTimestampAttributeValue(m_server, dadp->t, &tp);
  }
  switch(dp->getCDC()){
    case SPS:{
      IedServer_updateBooleanAttributeValue(m_server, dadp->value, dp->getIntVal());
      break;
    }
    case DPS:{
      IedServer_updateDbposValue(m_server, dadp->value, (Dbpos) dp->getIntVal());
      break;
    }
    case MV:{
      if(dp->hasIntVal()){
        IedServer_updateInt32AttributeValue(m_server, dadp->value, dp->getIntVal());
      }
      else{
        IedServer_updateFloatAttributeValue(m_server, dadp->value, dp->getFloatVal());
      }
      break;
    }
    case BSC:{
      IedServer_updateInt32AttributeValue(m_server, dadp->value, dp->getIntVal());
      break;
    }
    default:{
      m_log->error("Invalid cdc type %s", dp->getObjRef().c_str());
    }
  }
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

