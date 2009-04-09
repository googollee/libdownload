#include "ProtocolFactory.h"

using namespace std;

bool ProtocolFactory::addProtocol(auto_ptr<ProtocolBase> protocol)
{
    ProtocolBase *p = protocol.release();

    if (p == 0)
        return false;

    Protocols::iterator it = find(protocols_.begin(), protocols_.end(), p);

    if (it != protocols_.end())
        return false;

    protocols_.push_back(p);

    return true;
}

ProtocolFactory::ProtocolFactory()
{}

ProtocolFactory::~ProtocolFactory()
{
    for (Protocols::iterator it = protocols_.begin();
         it != protocols_.end();
         ++it)
    {
        delete *it;
    }
}
