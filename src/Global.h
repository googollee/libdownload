#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include "utility/BitMap.h"

#include <string>

typedef int TaskId;

enum TaskState
{
    WAITING,
    DOWNLOAD,
    FINISH,
    ERROR,
};

class ProtocolBase;

struct TaskInfo
{
    TaskId id;

    char *url;
    char *outputPath;
    char *outputName;

    // Options is a string like:
    // <key1>value1</key1>
    // <key2>value2</key2>
    //...
    char *options;

    size_t totalSize;

    BitMap downloadMap;
    char *comment;

    // no need save below
    size_t downloadSize;
    size_t uploadSize;

    BitMap validMap;

    int totalSource;
    int validSource;

    TaskState state;
    ProtocolBase *protocol;

    TaskInfo()
        : id(0),
          url(NULL),
          outputPath(NULL),
          outputName(NULL),
          options(NULL),
          totalSize(0),
          comment(NULL),
          downloadSize(0),
          uploadSize(0),
          totalSource(0),
          validSource(0),
          state(WAITING),
          protocol(NULL)
        {}
};

#endif
