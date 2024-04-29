#include "iec61850_config.hpp"
#include "iec61850_datapoint.hpp"
#include "logger.h"
#include <cstdint>
#include <iec61850.hpp>

#include <config_category.h>
#include <iterator>
#include <libiec61850/iec61850_common.h>
#include <libiec61850/iec61850_config_file_parser.h>
#include <libiec61850/iec61850_model.h>
#include <libiec61850/iec61850_server.h>
#include <libiec61850/mms_value.h>
#include <map>
#include <memory>
#include <plugin_api.h>

#include <libiec61850/hal_thread.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
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

#include <stdbool.h>
#include <string>
#include <thread>
#include <vector>

void IEC61850Server::bgThreadFunc(IEC61850Server *self)
{
  std::map<std::string, std::shared_ptr<SchedulerTarget>>* targets = NULL; 

  while (self->bgThreadRunning)
  {
    uint64_t currentTime = Hal_getTimeInMs();

    targets = self->m_config->getSchedulerTargets();

    if (targets)
    {
      for (const auto& target : *targets)
      {
        std::shared_ptr<SchedulerTarget> schedTarget = target.second;

        if (schedTarget->targetDp == NULL)
        {
          const char* targetEntityRef = Scheduler_getCtlEntityRef(self->m_scheduler, schedTarget->schedControllerRef.c_str());

          if (targetEntityRef)
          {
            char objRef[130];

            if (targetEntityRef[0] == '@') {
              strncpy(objRef, self->m_model->name, 129);
              strncat(objRef, targetEntityRef + 1, 129);
              objRef[129] = 0;
            }
            else {
              strncpy(objRef, targetEntityRef, 129);
              objRef[129] = 0;
            }

            schedTarget->targetDp = self->m_config->getDatapointByObjectReference(std::string(objRef));

            if (schedTarget->targetDp) {
              Iec61850Utility::log_info("bgThreadFunc: assigned DP %s to target %s", objRef, schedTarget->schedControllerRef.c_str());
            }
            else {
              Iec61850Utility::log_warn("bgThreadFunc: DP %s not found", objRef);
            }
          }
          else {
            Iec61850Utility::log_warn("bgThreadFunc: ctlEntityRef no found for %s", schedTarget->schedControllerRef.c_str());
          }
        }

        if (schedTarget->forwardSchedule && schedTarget->targetDp)
        {
          if (currentTime >= (schedTarget->lastSchedulerForwarding + (schedTarget->forwardScheduleInterval * 1000)))
          {
            LinkedList forecast = Scheduler_createForecast(self->m_scheduler, 
                schedTarget->schedControllerRef.c_str(), currentTime, currentTime + (schedTarget->forwardSchedulePeriod * 1000));

            if (forecast)
            {
              if (self->forwardScheduleForecast(schedTarget->targetDp.get(), forecast)) {
                Iec61850Utility::log_info("Sent schedule forecast");
              }
              else {
                Iec61850Utility::log_error("Failed to send schedule forecast");
              }

              LinkedList_destroyDeep(forecast, (LinkedListValueDeleteFunction)ScheduleEvent_destroy);
            }
            else {
              Iec61850Utility::log_error("Failed to create schedule forecast");
            }

            schedTarget->lastSchedulerForwarding = currentTime;
          }
        }
      }
    }

    Thread_sleep(100);
  }
}

IEC61850Server::IEC61850Server()
    : m_started(false),
      m_log(Logger::getLogger()) {
  Logger::getLogger()->setMinLevel("debug");

  bgThreadRunning = true;

  m_config = new IEC61850Config();

  backgroundThread = new std::thread(bgThreadFunc, this);
}

IEC61850Server::~IEC61850Server() {

  bgThreadRunning = false;

  backgroundThread->join();

  stop();

  delete m_config;
}

bool isCommandCDC(CDCTYPE cdc) { return cdc >= SPC; }

std::map<CDCTYPE, std::string> cdcStringMap = {
    {SPS, "SpsTyp"}, {DPS, "DpsTyp"}, {BSC, "BscTyp"}, {MV, "MvTyp"},
    {SPC, "SpcTyp"}, {DPC, "DpcTyp"}, {APC, "ApcTyp"}, {INC, "IncTyp"}};

