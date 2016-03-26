/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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
    RTS_C,
    RTS_OCL,
    RTS_NUM
  } RuntimeServicesTypes;

  /// @brief Constructor
  /// @param Builtins Built-in module
  /// @param Builtins Built-in module
  BuiltinLibInfo(Module *Builtins, RuntimeServicesTypes type);

  /// @brief Empty Constructor
  ///   Should not be called, exists only to be able to register it to opt.
  BuiltinLibInfo() : ImmutablePass(ID), m_pBIModule(NULL), m_pRuntimeServices(NULL) {
    assert(false && "Default constructor is not supported");
  }

  ~BuiltinLibInfo() {
    delete m_pRuntimeServices;
  }

  virtual const char *getPassName() const { return "BuiltinLibInfo"; }

  /// @brief returns built-ins module
  /// @return the builtin library module
  const Module* getBuiltinModule() const { return m_pBIModule; }
  Module* getBuiltinModule() { return m_pBIModule; }

  /// @brief returns runtime services
  /// @return the runtime services
  const RuntimeServices* getRuntimeServices() const { return m_pRuntimeServices; }
  RuntimeServices* getRuntimeServices() { return m_pRuntimeServices; }

private:
  /// This member holds Built-in Module (is not owned by this pass)
  Module *m_pBIModule;
  /// This member holds Runtime Services instance.
  /// (is owned by this pass and should be deleted at destructor)
  RuntimeServices *m_pRuntimeServices;
};

} // namespace intel

#endif // __BUILTIN_LIB_INFO_H__
