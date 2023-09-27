#ifndef IEC61850_DATAPOINT_H
#define IEC61850_DATAPOINT_H

#include <cstdint>
#include <map>
#include <string>
#include <memory>

#include "libiec61850/iec61850_common.h"
#include "libiec61850/iec61850_model.h"
#include "datapoint.h"

typedef enum { GTIS, GTIM, GTIC } PIVOTROOT;
typedef enum { SPS, DPS, BSC, MV, SPC, DPC, APC, INC } CDCTYPE;

class DataAttributesDp {
  public:
    DataAttribute* value;
    DataAttribute* q;
    DataAttribute* t; 
};

class IEC61850Datapoint {
 public:
  IEC61850Datapoint(const std::string& label, const std::string& objref,
                    CDCTYPE type, std::shared_ptr<DataAttributesDp> dadp);
  ~IEC61850Datapoint() = default;

  static int getCdcTypeFromString(const std::string& cdc);
  static int getRootFromCDC(const CDCTYPE cdc);
  
  CDCTYPE getCDC(){return m_cdc;};

  const std::string getObjRef(){return m_objref;};
  
  std::shared_ptr<DataAttributesDp> getDadp(){return m_dadp;};

  const uint64_t getMsTimestamp(){return m_timestamp;};

  const Quality getQuality(){return m_quality;};

  void setQuality(Datapoint* qualityDp);
    
  const long getIntVal(){return m_intVal;};

  const float getFloatVal(){return m_floatVal;};
  
  const bool hasIntVal(){return m_hasIntVal;};

  bool updateDatapoint(Datapoint* value, Datapoint* timestamp, Datapoint* quality);

private:
  std::string m_label;
  std::string m_objref;
  
  std::shared_ptr<DataAttributesDp> m_dadp;

  bool isTransient;

  long m_intVal;
  float m_floatVal;
  
  bool m_hasIntVal;

  Quality m_quality;
  uint64_t m_timestamp;
  
  CDCTYPE m_cdc;
};

#endif