std::string cdcToString(CDCTYPE cdc) {
  auto it = cdcStringMap.find(cdc);
  if (it != cdcStringMap.end()) {
    return it->second;
  }
  return "";
}

static Datapoint *createDp(const std::string &name) {
  std::vector<Datapoint *> *datapoints = new std::vector<Datapoint *>;

  DatapointValue dpv(datapoints, true);

  Datapoint *dp = new Datapoint(name, dpv);

  return dp;
}

template <class T>
static Datapoint *createDpWithValue(const std::string &name, const T value) {
  DatapointValue dpv(value);

  Datapoint *dp = new Datapoint(name, dpv);

  return dp;
}

static Datapoint *addElement(Datapoint *dp, const std::string &name) {
  DatapointValue &dpv = dp->getData();

  std::vector<Datapoint *> *subDatapoints = dpv.getDpVec();

  Datapoint *element = createDp(name);

  if (element) {
    subDatapoints->push_back(element);
  }

  return element;
}

template <class T>
static Datapoint *addElementWithValue(Datapoint *dp, const std::string &name,
                                      const T value) {
  DatapointValue &dpv = dp->getData();

  std::vector<Datapoint *> *subDatapoints = dpv.getDpVec();

  Datapoint *element = createDpWithValue(name, value);

  if (element) {
    subDatapoints->push_back(element);
  }

  return element;
}

static Datapoint *getChild(Datapoint *dp, const std::string &name) {
  Datapoint *childDp = nullptr;

  DatapointValue &dpv = dp->getData();

  if (dpv.getType() != DatapointValue::T_DP_DICT) {
    Iec61850Utility::log_warn("Datapoint not a dictionary");
    return nullptr;
  }

  std::vector<Datapoint *> *datapoints = dpv.getDpVec();

  if (!datapoints) {
    Iec61850Utility::log_warn("datapoints is nullptr");
    return nullptr;
  }

  for (auto child : *datapoints) {
    if (child->getName() == name) {
      childDp = child;
      break;
    }
  }

  return childDp;
}

static const std::string getValueStr(Datapoint *dp) {
  DatapointValue &dpv = dp->getData();

  if (dpv.getType() == DatapointValue::T_STRING) {
    return dpv.toStringValue();
  } else {
    Iec61850Utility::log_error("datapoint " + dp->getName() +
                               " has mot a std::string value");
  }

  return nullptr;
}

static Datapoint *getCdc(Datapoint *dp) {
  if (!dp)
    return nullptr;

  DatapointValue &dpv = dp->getData();

  if (dpv.getType() != DatapointValue::T_DP_DICT)
    return nullptr;

  std::vector<Datapoint *> *datapoints = dpv.getDpVec();

  for (Datapoint *child : *datapoints) {
    if (IEC61850Datapoint::getCdcTypeFromString(child->getName()) != -1) {
      return child;
    }
  }

  return nullptr;
}

void IEC61850Server::scheduler_TargetValueChanged(void *parameter,
                                                  const char *targetValueObjRef,
                                                  MmsValue *value,
                                                  Quality quality,
                                                  uint64_t timestampMs)
{
  char mmsValueBuf[200];
  mmsValueBuf[0] = 0;

  if (value) {
    MmsValue_printToBuffer(value, mmsValueBuf, 200);
  }

  Iec61850Utility::log_info("New schedule target value for %s: %s", targetValueObjRef, mmsValueBuf);

  IEC61850Server *server = (IEC61850Server *)parameter;

  char translatedObjRefBuf[200] = {0};

  ModelNode *node = IedModel_getModelNodeByShortObjectReference(
      server->m_model, targetValueObjRef);
  if (!node) {
    Iec61850Utility::log_error(
        "Model node with reference %s not found in model", targetValueObjRef);
    return;
  }

  ModelNode_getObjectReference(node, translatedObjRefBuf);

  std::string translatedObjRef = std::string(translatedObjRefBuf);

  std::shared_ptr<IEC61850Datapoint> dp =
      server->m_config->getDatapointByObjectReference(translatedObjRef);

  if (!dp) {
    Iec61850Utility::log_warn(
        "%s not found in exchanged data, operation won't be sent to south",
        translatedObjRef.c_str());
  }

  if (value) {
    MmsValue_printToBuffer(value, mmsValueBuf, 200);

    Iec61850Utility::log_debug("Received target value change: %s: %s\n",
                               targetValueObjRef, mmsValueBuf);

    if (dp) {
      server->forwardScheduleCommand(value,
                                     Quality_isFlagSet(&quality, QUALITY_TEST),
                                     dp.get(), timestampMs);
    }
  }
}

