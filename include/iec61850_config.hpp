#ifndef IEC61850_CONFIG_H
#define IEC61850_CONFIG_H

#include "der_scheduler.h"
#include "iec61850_datapoint.hpp"
#include "logger.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <algorithm>
#include <map>
#include <regex>
#include <sstream>
#include <vector>

#include <libiec61850/iec61850_server.h>

class SchedulerTarget {
public:
  std::string schedControllerRef;
  std::shared_ptr<IEC61850Datapoint> targetDp = nullptr;
  bool forwardOutput =
      false; /* forward target value output as PivotCommand operation  */
  bool forwardSchedule = false;    /*  forward target value schedule forecast as
                                      PivotCommand operation */
  int forwardScheduleInterval = 5; /* in seconds */

  int forwardSchedulePeriod = 86400; /* in seconds -> default = 24 h */

  uint64_t lastSchedulerForwarding = 0;
};

class IEC61850Config {
public:
  IEC61850Config();
  IEC61850Config(const std::string &protocolConfig,
                 const std::string &exchangeConfig);
  ~IEC61850Config();

  void importProtocolConfig(const std::string &protocolConfig);
  void importExchangeConfig(const std::string &exchangeConfig, IedModel *model);
  void importTlsConfig(const std::string &tlsConfig);
  void importSchedulerConfig(const std::string &schedulerConfig,
                             Scheduler sched);

  int TcpPort();
  bool bindOnIp() { return m_bindOnIp; };
  std::string ServerIp();

  bool schedulerEnabled() { return m_useScheduler; };
  bool TLSEnabled() { return m_useTLS; };

  std::map<std::string, std::shared_ptr<IEC61850Datapoint>> *
  getExchangeDefinitions() {
    return m_exchangeDefinitions;
  };

  std::map<std::string, std::shared_ptr<SchedulerTarget>> *
  getSchedulerTargets() {
    return m_schedulerTargets;
  };

  std::shared_ptr<IEC61850Datapoint>
  getDatapointByObjectReference(const std::string &objRef);

private:
  static bool isValidIPAddress(const std::string &addrStr);

  void deleteExchangeDefinitions();
  void deleteScheduleTargetDefinitions();

  std::string m_ip = "";
  int m_tcpPort = -1;

  bool m_useScheduler = false;
  bool m_useTLS = false;

  bool m_bindOnIp;
  bool m_protocolConfigComplete;
  bool m_exchangeConfigComplete;

  std::map<std::string, std::shared_ptr<IEC61850Datapoint>>
      *m_exchangeDefinitions = nullptr;
  std::map<std::string, std::shared_ptr<IEC61850Datapoint>>
      *m_exchangeDefinitionsObjRef = nullptr;
  std::map<std::string, std::shared_ptr<SchedulerTarget>> *m_schedulerTargets =
      nullptr;

  std::string m_privateKey;
  std::string m_ownCertificate;
  std::vector<std::string> m_remoteCertificates;
  std::vector<std::string> m_caCertificates;
};

#endif
