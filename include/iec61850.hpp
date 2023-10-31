#ifndef _IEC61850SERVER_H
#define _IEC61850SERVER_H

#include <cstdint>
#include <libiec61850/mms_value.h>
#include <memory>
#include <reading.h>
#include <config_category.h>
#include <logger.h>
#include <gtest/gtest.h>
#include <plugin_api.h>

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>

#include "der_scheduler.h"
#include "iec61850_config.hpp"
#include "iec61850_datapoint.hpp"
#include "iec61850_utility.hpp"
#include "iec61850_scheduler_config.hpp"
#include "libiec61850/iec61850_server.h"
#include "libiec61850/hal_thread.h"
#include "libiec61850/hal_time.h"

struct sOutputData {
    char* targetObjRef;
    char* targetValue;
};

typedef struct sOutputData* OutputData;

class IEC61850ServerException : public std::exception //NOSONAR
{
 public:
    explicit IEC61850ServerException(const std::string& context):
        m_context(context) {}

    const std::string& getContext(void) {return m_context;};

 private:
    const std::string m_context;
};

class IEC61850Server
{
  public:
    IEC61850Server();
    ~IEC61850Server();

    void setJsonConfig(const std::string& stackConfig,
                       const std::string& dataExchangeConfig,
                       const std::string& tlsConfig,
                       const std::string& schedulerConfig);
    
    void setModelPath(const std::string& path){m_modelPath = path;};
    void configure (const ConfigCategory* conf);
    uint32_t send(const std::vector<Reading*>& readings);
    void stop();
    void registerControl(int (* operation)(char *operation, int paramCount, char* names[], char *parameters[], ControlDestination destination, ...));
    bool forwardCommand(ControlAction action, MmsValue* ctlVal, bool test, IEC61850Datapoint* dp);
    bool forwardScheduleCommand(MmsValue* ctlVal, bool test, IEC61850Datapoint* dp, uint64_t timestampMs);
    void updateDatapointInServer(std::shared_ptr<IEC61850Datapoint>, bool timeSynced);
    const std::string getObjRefFromID(const std::string& id);
    IEC61850Config* getConfig(){return m_config;};
    Datapoint* buildPivotOperation(CDCTYPE type, MmsValue* ctlVal, bool test, bool isSelect, const std::string& label, PivotTimestamp* timestamp, bool hasSelect);
    Datapoint* ControlActionToPivot(ControlAction action, MmsValue* ctlVal, bool test, IEC61850Datapoint* dp);
    static void scheduler_TargetValueChanged(void* parameter, const char* targetValueObjRef, MmsValue* value, Quality quality, uint64_t timestampMs);

  private:
    
    Semaphore outputQueueLock = nullptr;
    LinkedList outputQueue = nullptr;
 
    IedServer m_server = nullptr;
    IedModel* m_model = nullptr;
    Scheduler m_scheduler = nullptr;
    
    std::string m_modelPath;
    
    bool m_started; 
    std::string m_name;
    IEC61850Config* m_config = nullptr;
    Logger* m_log;
    IEC61850SchedulerConfig* m_schedulerConfig; 

    std::map<std::string, std::shared_ptr<IEC61850Datapoint>>* m_exchangeDefinitions = nullptr;

    int (*m_oper)(char *operation, int paramCount, char* names[], char* parameters[], ControlDestination destination, ...) = NULL;

    bool createTLSConfiguration();
    
    static ControlHandlerResult controlHandler(ControlAction action, void* parameter, MmsValue* value, bool test);

    static MmsDataAccessError writeAccessHandler (DataAttribute* dataAttribute, MmsValue* value, ClientConnection connection, void* parameter);
    
    static CheckHandlerResult checkHandler(ControlAction action, void* parameter, MmsValue* ctlVal, bool test, bool interlockCheck);

    bool forwardCommand();

    FRIEND_TEST(ConnectionHandlerTest,NormalConnection);
};

class ServerDatapointPair {
  public:
    IEC61850Server* server;
    IEC61850Datapoint* dp;
};

#endif