void IEC61850Server::setJsonConfig(const std::string &stackConfig,
                                   const std::string &dataExchangeConfig,
                                   const std::string &tlsConfig,
                                   const std::string &schedulerConfig) {
  Logger::getLogger()->setMinLevel("debug");
  m_model = ConfigFileParser_createModelFromConfigFileEx(m_modelPath.c_str());

  if (!m_model) {
    Iec61850Utility::log_fatal("Invalid Model File Path %s",
                               m_modelPath.c_str());
    return;
  }

  m_config->importExchangeConfig(dataExchangeConfig, m_model);
  m_config->importProtocolConfig(stackConfig);

  if (m_config->TLSEnabled() && !tlsConfig.empty())
    m_config->importTlsConfig(tlsConfig);

  m_server = IedServer_create(m_model);

  if (!m_server) {
    Iec61850Utility::log_error("Server couldn't be created");
    return;
  }

  m_exchangeDefinitions = m_config->getExchangeDefinitions();

  if (m_config->schedulerEnabled() && !schedulerConfig.empty())
  {
    m_scheduler = Scheduler_create(m_model, m_server);
    m_config->importSchedulerConfig(schedulerConfig, m_scheduler);
    Scheduler_setTargetValueHandler(m_scheduler, scheduler_TargetValueChanged,
                                    this);
    Iec61850Utility::log_warn("Scheduler created");
  }

  sdpObjects =
      new std::vector<std::pair<IEC61850Server *, IEC61850Datapoint *> *>();
  for (auto def : *m_exchangeDefinitions) {
    std::shared_ptr<IEC61850Datapoint> dp = def.second;

    if (!isCommandCDC(dp->getCDC()))
      continue;

    Iec61850Utility::log_info("Adding command at %s", dp->getObjRef().c_str());

    std::shared_ptr<DataAttributesDp> dadp = dp->getDadp();
    auto dataObject = (DataObject *)dadp->value;
    auto sdp =
        new std::pair<IEC61850Server *, IEC61850Datapoint *>(this, dp.get());
    sdpObjects->push_back(sdp);

    IedServer_setControlHandler(m_server, dataObject,
                                (ControlHandler)controlHandler, this);
    IedServer_handleWriteAccess(m_server, (DataAttribute *)dataObject,
                                (WriteAccessHandler)writeAccessHandler, this);
    IedServer_setPerformCheckHandler(m_server, dataObject, checkHandler, sdp);
  }

  IedServer_start(m_server, m_config->TcpPort());
  Iec61850Utility::log_debug("PORT %d \n", m_config->TcpPort());

  if (IedServer_isRunning(m_server)) {
    Iec61850Utility::log_info("Server is running on port " +
                              std::to_string(m_config->TcpPort()));
  } else {
    Iec61850Utility::log_debug("Server could not start \n");
  }
}

