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

#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include "RuntimeServices.h"
#include "llvm/ADT/StringMap.h"
#include <string>

namespace intel {
const unsigned NUM_VERSIONS = 6;
/// @brief Structure returned for each function name query
struct hashEntry {
  const char
      *funcs[NUM_VERSIONS]; // Name of functions, sorted as (1,2,4,8,16,3)
  unsigned isScalarizable;
  unsigned isPacketizable;
};

struct funcEntry : VectorizerFunction {
  funcEntry(const hashEntry *, unsigned width);
  funcEntry();
  unsigned getWidth() const override;
  bool isPacketizable() const override;
  bool isScalarizable() const override;
  std::string getVersion(unsigned) const override;
  bool isNull() const override;

private:
  int getIndex() const;
  const hashEntry *m_hashEntry;
  unsigned m_width;
};

/// @brief
///  Functions mapping for Intel's Opencl builtin functions
class VFH {
public:
  /// @brief C'tor
  /// @param initializer Database to load
  ///  DB is null terminated
  VFH(hashEntry *initializer);

  /// @brief Search the hash for a vector function (used by scalarizer)
  /// @param inp_name Function name to look for
  /// @param vecWidth Returns the vector width of the found function here
  funcEntry findFunctionInHash(std::string &inp_name) const;

private:
  /// function name storage
  mutable StringMap<funcEntry> m_functions;
  std::vector<hashEntry> m_entries;
};

} // namespace intel

#endif // _FUNCTIONS_H_
