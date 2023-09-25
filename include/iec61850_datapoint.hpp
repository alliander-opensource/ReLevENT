#ifndef IEC61850_DATAPOINT_H
#define IEC61850_DATAPOINT_H

#include <map>
#include <string>

class IEC61850Datapoint {
 public:
  typedef enum { SPS, DPS, BSC, MV, SPC, DPC, APC, INC } CDCTYPE;

  IEC61850Datapoint(const std::string& label, const std::string& objref,
                    CDCTYPE type);
  ~IEC61850Datapoint() = default;

  static int getCdcTypeFromString(const std::string& cdc);

 private:
  std::string m_label;
  std::string m_objref;

  CDCTYPE m_cdc;
};

#endif