Datapoint *IEC61850Server::buildPivotOperation(CDCTYPE type, MmsValue *ctlVal,
                                               bool test, bool isSelect,
                                               const std::string &label,
                                               PivotTimestamp *timestamp,
                                               bool hasSelect) {
  long seconds = timestamp->SecondSinceEpoch();
  long fraction = timestamp->FractionOfSecond();

  Datapoint *rootDp = createDp("GTIC");
  Datapoint *comingFrom =
      addElementWithValue(rootDp, "ComingFrom", (std::string) "iec61850");
  Datapoint *typeDp = addElement(rootDp, cdcToString(type));
  Datapoint *identifierDp =
      addElementWithValue(rootDp, "Identifier", (std::string)label);

  if (hasSelect) {
    Datapoint *selectDp = addElement(rootDp, "Select");
    Datapoint *selectStVal =
        addElementWithValue(selectDp, "stVal", (long)isSelect);
  }

  Datapoint *qualityDp = addElement(typeDp, "q");
  Datapoint *testDp = addElementWithValue(qualityDp, "test", (long)test);

  Datapoint *tsDp = addElement(typeDp, "t");
  Datapoint *secondSinceEpoch =
      addElementWithValue(tsDp, "SecondSinceEpoch", (long)seconds);
  Datapoint *fractionOfSecond =
      addElementWithValue(tsDp, "FractionOfSecond", (long)fraction);

  Datapoint *ctlValDp = nullptr;

  if (isSelect) {
    return rootDp;
  }
  switch (type) {
  case SPC: {
    ctlValDp = addElementWithValue(typeDp, "ctlVal",
                                   (long)MmsValue_getBoolean(ctlVal));
    break;
  }
  case DPC: {
    int value = MmsValue_toInt32(ctlVal);
    std::string strVal = "";
    if (value == 0)
      strVal = "intermediate-state";
    else if (value == 1)
      strVal = "off";
    else if (value == 2)
      strVal = "on";
    else if (value == 3)
      strVal = "bad-state";
    ctlValDp = addElementWithValue(typeDp, "ctlVal", (std::string)strVal);
    break;
  }
  case INC: {
    ctlValDp =
        addElementWithValue(typeDp, "ctlVal", (long)MmsValue_toInt32(ctlVal));
    break;
  }
  case APC: {
    ctlValDp =
        addElementWithValue(typeDp, "ctlVal", (double)MmsValue_toFloat(ctlVal));
    break;
  }
  case BSC: {
    int value = MmsValue_toInt32(ctlVal);
    std::string strVal = "";
    if (value == 0)
      strVal = "stop";
    else if (value == 1)
      strVal = "lower";
    else if (value == 2)
      strVal = "higher";
    else if (value == 3)
      strVal = "reserved";
    ctlValDp = addElementWithValue(typeDp, "ctlVal", (std::string)strVal);
    break;
  }
  default: {
    Iec61850Utility::log_error("Unrecognised command type -> ignore");
    return nullptr;
  }
  }
  return rootDp;
}

Datapoint *IEC61850Server::ControlActionToPivot(ControlAction action,
                                                MmsValue *ctlVal, bool test,
                                                IEC61850Datapoint *dp) {
  if (!action) {
    Iec61850Utility::log_warn("No control action -> ignore");
  }

  bool isSelect = ControlAction_isSelect(action);

  if (!isSelect && !ctlVal) {
    Iec61850Utility::log_warn("No ctlVal -> ignore");
  }

  CDCTYPE type = dp->getCDC();
  std::string label = dp->getLabel();
  PivotTimestamp *timestamp = dp->getTimestamp();
  timestamp->setTimeInMs(PivotTimestamp::GetCurrentTimeInMs());

  return buildPivotOperation(type, ctlVal, test, isSelect, label, timestamp,
                             true);
}

CheckHandlerResult IEC61850Server::checkHandler(ControlAction action,
                                                void *parameter,
                                                MmsValue *ctlVal, bool test,
                                                bool interlockCheck) {
  ClientConnection clientCon = ControlAction_getClientConnection(action);

  if (clientCon) {
    Iec61850Utility::log_debug("Control from client %s",
                               ClientConnection_getPeerAddress(clientCon));
  } else {
    Iec61850Utility::log_warn("clientCon == NULL");
  }

  if (ControlAction_isSelect(action))
    Iec61850Utility::log_debug("check handler called by select command!");
  else
    Iec61850Utility::log_debug("check handler called by operate command!");

  if (interlockCheck)
    Iec61850Utility::log_debug("  with interlock check bit set!");

  Iec61850Utility::log_debug("  ctlNum: %i", ControlAction_getCtlNum(action));

  auto sdp = (std::pair<IEC61850Server *, IEC61850Datapoint *> *)parameter;

  sdp->first->forwardCommand(action, ctlVal, test, sdp->second);

  return CONTROL_ACCEPTED;
}

