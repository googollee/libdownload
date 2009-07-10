#ifndef LOCAL_PTR_CLASS_HEAD
#define LOCAL_PTR_CLASS_HEAD

namespace Utility
{

template <typename T>
class LocalPtr
{
public:
    typedef void (*FreeFunc)(T* ptr);

    LocalPtr(T* ptr, FreeFunc freeFunc)
        : ptr_(ptr),
          free_(freeFunc)
        {}

    ~LocalPtr()
        {
            reset();
        }

    void reset(T* p = 0)
        {
            if (ptr_ != 0) free_(ptr_);
            ptr_ = p;
        }

    T& operator*() const
        {
            return *ptr_;
        }

    T* operator->() const
        {
            return ptr_;
        }

    T* get() const
        {
            return ptr_;
        }

    void swap(LocalPtr<T>& p)
        {
            T* temp = ptr_;
            ptr_ = p;
            p = temp;
        }

private:
    T* ptr_;
    FreeFunc free_;
};

template<class T> void swap(LocalPtr<T>& lhs, LocalPtr<T>& rhs)
{
    lhs.swap(rhs);
}

}

#endif
