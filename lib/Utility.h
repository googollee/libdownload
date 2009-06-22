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

#ifdef _DEBUG
#define LOG(level, ...) printf(__VA_ARGS__)
#else
#define LOG(level, ...)
#endif //_DEBUG

#endif
