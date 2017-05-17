/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __OCL_FUNCTION_ATTRS_H__
#define __OCL_FUNCTION_ATTRS_H__

#include "llvm/Pass.h"
#include "llvm/IR/InstrTypes.h"

namespace intel {

using namespace llvm;

/// OclFunctionAttrs - This pass intended to take advantage of OpenCL language
/// features and modify function attributes. Currently, the only modification
/// that is made is setting the NoAlias attribute on local memory arguments.
class OclFunctionAttrs : public ModulePass {

public:
  static char ID;

  OclFunctionAttrs();

  virtual llvm::StringRef getPassName() const { return "OclFunctionAttrs"; }
  virtual bool runOnModule(Module &M);
};


/// OclSyncFunctionAttrs - This pass intended to mark synchronize built-ins of
/// OpenCL language and all functions that calls them direct or indirect,
/// in order to prevent LLVM Standard passes from brake their symantic. Currently,
/// the it is only propogate NoDuplicate attribute to all synchronize built-ins.
class OclSyncFunctionAttrs : public ModulePass {

public:
  static char ID;

  OclSyncFunctionAttrs();

  virtual llvm::StringRef getPassName() const { return "OclSyncFunctionAttrs"; }
  virtual bool runOnModule(Module &M);
};

} // namespace intel

#endif // __OCL_FUNCTION_ATTRS_H__
