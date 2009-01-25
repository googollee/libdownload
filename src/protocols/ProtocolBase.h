#ifndef PROTOCOL_BASE_HEADER
#define PROTOCOL_BASE_HEADER

#include "../Global.h"

#include <boost/signals.hpp>

#include <iterator>

class ProtocolBase
{
public:
    typedef void ErrorCallback(TaskId id, int errno);
    typedef void FinishedCallback(TaskId id, int errno);

    ProtocolBase();
    virtual ~ProtocolBase();

    // Return protocol name for display, in utf8 codec
    virtual const char* name() = 0;

    // Check whether uri can be processed by this protocol
    // uri should in utf8 codec
    virtual bool canProcess(const char *uri) = 0;

    // Options
    // opt is a utf8 string like:
    // <key1>value1</key1>
    // <key2>value2</key2>
    //...
    virtual void setOptions(const char* opts) = 0;

    // Return the all options for protocol. For example, max opened socket.
    // return format must be below in utf8 codec:
    // <key1 default="default value" help="help of key1">value</key1>
    // <key2 default="default value" help="help of key2" /> <!-- short for only default value -->
    virtual const char* getAllOptions() = 0;

    // Task control
    // this options is session related
    // info is controled by manager, can modify in protocol
    virtual void addTask(const TaskId id, TaskInfo *info) = 0;
    virtual void delTask(const TaskId id) = 0;
    virtual bool hasTask(const TaskId id) = 0;

    // Save and load
    virtual void saveTask(const TaskId id,
                          std::ostream_iterator<char> &out) = 0;
    virtual void loadTask(const TaskId id,
                          std::istream_iterator<char> &begin,
                          std::istream_iterator<char> &end) = 0;

    // Perform download
    virtual void getFdSet(fd_set *read_fd_set,
                          fd_set *write_fd_set,
                          fd_set *exc_fd_set,
                          int *max_fd) = 0;

    // return the downloading items number
    // downloaded and uploaded speed unit is byte.
    virtual size_t perform(size_t *downloaded, size_t *uploaded) = 0;

    boost::signal<ErrorCallback> errorSignal;
    boost::signal<FinishedCallback> finishSignal;

private:
    ProtocolBase(const ProtocolBase &);
    bool operator=(const ProtocolBase &);
};

inline ProtocolBase::ProtocolBase()
{}

inline ProtocolBase::~ProtocolBase()
{}

#endif