bool IEC61850Server::forwardCommand(ControlAction action, MmsValue *ctlVal,
                                    bool test, IEC61850Datapoint *dp) {
  Datapoint *pivotControlDp = ControlActionToPivot(action, ctlVal, test, dp);

  if (!pivotControlDp) {
    Iec61850Utility::log_error("Couldn't convert command to pivot");
    return false;
  }

  char *names[1];
  char *parameters[1];

  std::string jsonDp = "{" + pivotControlDp->toJSONProperty() + "}";
  char *jsonDpCString = (char *)jsonDp.c_str();

  names[0] = (char *)"PIVOTTC";
  parameters[0] = (char *)(jsonDpCString);

  Iec61850Utility::log_info("Send operation -> %s", jsonDp.c_str());
  m_oper((char *)"PivotCommand", 1, names, parameters, DestinationBroadcast,
         NULL);

  delete pivotControlDp;
  return true;
}

bool IEC61850Server::forwardScheduleCommand(MmsValue *ctlVal, bool test,
                                            IEC61850Datapoint *dp,
                                            uint64_t timestampMs)
{
  if (m_oper) {
    CDCTYPE type = dp->getCDC();
    std::string label = dp->getLabel();
    PivotTimestamp *timestamp = dp->getTimestamp();
    timestamp->setTimeInMs(timestampMs);

    Datapoint *pivotControlDp =
        buildPivotOperation(type, ctlVal, test, false, label, timestamp, false);

    if (!pivotControlDp) {
      Iec61850Utility::log_error("Couldn't convert command to pivot");
      return false;
    }

    char *names[1];
    char *parameters[1];

    std::string jsonDp = "{" + pivotControlDp->toJSONProperty() + "}";
    char *jsonDpCString = (char *)jsonDp.c_str();

    names[0] = (char *)"PIVOTTC";
    parameters[0] = (char *)(jsonDpCString);

    Iec61850Utility::log_info("Send operation -> %s", jsonDp.c_str());


    m_oper((char *)"PivotCommand", 1, names, parameters, DestinationBroadcast,
          NULL);

    delete pivotControlDp;

      return true;
  }
  else {
    Iec61850Utility::log_error("Failed to send schedule command -> operation handler missing");
    return false;
  }
}

bool IEC61850Server::forwardScheduleForecast(IEC61850Datapoint *dp,
                                             LinkedList schedule)
{
  // send a PivotCommand with multiple parameters (one parameter for each
  // schedule entry)
  int numberOfSchedules = LinkedList_size(schedule);

  LinkedList scheduleElem = LinkedList_getNext(schedule);

  char *names[numberOfSchedules];
  char *parameters[numberOfSchedules];
  std::string parameterStrings[numberOfSchedules];

  int idx = 0;

  while (scheduleElem) {
    ScheduleEvent event = (ScheduleEvent)LinkedList_getData(scheduleElem);

    char valBuf[200];
    valBuf[0] = 0;

    if (ScheduleEvent_getValue(event)) {
      MmsValue_printToBuffer(ScheduleEvent_getValue(event), valBuf, 200);
    }

    names[idx] = (char *)"PIVOTTC";

    PivotTimestamp pivotTime(ScheduleEvent_getTime(event));

    Datapoint *pivotControlDp =
        buildPivotOperation(dp->getCDC(), ScheduleEvent_getValue(event), false,
                            false, dp->getLabel(), &pivotTime, false);

    parameterStrings[idx] = "{" + pivotControlDp->toJSONProperty() + "}";
    parameters[idx] = (char *)(parameterStrings[idx].c_str());

    idx++;

    scheduleElem = LinkedList_getNext(scheduleElem);
  }

  if (m_oper) {
    m_oper((char *)"PivotSchedule", idx, names, parameters, DestinationBroadcast,
          NULL);
  }
  else {
    Iec61850Utility::log_error("Failed to send schedule -> no operation handler");

    return false;
  }

  return true;
}

