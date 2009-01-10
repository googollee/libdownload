#ifndef PROTOCOL_BASE_HEADER
#define PROTOCOL_BASE_HEADER

#include "../Global.h"

#include <glib.h>

#include <boost/signals.hpp>

#include <iterator>

class ProtocolBase
{
public:
    typedef void ErrorCallback(TaskId id, int errno);
    typedef void FinishedCallback(TaskId id, int errno);

    Protocol();
    virtual ~Protocol();

    // Return protocol name for display, in utf8 codec
    virtual const gchar* name() = 0;

    // Check whether uri can be processed by this protocol
    // uri should in utf8 codec
    virtual bool canProcess(const gchar *uri) = 0;

    // Options
    // opt is a utf8 string like:
    // <key1>value1</key1>
    // <key2>value2</key2>
    //...
    virtual void setOptions(const gchar* opts) = 0;

    // Return the all options for protocol. For example, max opened socket.
    // return format must be below in utf8 codec:
    // <key1 default="default value" help="help of key1">value</key1>
    // <key2 default="default value" help="help of key2" /> <!-- short for only default value -->
    virtual const gchar* getAllOptions() = 0;

    // Task control
    // this options is session related
    // info is controled by manager, can modify in protocol
    virtual void addTask(const TaskID id, TaskInfo *info) = 0;
    virtual void delTask(const TaskID id) = 0;
    virtual bool hasTask(const TaskID id) = 0;
    virtual BitMap getTaskBitmap(const TaskID id) = 0;

    // Save and load
    virtual void saveTask(std::ostream_iterator &out) = 0;
    virtual void loadTask(const TaskID id,
                          std::istream_iterator &begin,
                          std::istream_iterator &end) = 0;

    // Perform download
//     virtual void getFdSet(fd_set *read_fd_set,
//                           fd_set *write_fd_set,
//                           fd_set *exc_fd_set,
//                           int *max_fd) = 0;

    // return the downloading items number
    // downloaded and uploaded speed unit is byte.
    virtual size_t performDownload(size_t *downloaded) = 0;
    virtual size_t performUpload(size_t *uploaded) = 0;

    boost::signal<ErrorCallback> errorSignal;
    boost::signal<FinishedCallback> finishSignal;

private:
    Protocol(const Protocol &);
    bool operator=(const Protocol &);
};

inline Protocol::Protocol()
{}

inline Protocol::~Protocol()
{}

#endif
