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

#include "iec61850_datapoint.hpp"
#include "datapoint.h"

#include <cstdint>
#include <ctime>
#include <libiec61850/iec61850_common.h>
#include <libiec61850/iec61850_model.h>
#include <string>
#include <sys/time.h>

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

static long
getValueInt(Datapoint* dp)
{
    DatapointValue& dpv = dp->getData();

    if (dpv.getType() == DatapointValue::T_INTEGER) {
        return dpv.toInt();
    }
    else {
        Iec61850Utility::log_error("Value is not int %s", dp->toJSONProperty().c_str());
    }
  return -1;
}

std::map<std::string,CDCTYPE> cdcMap = {
    {"SpsTyp",SPS}, {"DpsTyp",DPS},
    {"BscTyp",BSC}, {"MvTyp",MV},
    {"SpcTyp",SPC}, {"DpcTyp",DPC},
    {"ApcTyp",APC}, {"IncTyp",INC}};

std::map<CDCTYPE, PIVOTROOT> rootMap = {
    {SPS, GTIS}, {DPS, GTIS},
    {BSC, GTIS}, {MV,  GTIM},
    {SPC, GTIC}, {DPS,GTIC},
    {APC,GTIC}, {INC, GTIC}
};

std::map<std::string, PIVOTROOT> rootStrMap = {
  {"GTIC", GTIC}, {"GTIS", GTIS}, {"GTIM", GTIM}
};


IEC61850Datapoint::IEC61850Datapoint(const std::string& label,
                                     const std::string& objRef, CDCTYPE cdc, std::shared_ptr<DataAttributesDp> dadp) {
  m_label = label;
  m_objref = objRef;
  m_cdc = cdc;
  m_dadp = dadp;

  m_timestamp = new PivotTimestamp(PivotTimestamp::GetCurrentTimeInMs());
}

IEC61850Datapoint::~IEC61850Datapoint(){
  if(m_timestamp){
    delete m_timestamp;
  }
}

int
IEC61850Datapoint::getCdcTypeFromString( const std::string& cdc) {
    auto it = cdcMap.find(cdc);
  if (it != cdcMap.end()) {
    return it->second;
  }
  return -1;
}

int 
IEC61850Datapoint::getRootFromCDC( const CDCTYPE cdc){
  auto it = rootMap.find(cdc);
  if(it != rootMap.end()) {
    return it->second;
  }
  return -1;
}

uint64_t
PivotTimestamp::GetCurrentTimeInMs()
{
    struct timeval now;

    gettimeofday(&now, nullptr);

    return ((uint64_t) now.tv_sec * 1000LL) + (now.tv_usec / 1000);
}

void
PivotTimestamp::handleTimeQuality(Datapoint* timeQuality)
{
    DatapointValue& dpv = timeQuality->getData();

    if (dpv.getType() == DatapointValue::T_DP_DICT)
    {
        std::vector<Datapoint*>* datapoints = dpv.getDpVec();

        for (Datapoint* child : *datapoints)
        {
            if (child->getName() == "clockFailure") {
                if (getValueInt(child) > 0)
                    m_clockFailure = true;
                else
                    m_clockFailure = false;
            }
            else if (child->getName() == "clockNotSynchronized") {
                if (getValueInt(child) > 0)
                    m_clockNotSynchronized = true;
                else
                    m_clockNotSynchronized = false;
            }
            else if (child->getName() == "leapSecondKnown") {
                if (getValueInt(child) > 0)
                    m_leapSecondKnown = true;
                else
                    m_leapSecondKnown = false;
            }
            else if (child->getName() == "timeAccuracy") {
                m_timeAccuracy = getValueInt(child);
            }
        }
    }
}

