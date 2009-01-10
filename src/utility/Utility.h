#ifndef DOWNLOAD_UTILITY_CLASSES_HEAD
#define DOWNLOAD_UTILITY_CLASSES_HEAD

class Noncopiable
{
public:
    Noncopiable() {}
    ~Noncopiable() {}

private:
    Noncopiable(const Noncopiable &);
    const Noncopiable& operator=(const Noncopiable &);
};

#include "DownloadException.h"

#endif
