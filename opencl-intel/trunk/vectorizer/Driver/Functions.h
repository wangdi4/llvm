/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_
#include "llvm/ADT/StringMap.h"
#include "RuntimeServices.h"

#include <string>

namespace intel {

/// @brief
///  Functions mapping for Intel's Opencl builtin functions
/// @Author Sion Berkowits, Nadav Rotem
class VFH {
public:

  typedef RuntimeServices::hashEntry hashEntry;
  typedef RuntimeServices::funcEntry funcEntry;

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
