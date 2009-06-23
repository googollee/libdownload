#ifndef ALLOCATOR_CLASS_HEAD
#define ALLOCATOR_CLASS_HEAD

#include <sys/types.h>

namespace Utility
{

class Allocator
{
public:
    template <typename T>
    static T* alloc()
        {
            return new T;
        }

    template <typename T>
    static T* allocArray(size_t size)
        {
            return new T[size];
        }

    template <typename T>
    static void free(T* p)
        {
            delete p;
        }

    template <typename T>
    static void freeArray(T* p)
        {
            delete [] p;
        }
};

}

#endif
