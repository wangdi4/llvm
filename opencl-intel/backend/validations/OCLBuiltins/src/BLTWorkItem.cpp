/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Reference OpenCL Builtins                                                   *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/



/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTWorkItem.cpp

\*****************************************************************************/

#include <vector>
#include <llvm/ExecutionEngine/GenericValue.h>
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
                         const std::vector<GenericValue> &Args) {

  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  uint32_t work_dim = pI->GetWorkDim();
  GenericValue gv;
  // returns uint (32bit)
  gv.IntVal=APInt(32, work_dim);
  return gv;
}

GenericValue lle_X_get_global_size(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {

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
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<llvm::GenericValue> &Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;

  uint32_t lId = pI->GetGlobalIdNoOffset(2)*pI->GetGlobalSize(1)*pI->GetGlobalSize(0) +
      pI->GetGlobalIdNoOffset(1)*pI->GetGlobalSize(0) + pI->GetGlobalIdNoOffset(0);
  gv.IntVal = APInt(SizeTInBits, lId);
  return gv;
}

llvm::GenericValue lle_X_get_local_linear_id(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv;

  uint32_t lId = pI->GetLocalId(2)*pI->GetLocalSize(1)*pI->GetLocalSize(0) +
      pI->GetLocalId(1)*pI->GetLocalSize(0) + pI->GetLocalId(0);
  gv.IntVal = APInt(SizeTInBits, lId);
  return gv;
}

llvm::GenericValue lle_X_get_enqueued_local_size(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args) {
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
BUILTINS_API void initOCLBuiltinsWorkItem() {return;}


  BUILTINS_API llvm::GenericValue lle_X__Z23get_enqueued_local_sizej( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_enqueued_local_size(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z13get_global_idj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_global_id(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z20get_global_linear_idv( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_global_linear_id(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z17get_global_offsetj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_global_offset(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z15get_global_sizej( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_global_size(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z12get_group_idj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_group_id(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z12get_local_idj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_local_id(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z19get_local_linear_idv( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_local_linear_id(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z14get_local_sizej( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_local_size(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z14get_num_groupsj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_num_groups(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z12get_work_dimv( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_get_work_dim(FT,Args);}//10



}

  
