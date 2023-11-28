// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 2.0 rev 06, Table 6.13.1

let Types = [v1u32],
    Attrs = [OVERLOADABLE] in {
def get_work_dim              : REF_OclBuiltin<"get_work_dim",            (outs gentype:$ret), (ins), [], [], TemplateAttr< [] > >;
def get_global_size           : REF_OclBuiltin<"get_global_size",         (outs size_t:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def get_global_id             : REF_OclBuiltin<"get_global_id",           (outs size_t:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def get_local_size            : REF_OclBuiltin<"get_local_size",          (outs size_t:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def get_enqueued_local_size   : REF_OclBuiltin<"get_enqueued_local_size", (outs size_t:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def get_local_id              : REF_OclBuiltin<"get_local_id",            (outs size_t:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def get_num_groups            : REF_OclBuiltin<"get_num_groups",          (outs size_t:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def get_group_id              : REF_OclBuiltin<"get_group_id",            (outs size_t:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def get_global_offset         : REF_OclBuiltin<"get_global_offset",       (outs size_t:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def get_global_linear_id      : REF_OclBuiltin<"get_global_linear_id",    (outs size_t:$ret), (ins), [], [], TemplateAttr< [] > >;
def get_local_linear_id       : REF_OclBuiltin<"get_local_linear_id",     (outs size_t:$ret), (ins), [], [], TemplateAttr< [] > >;
}

class Generic {
  code Prolog = [{

//
// Copyright (C) 2011 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//

#include <vector>
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/DynamicLibrary.h"
#include "BLTWorkItem.h"
#include "IWorkItemBuiltins.h"

using namespace llvm;
using std::string;
using std::vector;
using namespace Validation::OCLBuiltins;

#ifndef BUILTINS_API
   #if defined(_WIN32)
      #define BUILTINS_API __declspec(dllexport)
   #else
      #define BUILTINS_API
   #endif
#endif


namespace Validation {
namespace OCLBuiltins {

// static member definition of singleton
WorkItemInterfaceSetter * WorkItemInterfaceSetter::m_pInst = NULL;

// define const SizeTInBits for x86 and x64
// it is needed since come workgroup built-ins returns size_t
const uint32_t SizeTInBits = sizeof(size_t) * 8;

GenericValue lle_X_get_work_dim(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {

  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  uint32_t work_dim = pI->GetWorkDim();
  GenericValue gv;
  // returns uint (32bit)
  gv.IntVal=APInt(32, work_dim);
  return gv;
}

GenericValue lle_X_get_global_size(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim.
  // For other values of index get_global_size() returns 1
  const uint64_t intval = (index < work_dim) ? pI->GetGlobalSize(index) : 1;
  // returns size_t
  gv.IntVal = APInt(SizeTInBits, intval);
  return gv;
}

GenericValue lle_X_get_global_id(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {

  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim.
  // For other values of index get_global_id() returns 0
  const uint64_t intval = (index < work_dim) ?
      pI->GetGlobalIdNoOffset(index) + pI->GetGlobalOffset(index) : 0;
  // returns size_t
  gv.IntVal = APInt(SizeTInBits, intval );
  return gv;
}

GenericValue lle_X_get_local_size(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim.
  // For other values of index get_local_size() returns 1
  const uint64_t intval = (index < work_dim) ?
      pI->GetLocalSize(index) : 1;
  // returns size_t
  gv.IntVal = APInt( SizeTInBits, intval );
  return gv;
}

GenericValue lle_X_get_local_id(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim.
  // For other values of index get_local_id() returns 0
  const uint64_t intval = (index < work_dim) ?
      pI->GetLocalId(index) : 0;
  // returns size_t
  gv.IntVal = APInt(SizeTInBits, intval );
  return gv;
}

GenericValue lle_X_get_num_groups(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim.
  // For other values of index get_num_groups() returns 1
  const uint64_t intval = (index < work_dim) ?
      pI->GetGlobalSize(index) / pI->getEnqueuedLocalSize(index) : 1;
  // returns size_t
  gv.IntVal=APInt( SizeTInBits, intval );
  return gv;
}

GenericValue lle_X_get_group_id(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim.
  // For other values of index get_groups_id() returns 0
  const uint64_t intval = (index < work_dim) ?
      pI->GetGlobalIdNoOffset(index) / pI->getEnqueuedLocalSize(index) : 0;
  // returns size_t
  gv.IntVal = APInt(SizeTInBits, intval );
  return gv;
}

GenericValue lle_X_get_global_offset(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim.
  // For other values of index get_global_offset() returns 0
  const uint64_t intval = (index < work_dim) ?
      pI->GetGlobalOffset(index) : 0;
  // returns size_t
  gv.IntVal = APInt(SizeTInBits, intval );
  return gv;
}

llvm::GenericValue lle_X_get_global_linear_id(llvm::FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;

  uint32_t lId = pI->GetGlobalIdNoOffset(2)*pI->GetGlobalSize(1)*pI->GetGlobalSize(0) +
      pI->GetGlobalIdNoOffset(1)*pI->GetGlobalSize(0) + pI->GetGlobalIdNoOffset(0);
  gv.IntVal = APInt(SizeTInBits, lId);
  return gv;
}

llvm::GenericValue lle_X_get_local_linear_id(llvm::FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;

  uint32_t lId = pI->GetLocalId(2)*pI->GetLocalSize(1)*pI->GetLocalSize(0) +
      pI->GetLocalId(1)*pI->GetLocalSize(0) + pI->GetLocalId(0);
  gv.IntVal = APInt(SizeTInBits, lId);
  return gv;
}

llvm::GenericValue lle_X_get_enqueued_local_size(llvm::FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim.
  // For other values of index get_local_size() returns 1
  const uint64_t intval = (index < work_dim) ?
      pI->getEnqueuedLocalSize(index) : 1;
  // returns size_t
  gv.IntVal = APInt( SizeTInBits, intval );
  return gv;
}

} // namespace OCLBuiltins
} // namespace Validation

extern "C" {
  }];

  code EpilogInit = [{
    BUILTINS_API void initOCLBuiltinsWorkItem() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;
