#include "iec61850_config.hpp"
#include "iec61850_datapoint.hpp"
#include "logger.h"
#include <iec61850.hpp>

#include <iterator>
#include <libiec61850/iec61850_common.h>
#include <libiec61850/iec61850_config_file_parser.h>
#include <libiec61850/iec61850_model.h>
#include <libiec61850/iec61850_server.h>
#include <libiec61850/mms_value.h>
#include <memory>
#include <plugin_api.h>
#include <config_category.h>


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <vector>

bool 
isCommandCDC(CDCTYPE cdc){
  return cdc>=SPC;
}

std::string
cdcToString(CDCTYPE cdc){
  switch(cdc){
    case SPC:
      return "SpcTyp";
    case DPC:
      return "DpcTyp";
    case BSC:
      return "BscTyp";
    case APC: 
      return "ApcTyp";
    case INC:
      return "IncTyp";  
    default:
      return "";
  }
  return "";
}

static Datapoint*
createDp(const std::string& name)
{
    std::vector<Datapoint*>* datapoints = new std::vector<Datapoint*>;

    DatapointValue dpv(datapoints, true);

    Datapoint* dp = new Datapoint(name, dpv);

    return dp;
}

template <class T>
static Datapoint*
createDpWithValue(const std::string& name, const T value)
{
    DatapointValue dpv(value);

    Datapoint* dp = new Datapoint(name, dpv);

    return dp;
}

static Datapoint*
addElement(Datapoint* dp, const std::string& name)
{
    DatapointValue& dpv = dp->getData();

    std::vector<Datapoint*>* subDatapoints = dpv.getDpVec();

    Datapoint* element = createDp(name);

    if (element) {
       subDatapoints->push_back(element);
    }

    return element;
}

