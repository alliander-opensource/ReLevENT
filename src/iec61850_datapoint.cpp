#include "iec61850_datapoint.hpp"

#include <string>


std::map<std::string, IEC61850Datapoint::CDCTYPE> cdcMap = {
    {"SpsTyp", IEC61850Datapoint::SPS}, {"DpsTyp", IEC61850Datapoint::DPS},
    {"BscTyp", IEC61850Datapoint::BSC}, {"MvTyp", IEC61850Datapoint::MV},
    {"SpcTyp", IEC61850Datapoint::SPC}, {"DpcTyp", IEC61850Datapoint::DPC},
    {"ApcTyp", IEC61850Datapoint::APC}, {"IncTyp", IEC61850Datapoint::INC}};

IEC61850Datapoint::IEC61850Datapoint(const std::string& label,
                                     const std::string& objRef, CDCTYPE cdc) {
  m_label = label;
  m_objref = objRef;
  m_cdc = cdc;
}

int
IEC61850Datapoint::getCdcTypeFromString( const std::string& cdc) {
  auto it = cdcMap.find(cdc);
  if (it != cdcMap.end()) {
    return it->second;
  }
  return -1;
}
