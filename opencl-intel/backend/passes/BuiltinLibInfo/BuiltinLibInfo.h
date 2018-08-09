// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __BUILTIN_LIB_INFO_H__
#define __BUILTIN_LIB_INFO_H__

#include "RuntimeServices.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace intel {

/// BuiltinLibInfo - This pass is used for holding the Runtime library (built-ins).
class BuiltinLibInfo : public ImmutablePass {
public:
  static char ID;

  typedef enum {
    RTS_OCL,
    RTS_OSX,
    RTS_DX,
    RTS_RS,
    RTS_NUM
  } RuntimeServicesTypes;

  /// @brief Constructor
  /// @param BuiltinsList List of builtin modules
  /// @param type Runtime service type
  BuiltinLibInfo(SmallVector<Module*, 2> builtinsList, RuntimeServicesTypes type);

  /// @brief Empty Constructor
  BuiltinLibInfo() : ImmutablePass(ID), m_pRuntimeServices(nullptr) {
  }

  ~BuiltinLibInfo() {
    delete m_pRuntimeServices;
  }

  virtual llvm::StringRef getPassName() const { return "BuiltinLibInfo"; }

  /// @brief returns built-ins module
  /// @return the builtin library module
  SmallVector<Module*, 2> getBuiltinModules() const { return m_BIModuleList; }

  /// @brief returns runtime services
  /// @return the runtime services
  const RuntimeServices* getRuntimeServices() const { return m_pRuntimeServices; }
  RuntimeServices* getRuntimeServices() { return m_pRuntimeServices; }

private:
  /// This list holds the Builtin modules
  /// (pointers are not owned by this pass)
  SmallVector<Module*, 2> m_BIModuleList;

  /// This member holds Runtime Services instance.
  /// (is owned by this pass and should be deleted at destructor)
  RuntimeServices *m_pRuntimeServices;
};

} // namespace intel

#endif // __BUILTIN_LIB_INFO_H__
