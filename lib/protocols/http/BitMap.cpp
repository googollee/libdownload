/**
 * \file BitMap.cpp
 *       BitMap class implementaion.
 */

#include "BitMap.h"

#include <algorithm>

/**
 * \brief The default constructor. Set bit set size and bytes per block to zero.
 */
BitMap::BitMap() :
    map_(0),
    len_(0),
    bytesPerBit_(0)
{}

BitMap::BitMap(size_t len, size_t bytesPerBit)
    : map_( (len + bytesPerBit - 1) / bytesPerBit, false ),
      len_(len),
      bytesPerBit_(bytesPerBit)
{}

BitMap::~BitMap()
{}

BitMap::BitMap(const BitMap &arg)
    : map_(arg.map_),
      len_(arg.len_),
      bytesPerBit_(arg.bytesPerBit_)
{}

const BitMap& BitMap::operator=(const BitMap &arg)
{
    if (this == &arg)
        return *this;

    BitMap bit = arg;
    swap(bit);

    return *this;
}

void BitMap::swap(BitMap &arg)
{
    using std::swap;

    swap(map_, arg.map_);
    swap(len_, arg.len_);
    swap(bytesPerBit_, arg.bytesPerBit_);
}

void swap(BitMap &l, BitMap &r)
{
    l.swap(r);
}

BitMap::size_type BitMap::size()
{
    return map_.size();
}

size_t BitMap::bytesPerBit()
{
    return bytesPerBit_;
}

BitMap::size_type BitMap::find(bool v, BitMap::size_type pos)
{
    for(; pos<map_.size(); ++pos)
    {
        if (map_.at(pos) == v)
            break;
    }

    return pos;
}

BitMap::value_type BitMap::get(BitMap::size_type s)
{
    return map_.at(s);
}

void BitMap::set(BitMap::size_type s, bool v)
{
    map_.at(s) = v;
}

void BitMap::setAll(bool v)
{
    for (size_type i=0; i<map_.size(); ++i)
    {
        map_.at(i) = v;
    }
}

void BitMap::setRange(BitMap::size_type begin, BitMap::size_type end, BitMap::value_type v)
{
    for (size_type i=begin; i<end; ++i)
    {
        map_.at(i) = v;
    }

    if (end > map_.size())
    {
        map_.at(map_.size() - 1) = v;
    }
}

BitMap::size_type BitMap::getPositionByLength(size_t len)
{
    return len / bytesPerBit_;
}

void BitMap::setRangeByLength(size_t begin, size_t end, BitMap::value_type v)
{
    if (end >= len_)
    {
        end += bytesPerBit_;
    }
    setRange(begin / bytesPerBit_, end / bytesPerBit_, v);
}

std::vector<bool> BitMap::getVector()
{
    return map_;
}