PivotTimestamp::PivotTimestamp(Datapoint* timestampData)
{
    DatapointValue& dpv = timestampData->getData();
    m_valueArray = new uint8_t[7];

    if (dpv.getType() == DatapointValue::T_DP_DICT)
    {
        std::vector<Datapoint*>* datapoints = dpv.getDpVec();

        for (Datapoint* child : *datapoints)
        {
            if (child->getName() == "SecondSinceEpoch") {
                uint32_t secondSinceEpoch = getValueInt(child);

                m_valueArray[0] = (secondSinceEpoch / 0x1000000 & 0xff);
                m_valueArray[1] = (secondSinceEpoch / 0x10000 & 0xff);
                m_valueArray[2] = (secondSinceEpoch / 0x100 & 0xff);
                m_valueArray[3] = (secondSinceEpoch & 0xff);
            }
            else if (child->getName() == "FractionOfSecond") {
                uint32_t fractionOfSecond = getValueInt(child);

                m_valueArray[4] = ((fractionOfSecond >> 16) & 0xff);
                m_valueArray[5] = ((fractionOfSecond >> 8) & 0xff);
                m_valueArray[6] = (fractionOfSecond & 0xff);
            }
            else if (child->getName() == "TimeQuality") {
                handleTimeQuality(child);
            }
        }
    }
}

PivotTimestamp::PivotTimestamp(uint64_t ms)
{
    m_valueArray = new uint8_t[7];
    uint32_t timeval32 = (uint32_t) (ms/ 1000LL);

    m_valueArray[0] = (timeval32 / 0x1000000 & 0xff);
    m_valueArray[1] = (timeval32 / 0x10000 & 0xff);
    m_valueArray[2] = (timeval32 / 0x100 & 0xff);
    m_valueArray[3] = (timeval32 & 0xff);

    uint32_t remainder = (ms % 1000LL);
    uint32_t fractionOfSecond = (remainder) * 16777 + ((remainder * 216) / 1000);

    m_valueArray[4] = ((fractionOfSecond >> 16) & 0xff);
    m_valueArray[5] = ((fractionOfSecond >> 8) & 0xff);
    m_valueArray[6] = (fractionOfSecond & 0xff);
}

PivotTimestamp::~PivotTimestamp()
{
    delete[] m_valueArray;
}

void
PivotTimestamp::setTimeInMs(uint64_t ms){
    uint32_t timeval32 = (uint32_t) (ms/ 1000LL);

    m_valueArray[0] = (timeval32 / 0x1000000 & 0xff);
    m_valueArray[1] = (timeval32 / 0x10000 & 0xff);
    m_valueArray[2] = (timeval32 / 0x100 & 0xff);
    m_valueArray[3] = (timeval32 & 0xff);

    uint32_t remainder = (ms % 1000LL);
    uint32_t fractionOfSecond = (remainder) * 16777 + ((remainder * 216) / 1000);

    m_valueArray[4] = ((fractionOfSecond >> 16) & 0xff);
    m_valueArray[5] = ((fractionOfSecond >> 8) & 0xff);
    m_valueArray[6] = (fractionOfSecond & 0xff);
}

uint64_t
PivotTimestamp::getTimeInMs(){
    uint32_t timeval32;

    timeval32 = m_valueArray[3];
    timeval32 += m_valueArray[2] * 0x100;
    timeval32 += m_valueArray[1] * 0x10000;
    timeval32 += m_valueArray[0] * 0x1000000;

    uint32_t fractionOfSecond = 0;

    fractionOfSecond = (m_valueArray[4] << 16);
    fractionOfSecond += (m_valueArray[5] << 8);
    fractionOfSecond += (m_valueArray[6]);

    uint32_t remainder = fractionOfSecond / 16777;

    uint64_t msVal = (timeval32 * 1000LL) + remainder;

    return (uint64_t) msVal;
}

int
PivotTimestamp::FractionOfSecond(){
    uint32_t fractionOfSecond = 0;

    fractionOfSecond = (m_valueArray[4] << 16);
    fractionOfSecond += (m_valueArray[5] << 8);
    fractionOfSecond += (m_valueArray[6]);

    return fractionOfSecond;
}

int
PivotTimestamp::SecondSinceEpoch(){
    int32_t timeval32;

    timeval32 = m_valueArray[3];
    timeval32 += m_valueArray[2] * 0x100;
    timeval32 += m_valueArray[1] * 0x10000;
    timeval32 += m_valueArray[0] * 0x1000000;

    return timeval32;
}


