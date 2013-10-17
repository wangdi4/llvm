/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include "RuntimeServices.h"
#include "llvm/ADT/StringMap.h"
#include <string>

namespace intel {
  const unsigned NUM_VERSIONS = 6;
  /// @brief Structure returned for each function name query
  struct hashEntry {
    const char* funcs[NUM_VERSIONS]; // Name of functions, sorted as (1,2,4,8,16,3)
    unsigned isScalarizable;
    unsigned isPacketizable;
  };

struct funcEntry : VectorizerFunction{
  funcEntry(const hashEntry*, unsigned width);
  funcEntry();
  unsigned getWidth()const;
  bool isPacketizable()const;
  bool isScalarizable()const;
  std::string getVersion(unsigned)const;
  bool isNull()const;
private:
  int getIndex()const;
  const hashEntry* m_hashEntry;
  unsigned m_width;
};

/// @brief
///  Functions mapping for Intel's Opencl builtin functions
class VFH {
public:


  /// @brief C'tor
  /// @param initializer Database to load
  ///  DB is null terminated
  VFH(hashEntry* initializer);

  /// @brief Search the hash for a vector function (used by scalarizer)
  /// @param inp_name Function name to look for
  /// @param vecWidth Returns the vector width of the found function here
  funcEntry findFunctionInHash(std::string &inp_name) const;

private:
  /// function name storage
  mutable StringMap<funcEntry> m_functions;
  std::vector<hashEntry> m_entries;
};

} // Namespace

#endif // _FUNCTIONS_H_
