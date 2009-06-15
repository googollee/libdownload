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

    ProtocolFactory();
    ~ProtocolFactory();

    bool addProtocol(std::auto_ptr<ProtocolBase> protocol);
    ProtocolBase* getProtocol(const char *uri);

private:
    friend class DownloadManager;

    Protocols protocols_;
};

inline ProtocolBase* ProtocolFactory::getProtocol(const char *uri)
{
    for (Protocols::iterator it = protocols_.begin();
         it != protocols_.end();
         ++it)
    {
        if ((*it)->canProcess(uri))
            return *it;
    }

    return NULL;
}

#endif
