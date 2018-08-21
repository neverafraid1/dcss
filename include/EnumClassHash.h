/*
 * EnumClassHash.h
 *
 *  Created on: 2018年8月21日
 *      Author: wangzhen
 */

#ifndef INCLUDE_ENUMCLASSHASH_H_
#define INCLUDE_ENUMCLASSHASH_H_

#include <cstddef>

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

#endif /* INCLUDE_ENUMCLASSHASH_H_ */