bool
IEC61850Datapoint::updateDatapoint(Datapoint* value, Datapoint* timestamp, Datapoint* quality){
  
  DatapointValue valueData = value->getData();
  
  switch(m_cdc){
    case SPS:{
      if(valueData.getType() != DatapointValue::T_INTEGER){
        Iec61850Utility::log_error("Invalid value type for SpsTyp");
        return false;
      }

      int intVal = valueData.toInt();

      if(intVal != 0 && intVal != 1){
        Iec61850Utility::log_error("Sps value not a boolean");
      }
        
      m_intVal = intVal;
      m_hasIntVal = true;
      break;
    }
      
    case DPS:{
      if(valueData.getType() != DatapointValue::T_STRING){
        Iec61850Utility::log_error("Invalid value type for DpsTyp");
        return false;
      }

      std::string stringVal = valueData.toStringValue();

      if (stringVal == "intermediate-state") m_intVal = 0;
      else if (stringVal == "off") m_intVal = 1;
      else if (stringVal == "on") m_intVal = 2;
      else if (stringVal == "bad-state") m_intVal = 3;
      
      m_hasIntVal = true;
      break;
    }
    
    case MV:{
      if(valueData.getType() == DatapointValue::T_INTEGER){
        m_intVal = valueData.toInt();
        m_hasIntVal = true;
        break;
      }
      else if(valueData.getType() == DatapointValue::T_FLOAT){
        m_floatVal = valueData.toDouble();
        m_hasIntVal = false;
        break;
      }
      else{
        Iec61850Utility::log_error("Invalid value type for MvTyp");
        return false;
      }
      break;
    }

    case BSC:{
      Datapoint* posValDp = getChild(value,"posVal");
       if(!posValDp){
          Iec61850Utility::log_error("No posVal");
          return false;
       }
       if(posValDp->getData().getType() != DatapointValue::T_INTEGER){
         Iec61850Utility::log_error("posVal wrong type");
         return false;
      }

      m_intVal = posValDp->getData().toInt();

      m_hasIntVal = true;

      Datapoint* transInd = getChild(value, "transInd");

      if(!transInd){
        Iec61850Utility::log_error("No transInd");
        return false;
      }
      
      if(transInd->getData().getType() != DatapointValue::T_INTEGER){
        Iec61850Utility::log_error("transInd wrong type");
        return false;
      }
      
      isTransient = transInd->getData().toInt();
      
      break;
    }

    default:{
      Iec61850Utility::log_error("Invalid cdc class");
    }
  }

  m_timestamp = new PivotTimestamp(timestamp);

  setQuality(quality);

  return true;
}

