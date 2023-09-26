#include "iec61850_datapoint.hpp"
#include "datapoint.h"

#include <libiec61850/iec61850_common.h>
#include <string>


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

std::map<std::string, IEC61850Datapoint::CDCTYPE> cdcMap = {
    {"SpsTyp", IEC61850Datapoint::SPS}, {"DpsTyp", IEC61850Datapoint::DPS},
    {"BscTyp", IEC61850Datapoint::BSC}, {"MvTyp", IEC61850Datapoint::MV},
    {"SpcTyp", IEC61850Datapoint::SPC}, {"DpcTyp", IEC61850Datapoint::DPC},
    {"ApcTyp", IEC61850Datapoint::APC}, {"IncTyp", IEC61850Datapoint::INC}};

std::map<IEC61850Datapoint::CDCTYPE, IEC61850Datapoint::ROOT> rootMap = {
    {IEC61850Datapoint::SPS, IEC61850Datapoint::GTIS}, {IEC61850Datapoint::DPS, IEC61850Datapoint::GTIS},
    {IEC61850Datapoint::BSC, IEC61850Datapoint::GTIS}, {IEC61850Datapoint::MV,  IEC61850Datapoint::GTIM},
    {IEC61850Datapoint::SPC, IEC61850Datapoint::GTIC}, {IEC61850Datapoint::DPS, IEC61850Datapoint::GTIC},
    {IEC61850Datapoint::APC, IEC61850Datapoint::GTIC}, {IEC61850Datapoint::INC, IEC61850Datapoint::GTIC}
};


