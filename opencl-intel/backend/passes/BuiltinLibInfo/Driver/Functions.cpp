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
      m_functions[e->funcs[0]] = funcEntry(e, 1);
    }

    m_functions[e->funcs[1]] = funcEntry(e,  2);
    m_functions[e->funcs[2]] = funcEntry(e,  4);
    m_functions[e->funcs[3]] = funcEntry(e,  8);
    m_functions[e->funcs[4]] = funcEntry(e, 16);
    m_functions[e->funcs[5]] = funcEntry(e,  3);
    index++;
  }
}

funcEntry VFH::findFunctionInHash(std::string &inp_name) const{
  // If this function is known
  if (m_functions.find(inp_name) != m_functions.end()) {
    return m_functions[inp_name];
  } else {
    // unknown functions return null
    return funcEntry();
  }
}

//
//funcEntry
//

funcEntry::funcEntry(const hashEntry* he, unsigned width):
m_hashEntry(he), m_width(width){
}

funcEntry::funcEntry():
m_hashEntry(NULL), m_width(0){
}

unsigned funcEntry::getWidth()const{
  assert(!isNull() && "null entry");
  return m_width;
}

bool funcEntry::isPacketizable()const{
  assert(!isNull() && "null entry");
  if (0 == m_width)
    return false;
  return m_hashEntry[getIndex()].isPacketizable;
}

bool funcEntry::isScalarizable()const{
  assert(!isNull() && "null entry");
  if (0 == m_width)
    return false;
  return m_hashEntry[getIndex()].isScalarizable;
}

std::string funcEntry::getVersion(unsigned v)const{
  assert (v<NUM_VERSIONS && "invalid verison");
  return std::string(m_hashEntry->funcs[v]);
}

bool funcEntry::isNull()const{
  return (m_hashEntry == NULL);
}

int funcEntry::getIndex()const{
  assert(0 != m_width && "invalid width, cannot get index");
  switch(m_width){
    case 1:  return 0;
    case 2:  return 1;
    case 4:  return 2;
    case 8:  return 3;
    case 16: return 4;
    case 3:  return 5;
    default: //this is a bug, it means getIndex was called for a null entry
    return 0;
  }
}

} // Namespace