MmsDataAccessError
IEC61850Server::writeAccessHandler(DataAttribute *dataAttribute,
                                   MmsValue *value, ClientConnection connection,
                                   void *parameter) {
  ControlModel ctlModelVal = (ControlModel)MmsValue_toInt32(value);

  if ((ctlModelVal == CONTROL_MODEL_STATUS_ONLY) ||
      (ctlModelVal == CONTROL_MODEL_DIRECT_NORMAL)) {
    IedServer_updateCtlModel((IedServer)parameter, (DataObject *)dataAttribute,
                             ctlModelVal);

    return DATA_ACCESS_ERROR_SUCCESS;
  } else {
    IedServer_updateCtlModel((IedServer)parameter, (DataObject *)dataAttribute,
                             ctlModelVal);

    return DATA_ACCESS_ERROR_SUCCESS;
  }
}

ControlHandlerResult IEC61850Server::controlHandler(ControlAction action,
                                                    void *parameter,
                                                    MmsValue *value,
                                                    bool test) {
  IEC61850Server *self = (IEC61850Server *)parameter;

  Iec61850Utility::log_info("control handler called");
  Iec61850Utility::log_info("  ctlNum: %i", ControlAction_getCtlNum(action));

  ClientConnection clientCon = ControlAction_getClientConnection(action);

  if (clientCon) {
    Iec61850Utility::log_debug("Control from client %s",
                               ClientConnection_getPeerAddress(clientCon));
  } else {
    Iec61850Utility::log_warn("clientCon == NULL!");
  }
  return CONTROL_RESULT_OK;
}

void IEC61850Server::stop()
{
  m_started = false;

  if (m_scheduler != nullptr) {
    Scheduler_destroy(m_scheduler);
  }

  if (m_server != nullptr) {
    IedServer_stop(m_server);
    IedServer_destroy(m_server);
  }

  if (m_model != nullptr) {
    IedModel_destroy(m_model);
  }

  if (sdpObjects) {
    for (auto p : *sdpObjects) {
      delete p;
    }
    sdpObjects->clear();
    delete sdpObjects;
    sdpObjects = nullptr;
  }
}

const std::string IEC61850Server::getObjRefFromID(const std::string &id) {
  auto it = m_exchangeDefinitions->find(id);
  if (it != m_exchangeDefinitions->end()) {
    return it->second->getObjRef();
  }
  return "";
}

Datapoint *getCDCRootDp(Datapoint *dp) {
  Datapoint *cdcDp = nullptr;
  DatapointValue dpv = dp->getData();
  std::vector<Datapoint *> *sdp = dpv.getDpVec();

  for (Datapoint *child : *sdp) {
    if (IEC61850Datapoint::getCDCRootFromString(child->getName()) != -1) {
      cdcDp = new Datapoint(child->getName(), child->getData());
      break;
    }
  }
  return cdcDp;
}

Datapoint *getCDCValue(Datapoint *cdcDp) {
  const std::string cdcName = cdcDp->getName();

  int cdcTypeInt = IEC61850Datapoint::getCdcTypeFromString(cdcName);

  if (cdcTypeInt == -1) {
    Iec61850Utility::log_error("Invalid cdc type -> %s ", cdcName.c_str());
    return nullptr;
  }
  CDCTYPE cdcType = static_cast<CDCTYPE>(cdcTypeInt);

  switch (cdcType) {
  case SPS:
  case DPS: {
    Datapoint *stValDp = getChild(cdcDp, "stVal");
    if (!stValDp) {
      Iec61850Utility::log_error("No stValDp found %s -> continue",
                                 cdcDp->toJSONProperty().c_str());
      return nullptr;
    }
    return stValDp;
    break;
  }
  case MV: {
    Datapoint *magDp = getChild(cdcDp, "mag");
    if (!magDp) {
      Iec61850Utility::log_error("No mag datapoint found %s -> continue",
                                 cdcDp->toJSONProperty().c_str());
      return nullptr;
    }
    if (getChild(magDp, "i")) {
      return getChild(magDp, "i");
    } else if (getChild(magDp, "f")) {
      return getChild(magDp, "f");
    } else {
      Iec61850Utility::log_error("Invalid mag value");
      return nullptr;
    }
    break;
  }
  case BSC: {
    Datapoint *valWtrDp = getChild(cdcDp, "valWtr");
    if (!valWtrDp) {
      Iec61850Utility::log_error("No valWtr found %s -> continue",
                                 cdcDp->toJSONProperty().c_str());
      return nullptr;
    }
    return valWtrDp;
    break;
  }
  default: {
    Iec61850Utility::log_error("Invalid cdc type");
  }
  }
  return nullptr;
}

