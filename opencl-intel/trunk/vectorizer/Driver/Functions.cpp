/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/

#include "Functions.h"

namespace intel {

VFH::VFH(hashEntry* initializer) {
  size_t index = 0;
  while (initializer[index].funcs[0]) {
    const hashEntry* e = &initializer[index];
    if (e->isPacketizable) {
      m_functions[e->funcs[0]] = std::make_pair(e,  1);
    }

    m_functions[e->funcs[1]] = std::make_pair(e,  2);
    m_functions[e->funcs[2]] = std::make_pair(e,  4);
    m_functions[e->funcs[3]] = std::make_pair(e,  8);
    m_functions[e->funcs[4]] = std::make_pair(e, 16);
    m_functions[e->funcs[5]] = std::make_pair(e,  3);
    index++;
  }
}

RuntimeServices::funcEntry
VFH::findFunctionInHash(std::string &inp_name) const{
  // If this function is known
  if (m_functions.find(inp_name) != m_functions.end()) {
    return m_functions[inp_name];
  } else {
    // unknown functions return null
    funcEntry e;
    e.first = NULL;
    e.second = 0;
    return e;
  }
}

} // Namespace
