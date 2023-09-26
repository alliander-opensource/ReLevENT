#ifndef IEC61850_DATAPOINT_H
#define IEC61850_DATAPOINT_H

#include <cstdint>
#include <map>
#include <string>

#include "libiec61850/iec61850_common.h"
#include "libiec61850/iec61850_model.h"
#include "datapoint.h"


class DataAttributesDp {
  public:
    DataAttribute* value;
    DataAttribute* q;
    DataAttribute* t; 
};

class IEC61850Datapoint {
 public:
  
  typedef struct sDataAttributesDp* DataAttributesDp;


  typedef enum { GTIS, GTIM, GTIC } ROOT;
  typedef enum { SPS, DPS, BSC, MV, SPC, DPC, APC, INC } CDCTYPE;

  IEC61850Datapoint(const std::string& label, const std::string& objref,
                    CDCTYPE type, DataAttributesDp dadp);
  ~IEC61850Datapoint() = default;

  static int getCdcTypeFromString(const std::string& cdc);
  static int getRootFromCDC(const CDCTYPE cdc);
  
  const std::string getObjRef(){return m_objref;};
  
  void setQuality(Datapoint* qualityDp);

  bool updateDatapoint(Datapoint* value, Datapoint* timestamp, Datapoint* quality, bool timeSynced);

private:
  std::string m_label;
  std::string m_objref;
  
  DataAttributesDp m_dadp;

  bool isTransient;

  int m_intVal;
  float m_floatVal;
  std::string m_stringVal;

  Quality* m_quality;
  Timestamp m_timestamp;
  
  CDCTYPE m_cdc;
};

#endif