void
IEC61850Datapoint::setQuality(Datapoint* qualityDp){
  Datapoint* validityDp = getChild(qualityDp,"Validity");
  
  Quality* qualityPointer = &m_quality;

  if(validityDp){
    const std::string validity = validityDp->getData().toStringValue();

    if     (validity == "good")        {Quality_setValidity(qualityPointer,QUALITY_VALIDITY_GOOD);}
    else if(validity == "invalid")     {Quality_setValidity(qualityPointer,QUALITY_VALIDITY_INVALID);}
    else if(validity == "reserved")    {Quality_setValidity(qualityPointer,QUALITY_VALIDITY_RESERVED);}
    else if(validity == "questionable"){Quality_setValidity(qualityPointer,QUALITY_VALIDITY_QUESTIONABLE);}
  }
  
  Datapoint* testDp = getChild(qualityDp,"test");
  if(testDp){
    const int test =  testDp->getData().toInt();

    if(test == 0) Quality_unsetFlag(qualityPointer, QUALITY_TEST);
    else if(test == 1) Quality_unsetFlag(qualityPointer, QUALITY_TEST);
  }

  Datapoint* operatorBlockedDp = getChild(qualityDp,"operatorBlocked");
  
  if(operatorBlockedDp){
    const int operatorBlocked = operatorBlockedDp->getData().toInt();

    if(operatorBlocked == 0) Quality_unsetFlag(qualityPointer, QUALITY_OPERATOR_BLOCKED);
    else if (operatorBlocked == 1) Quality_setFlag(qualityPointer, QUALITY_OPERATOR_BLOCKED);
  }

  Datapoint* sourceDp = getChild(qualityDp, "Source");

  if(sourceDp){
    const std::string source = sourceDp->getData().toStringValue();

    if(source == "substituted") Quality_setFlag(qualityPointer, QUALITY_SOURCE_SUBSTITUTED);
    else Quality_unsetFlag(qualityPointer, QUALITY_SOURCE_SUBSTITUTED);
  }

  Datapoint* detailQualityDp = getChild(qualityDp, "DetailQuality");

  if(detailQualityDp){
    Datapoint* overflowDp = getChild(detailQualityDp, "overflow");
    Datapoint* outOfRangeDp = getChild(detailQualityDp, "outOfRange");
    Datapoint* badReferenceDp = getChild(detailQualityDp,"badReference");
    Datapoint* oscillatoryDp = getChild(detailQualityDp, "oscillatory");
    Datapoint* failureDp = getChild(detailQualityDp, "failure");
    Datapoint* oldDataDp = getChild(detailQualityDp, "oldData");
    Datapoint* inconsistentDp = getChild(detailQualityDp, "inconsistent");
    Datapoint* inaccurateDp = getChild(detailQualityDp, "inaccurate");
    
    if(overflowDp){
      if(overflowDp->getData().toInt() == 0) Quality_unsetFlag(qualityPointer, QUALITY_DETAIL_OVERFLOW);
      else if (overflowDp->getData().toInt() == 1) Quality_setFlag(qualityPointer, QUALITY_DETAIL_OVERFLOW);
    }
    if(outOfRangeDp){
      if(outOfRangeDp->getData().toInt() == 0) Quality_unsetFlag(qualityPointer, QUALITY_DETAIL_OUT_OF_RANGE);
      else if (outOfRangeDp->getData().toInt() == 1) Quality_setFlag(qualityPointer, QUALITY_DETAIL_OUT_OF_RANGE);
    }
    if(badReferenceDp){
      if(badReferenceDp->getData().toInt() == 0) Quality_unsetFlag(qualityPointer, QUALITY_DETAIL_OVERFLOW);
      else if (badReferenceDp->getData().toInt() == 1) Quality_setFlag(qualityPointer, QUALITY_DETAIL_BAD_REFERENCE);
    }
    if(oscillatoryDp){
      if(oscillatoryDp->getData().toInt() == 0) Quality_unsetFlag(qualityPointer, QUALITY_DETAIL_OSCILLATORY);
      else if (oscillatoryDp->getData().toInt() == 1) Quality_setFlag(qualityPointer, QUALITY_DETAIL_OSCILLATORY);
    }
    if(failureDp){
      if(failureDp->getData().toInt() == 0) Quality_unsetFlag(qualityPointer, QUALITY_DETAIL_FAILURE);
      else if (failureDp->getData().toInt() == 1) Quality_setFlag(qualityPointer, QUALITY_DETAIL_FAILURE);
    }
    if(oldDataDp){
      if(oldDataDp->getData().toInt() == 0) Quality_unsetFlag(qualityPointer, QUALITY_DETAIL_OLD_DATA);
      else if (oldDataDp->getData().toInt() == 1) Quality_setFlag(qualityPointer, QUALITY_DETAIL_OLD_DATA);
    }
    if(inconsistentDp){
      if(inconsistentDp->getData().toInt() == 0) Quality_unsetFlag(qualityPointer, QUALITY_DETAIL_INCONSISTENT);
      else if (inconsistentDp->getData().toInt() == 1) Quality_setFlag(qualityPointer, QUALITY_DETAIL_INCONSISTENT);
    }
    if(inaccurateDp){
      if(inaccurateDp->getData().toInt() == 0) Quality_unsetFlag(qualityPointer, QUALITY_DETAIL_INACCURATE);
      else if (inaccurateDp->getData().toInt() == 1) Quality_setFlag(qualityPointer, QUALITY_DETAIL_INACCURATE);
    }
  }
}

int 
IEC61850Datapoint::getCDCRootFromString(const std::string& rootStr)
{
  auto it = rootStrMap.find(rootStr);
  if(it != rootStrMap.end()) return it->second;
  return -1;
}


