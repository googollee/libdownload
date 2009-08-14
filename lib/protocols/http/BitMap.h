/**
 * \file BitMap.h
 *       BitMap class. Define the BitMap class which can contain the bit array and set value.
 */

#ifndef DOWNLOAD_BITMAP_CLASS_HEAD
#define DOWNLOAD_BITMAP_CLASS_HEAD

#include <vector>

/**
 * \brief It's a bit set(array of bits) in which some common APIs used to indicate things like saving process are wrapped.
 *
 * Unlike the bitmap structure of pictures, this class only offer a bitset. The main purpose
 * is to indicate the saving process of download.
 *
 * When downloading a file, it's common to devide it to same size block. For example:
 *   A file is 123400 bytes, and maybe protocol set the block size 100 bytes. Then the file state can be descripted by
 *   1234 bits, one bit indicate one block. At beginning, all bits are 0, and if one block downloaded, it will set as 1.
 *
 * This class can handle a bit set used to save the download process. When one or more blocks finished, you can use API
 * set or setRange to set bit by bit position, or use setRangeByLength to set bit by file position. As the class saved
 * block length, it will convert a file position to bit position. But be careful, don't use one BitMap instance between
 * different file at same time.
 *
 * \example ../../unittest/protocols/BitMap_unittest.cpp
 * You can find API example in unit test file.
 */
class BitMap
{
public:
    typedef bool value_type;
    typedef std::vector<value_type> container_type;
    typedef container_type::size_type size_type;

    BitMap();
    BitMap(size_t len, size_t bytesPerBit);
    ~BitMap();

    BitMap(const BitMap &arg);
    const BitMap& operator=(const BitMap &arg);
    void swap(BitMap &arg);

    size_type size();
    size_t bytesPerBit();

    size_type find(bool v, size_type pos = 0);
    value_type get(size_type s);
    void set(size_type s, value_type v);
    void setAll(value_type v);
    void setRange(size_type begin, size_type end, value_type v);

    size_type getPositionByLength(size_t len);
    void setRangeByLength(size_t begin, size_t end, value_type v);

    std::vector<bool> getVector();

private:
    std::vector<bool> map_;
    size_t len_;
    size_t bytesPerBit_;
};

void swap(BitMap &l, BitMap &r);

#endif