void IEC61850Server::registerControl(
    int (*operation)(char *operation, int paramCount, char *names[],
                     char *parameters[], ControlDestination destination, ...)) {
  m_oper = operation;

  Iec61850Utility::log_warn("RegisterControl is called"); // LCOV_EXCL_LINE
}

uint32_t IEC61850Server::send(const std::vector<Reading *> &readings) {
  int n = 0;
  int i = 0;
  if (!m_server) {
    Iec61850Utility::log_fatal("NO SERVER");
    return 0;
  }

  for (const auto &reading : readings) {
    if (!reading) {
      Iec61850Utility::log_warn("Reading is null");
      continue;
    }

    std::vector<Datapoint *> &dataPoints = reading->getReadingData();
    std::string assetName = reading->getAssetName();

    if (dataPoints.empty()) {
      Iec61850Utility::log_warn("Reading has no data");
      continue;
    }

    for (Datapoint *dp : dataPoints) {

      if (!dp) {
        Iec61850Utility::log_warn("Datapoint is null");
        continue;
      }

      Iec61850Utility::log_debug("  %s", dp->toJSONProperty().c_str());

      if (dp->getName() == "PIVOT") {

        if (!IedServer_isRunning(m_server)) {
          Iec61850Utility::log_warn("Server not running, can't send reading");
          return n;
        }

        Datapoint *rootDp = getCDCRootDp(dp);
        if (!rootDp) {
          Iec61850Utility::log_error("No CDC root or root invalid %s",
                                     dp->toJSONProperty().c_str());
          continue;
        }

        PIVOTROOT root = (PIVOTROOT)IEC61850Datapoint::getCDCRootFromString(
            rootDp->getName());

        Datapoint *identifierDp = getChild(rootDp, "Identifier");

        if (!identifierDp) {
          Iec61850Utility::log_error("Identifier missing");
          continue;
        }

        const std::string objRef = getObjRefFromID(getValueStr(identifierDp));

        if (objRef.empty()) {
          Iec61850Utility::log_error(
              "objRef for label %s not found -> continue",
              getValueStr(identifierDp).c_str());
          continue;
        }

        Datapoint *cdcDp = getCdc(rootDp);

        if (!cdcDp) {
          Iec61850Utility::log_error("No cdc found or cdc type invalid");
          continue;
        }

        Datapoint *causeDp = getChild(rootDp, "Cause");
        if (causeDp) {
          Datapoint *stVal = getChild(causeDp, "stVal");
          if (stVal) {
            int cause = stVal->getData().toInt();
            if (cause == 7 || cause == 10) {
              Iec61850Utility::log_debug(
                  "Command acknowledge datapoint (%s) -> ignore",
                  objRef.c_str());
              continue;
            }
          }
        }

        Datapoint *value = getCDCValue(cdcDp);

        if (!value) {
          Iec61850Utility::log_error("No value found -> %s",
                                     getValueStr(identifierDp).c_str());
          continue;
        }

        Datapoint *timestamp = getChild(cdcDp, "t");
        Datapoint *quality = getChild(cdcDp, "q");

        std::shared_ptr<IEC61850Datapoint> newDp;

        if (!m_exchangeDefinitions) {
          Iec61850Utility::log_error("m_exchangeDefinitions is null");
          continue;
        }

        auto it = m_exchangeDefinitions->find(getValueStr(identifierDp));

        if (it != m_exchangeDefinitions->end()) {
          newDp = it->second;
        } else {
          Iec61850Utility::log_error("Datapoint with identifier: %s not found",
                                     getValueStr(identifierDp).c_str());
          continue;
        }

        newDp->updateDatapoint(value, timestamp, quality);

        updateDatapointInServer(newDp, false);
      }
    }
    n++;
  }
  return n;
}

