#ifndef IEC61850_CONFIG_H
#define IEC61850_CONFIG_H

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

class IEC61850Config
{
public:
    IEC61850Config();
    IEC61850Config(const std::string& protocolConfig,
                   const std::string& exchangeConfig);
    ~IEC61850Config();

    void importProtocolConfig(const std::string& protocolConfig);
    void importExchangeConfig(const std::string& exchangeConfig);
    void importTlsConfig     (const std::string& tlsConfig);
    void importSchedulerConfig(const std::string& schedulerConfig);
    
    int TcpPort();
    bool bindOnIp() {return m_bindOnIp;};
    std::string ServerIp();
    
    bool schedulerEnabled(){return m_useScheduler;};
    bool TLSEnabled(){return m_useTLS;};

    std::map<std::string, std::shared_ptr<IEC61850Datapoint>>* getExchangeDefinitions() {return m_exchangeDefinitions;};

private:

    static bool isValidIPAddress(const std::string& addrStr);

    void deleteExchangeDefinitions();

    std::string m_ip = "";
    int m_tcpPort = -1;
    
    bool m_useScheduler = false;
    bool m_useTLS = false;

    bool m_bindOnIp;
    bool m_protocolConfigComplete;
    bool m_exchangeConfigComplete;

    std::map<std::string, std::shared_ptr<IEC61850Datapoint>>* m_exchangeDefinitions = nullptr;
    
    std::string m_privateKey;
    std::string m_ownCertificate;
    std::vector<std::string> m_remoteCertificates;
    std::vector<std::string> m_caCertificates;
};



#endif
