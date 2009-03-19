#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include "utility/BitMap.h"

#include <string>

enum TaskState
{
    TASK_WAIT,
    TASK_DOWNLOAD,
    TASK_FINISH,
    TASK_ERROR,
};

class ProtocolBase;

struct TaskInfo
{
    std::string uri;
    std::string outputPath;
    std::string outputName;
    // Options is a string like:
    // <key1>value1</key1>
    // <key2>value2</key2>
    //...
    std::string options;
    std::string comment;
    std::string processData;

    // no need save below
    size_t totalSize;
    size_t downloadSize;
    size_t uploadSize;

    BitMap validMap;
    BitMap downloadMap;

    int totalSource;
    int validSource;

    TaskState state;
    ProtocolBase *protocol;

    TaskInfo()
        : totalSize(0),
          downloadSize(0),
          uploadSize(0),
          totalSource(0),
          validSource(0),
          state(TASK_WAIT),
          protocol(NULL)
        {}
};

#endif
