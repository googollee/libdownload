#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include "utility/BitMap.h"

typedef int TaskId;

typedef enum
{
    BYTES,
    KILOBYTES,
    MEGABYTES,
    GIGABYTES,
} SpeedUnit;

typedef struct
{
    const gchar *url;
    const gchar *outputPath;
    const gchar *outputName;

    // Options is a string like:
    // <key1>value1</key1>
    // <key2>value2</key2>
    //...
    gchar *options;

    // if (total_size == 0) && (finish_size > 0) means not start download.
    size_t totalSize;
    size_t finishSize;

    BitMap bitMap;
    const gchar *comment;

    // no need save below
    size_t totalConnections;
    size_t validConnections;

    double downloadSpeed;
    speed_unit downloadUnit;
    double uploadSpeed;
    speed_unit uploadUnit;
} TaskInfo;

#endif
