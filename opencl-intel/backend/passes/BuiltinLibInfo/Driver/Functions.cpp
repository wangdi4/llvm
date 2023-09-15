// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#include "Functions.h"

namespace intel {

VFH::VFH(hashEntry *initializer) {
  size_t index = 0;
  while (initializer[index].funcs[0]) {
    const hashEntry *e = &initializer[index];
    if (e->isPacketizable) {
      m_functions[e->funcs[0]] = funcEntry(e, 1);
    }

    m_functions[e->funcs[1]] = funcEntry(e, 2);
    m_functions[e->funcs[2]] = funcEntry(e, 4);
    m_functions[e->funcs[3]] = funcEntry(e, 8);
    m_functions[e->funcs[4]] = funcEntry(e, 16);
    m_functions[e->funcs[5]] = funcEntry(e, 3);
    index++;
  }
}

funcEntry VFH::findFunctionInHash(std::string &inp_name) const {
  // If this function is known
  if (m_functions.find(inp_name) != m_functions.end()) {
    return m_functions[inp_name];
  } else {
    // unknown functions return null
    return funcEntry();
  }
}

//
// funcEntry
//

funcEntry::funcEntry(const hashEntry *he, unsigned width)
    : m_hashEntry(he), m_width(width) {}

funcEntry::funcEntry() : m_hashEntry(nullptr), m_width(0) {}

unsigned funcEntry::getWidth() const {
  assert(!isNull() && "null entry");
  return m_width;
}

bool funcEntry::isPacketizable() const {
  assert(!isNull() && "null entry");
  if (0 == m_width)
    return false;
  return m_hashEntry[getIndex()].isPacketizable;
}

bool funcEntry::isScalarizable() const {
  assert(!isNull() && "null entry");
  if (0 == m_width)
    return false;
  return m_hashEntry[getIndex()].isScalarizable;
}

std::string funcEntry::getVersion(unsigned v) const {
  assert(v < NUM_VERSIONS && "invalid verison");
  return std::string(m_hashEntry->funcs[v]);
}

bool funcEntry::isNull() const { return (m_hashEntry == nullptr); }

int funcEntry::getIndex() const {
  assert(0 != m_width && "invalid width, cannot get index");
  switch (m_width) {
  case 1:
    return 0;
  case 2:
    return 1;
  case 4:
    return 2;
  case 8:
    return 3;
  case 16:
    return 4;
  case 3:
    return 5;
  default: // this is a bug, it means getIndex was called for a null entry
    return 0;
  }
}

} // namespace intel
