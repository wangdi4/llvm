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