void IEC61850Server::updateDatapointInServer(
    std::shared_ptr<IEC61850Datapoint> dp, bool timeSynced) {
  std::shared_ptr<DataAttributesDp> dadp = dp->getDadp();

  if (dadp->q) {
    IedServer_updateQuality(m_server, dadp->q, dp->getQuality());
  }
  if (dadp->t) {
    Timestamp tp;
    Timestamp_clearFlags(&tp);
    Timestamp_setTimeInMilliseconds(&tp, dp->getTimestamp()->getTimeInMs());
    Timestamp_setSubsecondPrecision(&tp, 10);

    if (timeSynced == false) {
      Timestamp_setClockNotSynchronized(&tp, true);
    }
    IedServer_updateTimestampAttributeValue(m_server, dadp->t, &tp);
  }
  switch (dp->getCDC()) {
  case SPS: {

    IedServer_updateBooleanAttributeValue(m_server, dadp->mmsVal,
                                          dp->getIntVal());
    break;
  }
  case DPS: {
    IedServer_updateDbposValue(m_server, dadp->mmsVal, (Dbpos)dp->getIntVal());
    break;
  }
  case MV: {
    if (dp->hasIntVal()) {
      IedServer_updateInt32AttributeValue(m_server, dadp->mmsVal,
                                          dp->getIntVal());
    } else {
      IedServer_updateFloatAttributeValue(m_server, dadp->mmsVal,
                                          dp->getFloatVal());
    }
    break;
  }
  case BSC: {
    IedServer_updateInt32AttributeValue(m_server, dadp->mmsVal,
                                        dp->getIntVal());
    break;
  }
  default: {
    Iec61850Utility::log_error("Invalid cdc type %s", dp->getObjRef().c_str());
  }
  }
}

void IEC61850Server::configure(const ConfigCategory *config) {
  Iec61850Utility::log_info("configure called"); // LCOV_EXCL_LINE

  if (config->itemExists("name"))
    m_name = config->getValue("name"); // LCOV_EXCL_LINE
  else
    Iec61850Utility::log_error(
        "Missing name in configuration"); // LCOV_EXCL_LINE

  if (config->itemExists("protocol_stack") == false) {
    Iec61850Utility::log_error(
        "Missing protocol configuration"); // LCOV_EXCL_LINE
    return;
  }

  if (config->itemExists("exchanged_data") == false) {
    Iec61850Utility::log_error(
        "Missing exchange data configuration"); // LCOV_EXCL_LINE
    return;
  }

  if (config->itemExists("modelPath") == false) {
    Iec61850Utility::log_error("Missing model file path");
    return;
  }

  std::string protocolStack = config->getValue("protocol_stack");

  std::string dataExchange = config->getValue("exchanged_data");

  std::string modelPath = config->getValue("modelPath");

  if (protocolStack.empty()) {
    protocolStack = config->getDefault("protocol_stack");
  }
  if (dataExchange.empty()) {
    dataExchange = config->getDefault("exchanged_data");
  }
  if (modelPath.empty()) {
    modelPath = config->getDefault("modelPath");
  }

  setModelPath(modelPath);

  std::string schedulerConfig = "";

  if (config->itemExists("scheduler_conf") == false) {
    Iec61850Utility::log_warn("Missing scheduler config");
  } else {
    schedulerConfig = config->getValue("scheduler_conf");
  }

  if (schedulerConfig.empty()) {
    schedulerConfig = config->getDefault("scheduler_conf");
  }

  std::string tlsConfig = "";

  if (config->itemExists("tls_conf") == false) {
    Iec61850Utility::log_error("Missing TLS configuration"); // LCOV_EXCL_LINE
  } else {
    tlsConfig = config->getValue("tls_conf");
  }

  setJsonConfig(protocolStack, dataExchange, tlsConfig, schedulerConfig);
}