IEC61850Datapoint::IEC61850Datapoint(const std::string& label,
                                     const std::string& objRef, CDCTYPE cdc, DataAttributesDp dadp) {
  m_label = label;
  m_objref = objRef;
  m_cdc = cdc;
  m_dadp = dadp;
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

bool
IEC61850Datapoint::updateDatapoint(Datapoint* value, Datapoint* timestamp, Datapoint* quality, bool timeSynced){
  
  DatapointValue valueData = value->getData();

  switch(m_cdc){
    case SPS:{
      if(valueData.getType() != DatapointValue::T_INTEGER){
        Logger::getLogger()->error("Invalid value type for SpsTyp");
        return false;
      }

      int intVal = valueData.toInt();

      if(intVal != 0 && intVal != 1){
        Logger::getLogger()->error("Sps value not a boolean");
      }
        
      m_intVal = intVal;
      break;
    }
      
    case DPS:{
      if(valueData.getType() != DatapointValue::T_STRING){
        Logger::getLogger()->error("Invalid value type for DpsTyp");
        return false;
      }

      m_stringVal = valueData.toStringValue();
      break;
    }
    
    case MV:{
      if(valueData.getType() == DatapointValue::T_INTEGER){
        m_intVal = valueData.toInt();
        break;
      }
      else if(valueData.getType() == DatapointValue::T_FLOAT){
        m_floatVal = valueData.toDouble();
        break;
      }
      else{
        Logger::getLogger()->error("Invalid value type for MvTyp");
        return false;
      }
    }

    case BSC:{
      Datapoint* posValDp = getChild(value,"posVal");
       if(!posValDp){
          Logger::getLogger()->error("No posVal");
          return false;
       }
       if(posValDp->getData().getType() != DatapointValue::T_INTEGER){
         Logger::getLogger()->error("posVal wrong type");
         return false;
      }

      m_intVal = posValDp->getData().toInt();

      Datapoint* transInd = getChild(value, "transInd");

      if(!transInd){
        Logger::getLogger()->error("No transInd");
        return false;
      }
      
      if(transInd->getData().getType() != DatapointValue::T_INTEGER){
        Logger::getLogger()->error("transInd wrong type");
        return false;
      }
      
      isTransient = transInd->getData().toInt();
      
      break;
    }

    default:{
      Logger::getLogger()->error("Invalid cdc class");
    }
  }
    
  uint32_t timeval32 = getChild(timestamp, "secondSinceEpoch")->getData().toInt();

  uint32_t fractionOfSecond = getChild(timestamp,"fractionOfSecond")->getData().toInt();
  
  uint32_t remainder = fractionOfSecond / 16777;
  
  Timestamp_clearFlags(&m_timestamp);
  Timestamp_setTimeInMilliseconds(&m_timestamp,(uint64_t) ((timeval32 * 1000LL) + remainder));
  Timestamp_setSubsecondPrecision(&m_timestamp, 10);

  Timestamp_setClockNotSynchronized(&m_timestamp, !timeSynced);

  setQuality(quality);

  return true;
}

void
IEC61850Datapoint::setQuality(Datapoint* qualityDp){
  Datapoint* validityDp = getChild(qualityDp,"Validity");
  
  const std::string validity = validityDp->getData().toStringValue();

  if     (validity == "good"){Quality_setValidity(m_quality,QUALITY_VALIDITY_GOOD);}
  else if(validity == "invalid"){Quality_setValidity(m_quality,QUALITY_VALIDITY_INVALID);}
  else if(validity == "reserved"){Quality_setValidity(m_quality,QUALITY_VALIDITY_RESERVED);}
  else if(validity == "questionable"){Quality_setValidity(m_quality,QUALITY_VALIDITY_QUESTIONABLE);}
  
  Datapoint* testDp = getChild(qualityDp,"test");

  const int test =  testDp->getData().toInt();

  if(test == 0) Quality_unsetFlag(m_quality, QUALITY_TEST);
  else if(test == 1) Quality_unsetFlag(m_quality, QUALITY_TEST);
  
  Datapoint* operatorBlockedDp = getChild(qualityDp,"operatorBlocked");
  
  const int operatorBlocked = operatorBlockedDp->getData().toInt();

  if(operatorBlocked == 0) Quality_unsetFlag(m_quality, QUALITY_OPERATOR_BLOCKED);
  else if (operatorBlocked == 1) Quality_setFlag(m_quality, QUALITY_OPERATOR_BLOCKED);
  

  Datapoint* sourceDp = getChild(qualityDp, "Source");

  const std::string source = sourceDp->getData().toStringValue();

  if(source == "substituted") Quality_setFlag(m_quality, QUALITY_SOURCE_SUBSTITUTED);
  else Quality_unsetFlag(m_quality, QUALITY_SOURCE_SUBSTITUTED);
  
  Datapoint* detailQualityDp = getChild(qualityDp, "DetailQuality");


  Datapoint* overflowDp = getChild(detailQualityDp, "overflow");
  Datapoint* outOfRangeDp = getChild(detailQualityDp, "outOfRange");
  Datapoint* badReferenceDp = getChild(detailQualityDp,"badReference");
  Datapoint* oscillatoryDp = getChild(detailQualityDp, "oscillatory");
  Datapoint* failureDp = getChild(detailQualityDp, "failure");
  Datapoint* oldDataDp = getChild(detailQualityDp, "oldData");
  Datapoint* inconsistentDp = getChild(detailQualityDp, "inconsistent");
  Datapoint* inaccurateDp = getChild(detailQualityDp, "inaccurate");
  
  if(overflowDp){
    if(overflowDp->getData().toInt() == 0) Quality_unsetFlag(m_quality, QUALITY_DETAIL_OVERFLOW);
    else if (overflowDp->getData().toInt() == 1) Quality_setFlag(m_quality, QUALITY_DETAIL_OVERFLOW);
  }
  if(outOfRangeDp){
    if(outOfRangeDp->getData().toInt() == 0) Quality_unsetFlag(m_quality, QUALITY_DETAIL_OUT_OF_RANGE);
    else if (outOfRangeDp->getData().toInt() == 1) Quality_setFlag(m_quality, QUALITY_DETAIL_OUT_OF_RANGE);
  }
  if(badReferenceDp){
    if(badReferenceDp->getData().toInt() == 0) Quality_unsetFlag(m_quality, QUALITY_DETAIL_OVERFLOW);
    else if (badReferenceDp->getData().toInt() == 1) Quality_setFlag(m_quality, QUALITY_DETAIL_BAD_REFERENCE);
  }
  if(oscillatoryDp){
    if(oscillatoryDp->getData().toInt() == 0) Quality_unsetFlag(m_quality, QUALITY_DETAIL_OSCILLATORY);
    else if (oscillatoryDp->getData().toInt() == 1) Quality_setFlag(m_quality, QUALITY_DETAIL_OSCILLATORY);
  }
  if(failureDp){
    if(failureDp->getData().toInt() == 0) Quality_unsetFlag(m_quality, QUALITY_DETAIL_FAILURE);
    else if (failureDp->getData().toInt() == 1) Quality_setFlag(m_quality, QUALITY_DETAIL_FAILURE);
  }
  if(oldDataDp){
    if(oldDataDp->getData().toInt() == 0) Quality_unsetFlag(m_quality, QUALITY_DETAIL_OLD_DATA);
    else if (oldDataDp->getData().toInt() == 1) Quality_setFlag(m_quality, QUALITY_DETAIL_OLD_DATA);
  }
  if(inconsistentDp){
    if(inconsistentDp->getData().toInt() == 0) Quality_unsetFlag(m_quality, QUALITY_DETAIL_INCONSISTENT);
    else if (inconsistentDp->getData().toInt() == 1) Quality_setFlag(m_quality, QUALITY_DETAIL_INCONSISTENT);
  }
  if(inaccurateDp){
    if(inaccurateDp->getData().toInt() == 0) Quality_unsetFlag(m_quality, QUALITY_DETAIL_INACCURATE);
    else if (inaccurateDp->getData().toInt() == 1) Quality_setFlag(m_quality, QUALITY_DETAIL_INACCURATE);
  }
}
