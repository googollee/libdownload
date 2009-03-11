#ifndef PROTOCOL_FACTORY_HEADER
#define PROTOCOL_FACTORY_HEADER

#include "Global.h"

#include <protocols/ProtocolBase.h>
#include <utility/Utility.h>

#include <string.h>

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

class ProtocolFactory : private Noncopiable
{
public:
    typedef std::vector<ProtocolBase*> Protocols;

    static bool addProtocol(std::auto_ptr<ProtocolBase> protocol);
    static ProtocolBase* getProtocol(const char *url);
    ~ProtocolFactory();

private:
    static ProtocolFactory& instance();

    ProtocolFactory();

    Protocols protocols_;
};

inline ProtocolFactory& ProtocolFactory::instance()
{
    static ProtocolFactory instance;
    return instance;
}

bool checkCanProcess(ProtocolBase *protocol, const char *url)
{
    return protocol->canProcess(url);
}

inline ProtocolBase* ProtocolFactory::getProtocol(const char *uri)
{
    Protocols::iterator ret = find_if(protocols_.begin(),
                                      protocols_.end(),
                                      bind_2nd(checkCanProcess, uri));

    if (ret == protocols_.end())
        return 0;

    return *ret;
}

#endif
