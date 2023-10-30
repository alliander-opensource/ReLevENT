#ifndef IEC61850_DATAPOINT_H
#define IEC61850_DATAPOINT_H

#include <cstdint>
#include <map>
#include <string>
#include <memory>

#include "libiec61850/iec61850_common.h"
#include "libiec61850/iec61850_model.h"
#include "datapoint.h"
#include "iec61850_utility.hpp"

typedef enum { GTIS, GTIM, GTIC } PIVOTROOT;
typedef enum { SPS, DPS, BSC, MV,INS, ENS, SPC, DPC, APC, INC } CDCTYPE;

class DataAttributesDp {
  public:
    DataAttribute* value;
    DataAttribute* q;
    DataAttribute* t; 
    DataAttribute* mmsVal;
};

class PivotTimestamp
{
public:
    PivotTimestamp(Datapoint* timestampData);
    PivotTimestamp(uint64_t ms);
    ~PivotTimestamp();

    void setTimeInMs(uint64_t ms);

    int SecondSinceEpoch();
    int FractionOfSecond();
    uint64_t getTimeInMs();

    bool ClockFailure() {return m_clockFailure;};
    bool LeapSecondKnown() {return m_leapSecondKnown;};
    bool ClockNotSynchronized() {return m_clockNotSynchronized;};
    int TimeAccuracy() {return m_timeAccuracy;};

    static uint64_t GetCurrentTimeInMs();

private:

    void handleTimeQuality(Datapoint* timeQuality);

    uint8_t* m_valueArray;

    int m_secondSinceEpoch;
    int m_fractionOfSecond;

    int m_timeAccuracy;
    bool m_clockFailure = false;
    bool m_leapSecondKnown = false;
    bool m_clockNotSynchronized = false;
};

class IEC61850Datapoint {
 public:
  IEC61850Datapoint(const std::string& label, const std::string& objref,
                    CDCTYPE type, std::shared_ptr<DataAttributesDp> dadp);
  ~IEC61850Datapoint() ;

  static int getCdcTypeFromString(const std::string& cdc);
  static int getRootFromCDC(const CDCTYPE cdc);
  static int getCDCRootFromString(const std::string& rootStr);
  
  CDCTYPE getCDC(){return m_cdc;};

  const std::string getObjRef(){return m_objref;};
  const std::string getLabel(){return m_label;};
  
  std::shared_ptr<DataAttributesDp> getDadp(){return m_dadp;};

  PivotTimestamp* getTimestamp(){return m_timestamp;};

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
  PivotTimestamp* m_timestamp = nullptr;
  
  CDCTYPE m_cdc;
};

#endif
