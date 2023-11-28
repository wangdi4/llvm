// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __REFLECTION_MOD_UTILS_H__
#define __REFLECTION_MOD_UTILS_H__

#include <utility>

namespace Reflection {
///////////////////////////////////////////////////////////////////////////////
// Purpose: preforms a cartesian product between two given collections
// Template Parameters:
//   Container: the type of the container. (should receive one template
//   parameter, and have a standard iterator interface.
//   T: the element type of the first container.
//   U: the element type of the second container.
///////////////////////////////////////////////////////////////////////////////
template <template <typename> class Container, typename T, typename U>
class Cartesian {
  typename Container<T>::const_iterator lit, le;
  typename Container<U>::const_iterator rit, re, rb;

public:
  Cartesian(const Container<T> &left, const Container<U> &right) {
    assert(!left.empty() && "left container is empty");
    assert(!right.empty() && "right container is empty");
    lit = left.begin();
    rb = rit = right.begin();
    le = left.end();
    re = right.end();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: advances the cartesian product to the next pair.
  // Return: true if there are more pairs in the product, false otherwise
  /////////////////////////////////////////////////////////////////////////////
  bool next() {
    assert(lit != le && "iteration is finished");
    assert(rit != re && "internal bug, I suck");
    if (++rit == re) {
      rit = rb;
      ++lit;
    }
    return lit != le;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: gets the current pair
  /////////////////////////////////////////////////////////////////////////////
  std::pair<T, U> get() const { return std::make_pair(*lit, *rit); }
};

//
// TableEntry
//
struct TableRow {
  const char *names[6];
  bool isScalarizable;
  bool isPacketizable;
};

} // namespace Reflection
#endif //__REFLECTION_MOD_UTILS_H__
