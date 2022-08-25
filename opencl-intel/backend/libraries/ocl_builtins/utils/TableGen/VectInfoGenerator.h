// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
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

#ifndef __VECT_INFO_GENERATOR_H__
#define __VECT_INFO_GENERATOR_H__

#include "OclBuiltinEmitter.h"

#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"

#include <algorithm>
#include <sstream>

namespace llvm {

template <class T> using VecVec = std::vector<std::vector<T>>;
using SVecVec = VecVec<std::string>;
using BVecVec = VecVec<const OclBuiltin *>;
using TVecVec = VecVec<const OclType *>;

struct VectEntry {
  std::vector<std::string> funcNames;
  static std::vector<VFParameter> vectorKindEncode;
  static bool isMasked;
  static bool kernelCallOnce;
  static unsigned stride;
  const static VFISAKind isaClass;
  const static std::string baseName;
};

class VectInfo {
public:
  using type_iterator = TVecVec::const_iterator;

  static OclBuiltinDB *builtinDB;

  VectInfo(Record *record);

  const std::vector<const OclBuiltin *> &getBuiltins() const {
    return m_builtins;
  }

  type_iterator type_begin() const { return m_types.cbegin(); }

  type_iterator type_end() const { return m_types.cend(); }

  size_t getNumOfTypes() const { return m_types.size(); }

  bool handleAlias() const { return m_handleAlias; }

  unsigned stride() const { return m_stride; }

private:
  bool m_handleAlias;

  unsigned m_stride;

  TVecVec m_types;

  std::vector<const OclBuiltin *> m_builtins;
};

class VectInfoGenerator {
  // When taking alias into consideration, OclBuiltin x FuncName x OclTypeName
  // can distinguish a prototype.
  using FuncProto = std::pair<std::pair<const OclBuiltin *, const std::string>,
                              const std::string>;

public:
  explicit VectInfoGenerator(RecordKeeper &);
  void run(raw_ostream &);

private:
  void decodeParam(StringRef scalarFuncName, StringRef v4FuncName);
  void generateFunctions(const std::vector<const OclBuiltin *> &builtins,
                         const std::vector<const OclType *> &types,
                         const SVecVec &funcNames);

  RecordKeeper &m_RecordKeeper;
  OclBuiltinDB m_DB;

  // Temp stream for generated functions.
  std::stringstream m_funcStream;

  // Map from a function to it's index.
  std::map<FuncProto, size_t> m_funcProtoToIndex;

  // The counter of generated functions.
  size_t m_funcCounter;

  // Map from the index of a duplicate function with a broken name to
  // it's real function's index.
  std::map<size_t, size_t> m_dupFuncToOrigFunc;
};

} // namespace llvm

#endif //__VECT_INFO_GENERATOR_H__
