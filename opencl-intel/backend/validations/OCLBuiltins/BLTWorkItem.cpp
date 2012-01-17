/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

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
#include "IWorkItemBuiltins.h"
#include "BLTWorkItem.h"

using namespace llvm;
using std::map;
using std::string;
using std::vector;
namespace Validation {
namespace OCLBuiltins {

// static member definition of singleton
WorkItemInterfaceSetter * WorkItemInterfaceSetter::m_pInst = NULL;

// define const SizeTInBits for x86 and x64
// it is needed since come workgroup built-ins returns size_t
const uint32_t SizeTInBits = sizeof(size_t) * 8;

GenericValue lle_X_get_work_dim(const FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
 
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  uint32_t work_dim = pI->GetWorkDim();
  GenericValue gv;
  // returns uint (32bit)
  gv.IntVal=APInt(32, work_dim);
  return gv;
}

GenericValue lle_X_get_global_size(const FunctionType *FT,
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

GenericValue lle_X_get_global_id(const FunctionType *FT,
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

GenericValue lle_X_get_local_size(const FunctionType *FT,
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

GenericValue lle_X_get_local_id(const FunctionType *FT,
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

GenericValue lle_X_get_num_groups(const FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface(); 
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim. 
  // For other values of index get_num_groups() returns 1
  const uint64_t intval = (index < work_dim) ? 
      pI->GetGlobalSize(index) / pI->GetLocalSize(index) : 1;
  // returns size_t
  gv.IntVal=APInt( SizeTInBits, intval );
  return gv;
}

GenericValue lle_X_get_group_id(const FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface(); 
  GenericValue gv;
  uint32_t work_dim = pI->GetWorkDim();
  uint32_t index = (uint32_t) Args[0].IntVal.getLimitedValue();
  // Valid values of index are 0 to work_dim. 
  // For other values of index get_groups_id() returns 0
  const uint64_t intval = (index < work_dim) ? 
      pI->GetGlobalIdNoOffset(index) / pI->GetLocalSize(index) : 0;
  // returns size_t
  gv.IntVal = APInt(SizeTInBits, intval );
  return gv;
}

GenericValue lle_X_get_global_offset(const FunctionType *FT,
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


void WorkItemMapFiller::addOpenCLBuiltins( map<string, PBLTFunc>& funcNames )
{
    funcNames["lle_X_get_work_dim"]              =     lle_X_get_work_dim;
    funcNames["lle_X_get_global_size"]           =     lle_X_get_global_size;
    funcNames["lle_X_get_global_id"]             =     lle_X_get_global_id;
    funcNames["lle_X_get_local_size"]            =     lle_X_get_local_size;
    funcNames["lle_X_get_local_id"]              =     lle_X_get_local_id;
    funcNames["lle_X_get_num_groups"]            =     lle_X_get_num_groups;
    funcNames["lle_X_get_group_id"]              =     lle_X_get_group_id;
    funcNames["lle_X_get_global_offset"]         =     lle_X_get_global_offset;
}


} // namespace Validation 
} // namespace OCLBuiltins