template <class T>
static Datapoint*
addElementWithValue(Datapoint* dp, const std::string& name, const T value)
{
    DatapointValue& dpv = dp->getData();

    std::vector<Datapoint*>* subDatapoints = dpv.getDpVec();

    Datapoint* element = createDpWithValue(name, value);

    if (element) {
       subDatapoints->push_back(element);
    }

    return element;
}


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
    m_log->setMinLevel("debug");
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
    
    m_exchangeDefinitions = *m_config->getExchangeDefinitions();

    for(auto def : m_exchangeDefinitions){
      std::shared_ptr<IEC61850Datapoint> dp = def.second;

      if(!isCommandCDC(dp->getCDC())) continue;

      m_log->info("Adding command at %s", dp->getObjRef().c_str());

      std::shared_ptr<DataAttributesDp> dadp = dp->getDadp();
      DataObject* dataObject = (DataObject*) ModelNode_getParent((ModelNode*)dadp->t);
      ServerDatapointPair* sdp = new ServerDatapointPair();
      sdp->server = this;
      sdp->dp = dp.get();

      IedServer_setControlHandler(m_server, dataObject, (ControlHandler)controlHandler, this);
      IedServer_handleWriteAccess(m_server, (DataAttribute*)dataObject, (WriteAccessHandler)writeAccessHandler, this);
      IedServer_setPerformCheckHandler(m_server, dataObject, checkHandler, sdp);
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

Datapoint*
IEC61850Server::buildPivotOperation(CDCTYPE type, MmsValue* ctlVal, bool test, bool isSelect, const std::string& label, long seconds, long fraction){
    Datapoint* rootDp = createDp("GTIC"); 
    Datapoint* comingFrom = addElementWithValue(rootDp,"ComingFrom",(std::string) "iec61850");
    Datapoint* typeDp = addElement(rootDp, cdcToString(type));
    Datapoint* identifierDp = addElementWithValue(rootDp, "Identifier", (std::string) label);
    Datapoint* selectDp = addElement(rootDp, "Select");
    Datapoint* selectStVal = addElementWithValue(selectDp, "stVal", (long) isSelect);
    Datapoint* qualityDp = addElement(typeDp, "q");
    Datapoint* testDp = addElementWithValue(qualityDp, "test", (long) test);
    Datapoint* tsDp = addElement(typeDp, "t");
    Datapoint* secondSinceEpoch = addElementWithValue(tsDp, "SecondSinceEpoch", (long) seconds);
    Datapoint* fractionOfSecond = addElementWithValue(tsDp, "FractionOfSecond", (long) fraction);
    Datapoint* ctlValDp = nullptr;
    if(isSelect){
      return rootDp;
    }
    switch(type){
      case SPC: {
        ctlValDp = addElementWithValue(typeDp, "ctlVal", (long) MmsValue_getBoolean(ctlVal));
        break;
      }
      case DPC: {
        int value = MmsValue_toInt32(ctlVal);
        std::string strVal = "";
        if(value == 0) strVal = "intermediate-state"; 
        else if (value == 1) strVal = "off";
        else if (value == 2) strVal = "on";
        else if (value == 3) strVal = "bad-state";
        ctlValDp = addElementWithValue(typeDp, "ctlVal", (std::string)strVal);
        break;
      }
      case INC: {
        ctlValDp = addElementWithValue(typeDp, "ctlVal", (long) MmsValue_toInt32(ctlVal));
        break;
      }
      case APC: {
        ctlValDp = addElementWithValue(typeDp, "ctlVal", (double) MmsValue_toFloat(ctlVal));
        break;
      }
      case BSC: {
        int value = MmsValue_toInt32(ctlVal);
        std::string strVal = "";
        if(value == 0) strVal = "stop"; 
        else if (value == 1) strVal = "lower";
        else if (value == 2) strVal = "higher";
        else if (value == 3) strVal = "reserved";
        ctlValDp = addElementWithValue(typeDp, "ctlVal", (std::string)strVal);
        break;
      }
      default: {
        Logger::getLogger()->error("Unrecognised command type -> ignore");
        return nullptr;
      }
    }
    return rootDp;
}

Datapoint*
IEC61850Server::ControlActionToPivot(ControlAction action, MmsValue* ctlVal, bool test, IEC61850Datapoint* dp){
  if(!action){
    Logger::getLogger()->warn("No control action -> ignore");
  }
  bool isSelect = ControlAction_isSelect(action);

  if(!isSelect && !ctlVal){
    Logger::getLogger()->warn("No ctlVal -> ignore");
  }

  CDCTYPE type = dp->getCDC();
  std::string label = dp->getLabel();
  
  PivotTimestamp* timestamp = new PivotTimestamp(1000);
  
  long secondSinceEpoch = timestamp->SecondSinceEpoch();
  long fractionOfSecond = timestamp->FractionOfSecond();
  
  return buildPivotOperation(type, ctlVal, test, isSelect, label, secondSinceEpoch, fractionOfSecond);
}


CheckHandlerResult
IEC61850Server::checkHandler(ControlAction action, void* parameter, MmsValue* ctlVal, bool test, bool interlockCheck)
{
    ClientConnection clientCon = ControlAction_getClientConnection(action);

    if (clientCon) {
      Logger::getLogger()->debug("Control from client %s", ClientConnection_getPeerAddress(clientCon));
    }
    else {
      Logger::getLogger()->warn("clientCon == NULL");
    }
    
    if (ControlAction_isSelect(action))
        Logger::getLogger()->debug("check handler called by select command!");
    else
        Logger::getLogger()->debug("check handler called by operate command!");

    if (interlockCheck)
        Logger::getLogger()->debug("  with interlock check bit set!");

    Logger::getLogger()->debug("  ctlNum: %i", ControlAction_getCtlNum(action));
    
    ServerDatapointPair* sdp = (ServerDatapointPair*) parameter;

    sdp->server->forwardCommand(action,ctlVal,test,sdp->dp);
    
    return CONTROL_ACCEPTED;
}

bool
IEC61850Server::forwardCommand(ControlAction action, MmsValue* ctlVal, bool test, IEC61850Datapoint* dp){
   
    Datapoint* pivotControlDp = ControlActionToPivot(action,ctlVal,test,dp);
    
    if(!pivotControlDp){
      Logger::getLogger()->error("Couldn't convert command to pivot");
      return false;
    }

    char* names[1];
    char* parameters[1];

    std::string jsonDp = "{" + pivotControlDp->toJSONProperty() + "}";
    char* jsonDpCString = (char*) jsonDp.c_str(); 

    names[0] = (char*) "PIVOTTC";
    parameters[0] = (char*) (jsonDpCString);

    m_oper((char*)"PivotCommand", 1 ,names, parameters, DestinationBroadcast, NULL);
    
    return true;
}

MmsDataAccessError
IEC61850Server::writeAccessHandler (DataAttribute* dataAttribute, MmsValue* value, ClientConnection connection, void* parameter)
{
    ControlModel ctlModelVal = (ControlModel) MmsValue_toInt32(value);

    if ((ctlModelVal == CONTROL_MODEL_STATUS_ONLY) || (ctlModelVal == CONTROL_MODEL_DIRECT_NORMAL))
    {
        IedServer_updateCtlModel((IedServer) parameter, (DataObject*) dataAttribute, ctlModelVal);

        return DATA_ACCESS_ERROR_SUCCESS;
    }
    else {
        IedServer_updateCtlModel((IedServer) parameter, (DataObject*) dataAttribute, ctlModelVal);

        return DATA_ACCESS_ERROR_SUCCESS;
    }
}

ControlHandlerResult
IEC61850Server::controlHandler(ControlAction action, void* parameter, MmsValue* value, bool test){
  IEC61850Server* self = (IEC61850Server*)parameter;

  Logger::getLogger()->info("control handler called");
  Logger::getLogger()->info("  ctlNum: %i", ControlAction_getCtlNum(action));

  ClientConnection clientCon = ControlAction_getClientConnection(action);

  if (clientCon) {
      Logger::getLogger()->debug("Control from client %s", ClientConnection_getPeerAddress(clientCon));
  }
  else {
      Logger::getLogger()->warn("clientCon == NULL!");
  }
  return CONTROL_RESULT_OK;
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
        
        m_log->warn("%s ", rootDp->toJSONProperty().c_str());

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

