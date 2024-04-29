#ifndef IEC61850_SCHEDULER_CONFIG_H
#define IEC61850_SCHEDULER_CONFIG_H



#include "iec61850_datapoint.hpp"
#include "logger.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <map>
#include <regex>
#include <vector>
#include <sstream>
#include <algorithm>

#include <libiec61850/iec61850_server.h>


class IEC61850SchedulerConfig
{
public:
    IEC61850SchedulerConfig();
    IEC61850SchedulerConfig(const std::string& schedulerConfig);
    ~IEC61850SchedulerConfig();

    void importProtocolSchedulerConfig(const std::string& protocolConfig);

  };

#endif
