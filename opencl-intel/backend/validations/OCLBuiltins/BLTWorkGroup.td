// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 2.0 rev 06, Table 6.13.15
let Types = [v1f32, v1f64, v1i32, v1u32, v1i64, v1u64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def work_group_broadcast_1d   : REF_OclBuiltin<"work_group_broadcast_1D",   (outs gentype:$ret), (ins gentype:$x, size_t:$y), [], [], TemplateAttr< [] > >;
def work_group_broadcast_2d        : REF_OclBuiltin<"work_group_broadcast_2D",        (outs gentype:$ret), (ins gentype:$x, psize_t:$y), [], [], TemplateAttr< [] > >;
def work_group_broadcast_3d        : REF_OclBuiltin<"work_group_broadcast_3D",        (outs gentype:$ret), (ins gentype:$x, psize_t:$y), [], [], TemplateAttr< [] > >;
def work_group_broadcast_1d_pre_exec   : REF_OclBuiltin<"work_group_broadcast_1D_pre_exec",   (outs void:$ret), (ins gentype:$x, size_t:$y), [], [], TemplateAttr< [] > >;
def work_group_broadcast_2d_pre_exec   : REF_OclBuiltin<"work_group_broadcast_2D_pre_exec",   (outs void:$ret), (ins gentype:$x, psize_t:$y), [], [], TemplateAttr< [] > >;
def work_group_broadcast_3d_pre_exec   : REF_OclBuiltin<"work_group_broadcast_3D_pre_exec",   (outs void:$ret), (ins gentype:$x, psize_t:$y), [], [], TemplateAttr< [] > >;
def work_group_reduce_min           : REF_OclBuiltin<"work_group_reduce_min",           (outs gentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_reduce_max           : REF_OclBuiltin<"work_group_reduce_max",           (outs gentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_reduce_add        : REF_OclBuiltin<"work_group_reduce_add",        (outs gentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_reduce_min_pre_exec           : REF_OclBuiltin<"work_group_reduce_min_pre_exec",           (outs void:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_reduce_max_pre_exec           : REF_OclBuiltin<"work_group_reduce_max_pre_exec",           (outs void:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_reduce_add_pre_exec        : REF_OclBuiltin<"work_group_reduce_add_pre_exec",        (outs void:$ret), (ins gentype:$x), [], [], TypeTemplate>;

def work_group_prefixsum_exclusive_min       : REF_OclBuiltin<"work_group_prefixsum_exclusive_min",       (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [type0, arg_true] > >;
def work_group_prefixsum_exclusive_max     : REF_OclBuiltin<"work_group_prefixsum_exclusive_max",     (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [type0, arg_true] > >;
def work_group_prefixsum_exclusive_add         : REF_OclBuiltin<"work_group_prefixsum_exclusive_add",         (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [type0, arg_true] > >;
def work_group_prefixsum_inclusive_min       : REF_OclBuiltin<"work_group_prefixsum_inclusive_min",       (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [type0, arg_false] > >;
def work_group_prefixsum_inclusive_max     : REF_OclBuiltin<"work_group_prefixsum_inclusive_max",     (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [type0, arg_false] > >;
def work_group_prefixsum_inclusive_add         : REF_OclBuiltin<"work_group_prefixsum_inclusive_add",         (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [type0, arg_false] > >;
def work_group_prefixsum_exclusive_min_pre_exec       : REF_OclBuiltin<"work_group_prefixsum_exclusive_min_pre_exec",       (outs void:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_prefixsum_exclusive_max_pre_exec     : REF_OclBuiltin<"work_group_prefixsum_exclusive_max_pre_exec",     (outs void:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_prefixsum_exclusive_add_pre_exec         : REF_OclBuiltin<"work_group_prefixsum_exclusive_add_pre_exec",         (outs void:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_prefixsum_inclusive_min_pre_exec       : REF_OclBuiltin<"work_group_prefixsum_inclusive_min_pre_exec",       (outs void:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_prefixsum_inclusive_max_pre_exec     : REF_OclBuiltin<"work_group_prefixsum_inclusive_max_pre_exec",     (outs void:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def work_group_prefixsum_inclusive_add_pre_exec         : REF_OclBuiltin<"work_group_prefixsum_inclusive_add_pre_exec",         (outs void:$ret), (ins gentype:$x), [], [], TypeTemplate>;
}

let Types = [v1i32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def work_group_all            : REF_OclBuiltin<"work_group_all",            (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def work_group_all_pre_exec   : REF_OclBuiltin<"work_group_all_pre_exec",   (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def work_group_any            : REF_OclBuiltin<"work_group_any",            (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
def work_group_any_pre_exec   : REF_OclBuiltin<"work_group_any_pre_exec",   (outs gentype:$ret), (ins gentype:$x), [], [], TemplateAttr< [] > >;
}

class Generic {
  code Prolog = [{

/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTWorkGroup.cpp

\*****************************************************************************/

#include <vector>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "BLTWorkGroup.h"
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

GenericValue lle_X_work_group_all(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv;

  gv.IntVal = pG->GetValueForWorkGroupAllAnyBuiltin().IntVal;

  pG->DecRef();
  return gv;
}

GenericValue lle_X_work_group_all_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue initializer;
  initializer.IntVal = APInt(32, 1, true);
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue arg = Args[0];
  pG->AddRef("lle_X_work_group_all_pre_exec_impl", initializer);

  if(arg.IntVal==0)
      pG->GetValueForWorkGroupAllAnyBuiltin().IntVal=APInt(32, 0, true);

  return GenericValue();
}

GenericValue lle_X_work_group_any(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv;

  gv.IntVal = APInt(32, !pG->GetValueForWorkGroupAllAnyBuiltin().IntVal, true);

  pG->DecRef();
  return gv;
}

GenericValue lle_X_work_group_any_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue initializer;
  initializer.IntVal = APInt(32, 1, true);
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue arg = Args[0];
  pG->AddRef("lle_X_work_group_any_pre_exec_impl", initializer);

  if(arg.IntVal!=0)
      pG->GetValueForWorkGroupAllAnyBuiltin().IntVal=APInt(32, 0, true);

  return GenericValue();
}

GenericValue lle_X_work_group_broadcast_1D_pre_exec(FunctionType *FT,
    llvm::ArrayRef<llvm::GenericValue> Args) {
        IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
        IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
        llvm::GenericValue initializer;
        GenericValue bValue = Args[0];
        GenericValue localId = Args[1];
        pG->AddRef("lle_X_work_group_broadcast_impl", initializer);

        assert(pI->GetWorkDim() == 1 && "Dimentions quantity specified by brodcast build-in does not equal to real\
                          dimentions quantity");

        assert( (localId.IntVal.getZExtValue() < pI->GetLocalSize(0)) && "incorrect local ID");
        if(localId.IntVal == pI->GetLocalId(0))
            pG->GetValueForBroadcastBuiltin() = bValue;

        return GenericValue();
}

GenericValue lle_X_work_group_broadcast_2D_pre_exec(FunctionType *FT,
    llvm::ArrayRef<llvm::GenericValue> Args) {
        IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
        IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
        llvm::GenericValue initializer;
        GenericValue bValue = Args[0];
        GenericValue localId = Args[1];
        pG->AddRef("lle_X_work_group_broadcast_impl", initializer);

        assert(pI->GetWorkDim() == 2 && "Dimentions quantity specified by brodcast build-in does not equal to real\
                          dimentions quantity");

        assert( ( *((size_t*)localId.PointerVal) < pI->GetLocalSize(0) &&
            *((size_t*)localId.PointerVal + 1) < pI->GetLocalSize(1) ) && "incorrect local ID");

        if((*((size_t*)localId.PointerVal) == pI->GetLocalId(0))&&
            (*((size_t*)localId.PointerVal + 1) == pI->GetLocalId(1)))
            pG->GetValueForBroadcastBuiltin() = bValue;

        return GenericValue();
}

GenericValue lle_X_work_group_broadcast_3D_pre_exec(FunctionType *FT,
    llvm::ArrayRef<llvm::GenericValue> Args) {
        IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
        IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
        llvm::GenericValue initializer;
        GenericValue bValue = Args[0];
        GenericValue localId = Args[1];
        pG->AddRef("lle_X_work_group_broadcas_3D_impl", initializer);

        assert(pI->GetWorkDim() == 3 && "Dimentions quantity specified by brodcast build-in does not equal to real\
                          dimentions quantity");

        assert( ( *((size_t*)localId.PointerVal) < pI->GetLocalSize(0) &&
            *((size_t*)localId.PointerVal + 1) < pI->GetLocalSize(1) &&
            *((size_t*)localId.PointerVal + 2) < pI->GetLocalSize(2) ) && "incorrect local ID");

        if((*((size_t*)localId.PointerVal) == pI->GetLocalId(0))&&
            (*((size_t*)localId.PointerVal + 1) == pI->GetLocalId(1))&&
            (*((size_t*)localId.PointerVal + 2) == pI->GetLocalId(2)))
            pG->GetValueForBroadcastBuiltin() = bValue;

        return GenericValue();
}

GenericValue lle_X_work_group_broadcast(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv = pG->GetValueForBroadcastBuiltin();

  pG->DecRef();
  return gv;
}

template<typename T>
GenericValue lle_X_work_group_reduce_add_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue initializer;
  initializer = initWithZero<T>();
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue Value = Args[0];
  pG->AddRef("lle_X_work_group_reduce_add_pre_exec_impl", initializer);

  getRef<T>(pG->GetValueForReduceBuiltin())=getRef<T>(pG->GetValueForReduceBuiltin())+getVal<T>(Value);

  return GenericValue();
}

template<typename T>
GenericValue lle_X_work_group_reduce_add(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv = pG->GetValueForReduceBuiltin();

  pG->DecRef();
  return gv;
}

template<typename T>
GenericValue lle_X_work_group_reduce_min_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue initializer;
  initializer = initWithMax<T>();
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  pG->AddRef("lle_X_work_group_reduce_min_pre_exec_impl", initializer);
  llvm::GenericValue arg0 = Args[0];
  typename retType<T>::type Value = getRef<T>(arg0);
  typename retType<T>::type Min = getRef<T>(pG->GetValueForReduceBuiltin());

  getRef<T>(pG->GetValueForReduceBuiltin())=predLess<T>(Value,Min)?Value:Min;

  return GenericValue();
}

template<typename T>
GenericValue lle_X_work_group_reduce_min(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv = pG->GetValueForReduceBuiltin();

  pG->DecRef();
  return gv;
}

template<typename T>
GenericValue lle_X_work_group_reduce_max_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue initializer;
  initializer = initWithMin<T>();
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  pG->AddRef("lle_X_work_group_reduce_max_pre_exec_impl", initializer);
  llvm::GenericValue arg0 = Args[0];
  typename retType<T>::type Value = getRef<T>(arg0);
  typename retType<T>::type Max = getRef<T>(pG->GetValueForReduceBuiltin());

  getRef<T>(pG->GetValueForReduceBuiltin())=predLess<T>(Value,Max)?Max:Value;

  return GenericValue();
}

template<typename T>
GenericValue lle_X_work_group_reduce_max(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv = pG->GetValueForReduceBuiltin();

  pG->DecRef();
  return gv;
}

//common function for all prefixsum built-ins
template<typename T>
GenericValue lle_X_work_group_prefixsum_pre_exec_impl(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  //collect data in a special way
  //execute operation in post method
  uint64_t idx0, idx1, idx2;//indexes if current WI
  uint64_t size0, size1, size2;//global work sizes

  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  pG->AddRef("lle_X_work_group_prefixsum_inclusive_pre_exec_impl", llvm::GenericValue());
  llvm::GenericValue arg0 = Args[0];
  typename retType<T>::type Value = getRef<T>(arg0);

  //NOTE: Take local sizes instead of global
  //it's because in our model globalid = tid_x + lid_x
  //tid_x - start of wg
  //lid_x - local id
  idx0 = pI->GetLocalId(0);
  idx1 = pI->GetLocalId(1);
  idx2 = pI->GetLocalId(2);

  size0 = pI->GetLocalSize(0);
  size1 = pI->GetLocalSize(1);
  size2 = pI->GetLocalSize(2);

  //create linear order
  uint64_t idx = idx2*size1*size0 + idx1*size0 + idx0;

  //store in 1D array
  pG->GetValueForPrefixSumBuiltin().AggregateVal.resize(size0*size1*size2);
  getRef<T, NULL>(pG->GetValueForPrefixSumBuiltin(), idx) = Value;

  return GenericValue();
}

template<typename T, bool isExlusive>
GenericValue lle_X_work_group_prefixsum_max_impl(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv = initWithMin<T>();

  uint64_t idx0, idx1, idx2;//indexes if current WI
  uint64_t size0, size1, size2;//global work sizes

  //NOTE: Take local sizes instead of global
  //it's because in our model globalid = tid_x + lid_x
  //tid_x - start of wg
  //lid_x - local id
  idx0 = pI->GetLocalId(0);
  idx1 = pI->GetLocalId(1);
  idx2 = pI->GetLocalId(2);

  size0 = pI->GetLocalSize(0);
  size1 = pI->GetLocalSize(1);
  size2 = pI->GetLocalSize(2);

  //create linear order
  uint64_t idx = idx2*size1*size0 + idx1*size0 + idx0;
  typename retType<T>::type Value;
  typename retType<T>::type Max;

  for(uint32_t i = 0; (isExlusive?i<idx:i<=idx); ++i)
  {
      Value = getRef<T, NULL>(pG->GetValueForPrefixSumBuiltin(), i);
      Max = getRef<T>(gv);
      getRef<T>(gv) =  predLess<T>(Value, Max)?Max:Value;
  }
  pG->DecRef();

  return gv;
}

template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_add_impl(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv = initWithZero<T>();

  uint64_t idx0, idx1, idx2;//indexes if current WI
  uint64_t size0, size1, size2;//global work sizes

  //NOTE: Take local sizes instead of global
  //it's because in our model globalid = tid_x + lid_x
  //tid_x - start of wg
  //lid_x - local id
  idx0 = pI->GetLocalId(0);
  idx1 = pI->GetLocalId(1);
  idx2 = pI->GetLocalId(2);

  size0 = pI->GetLocalSize(0);
  size1 = pI->GetLocalSize(1);
  size2 = pI->GetLocalSize(2);

  //create linear order
  uint64_t idx = idx2*size1*size0 + idx1*size0 + idx0;
  typename retType<T>::type Value;
  typename retType<T>::type Add;

  for(uint32_t i = 0; (isExclusive?i<idx:i<=idx); ++i)
  {
      Value = getRef<T, NULL>(pG->GetValueForPrefixSumBuiltin(), i);
      Add = getRef<T>(gv);
      getRef<T>(gv) =  Add+Value;
  }

  pG->DecRef();
  return gv;
}

template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_min_impl(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  IWorkItemBuiltins * pI = WorkItemInterfaceSetter::inst()->GetWorkItemInterface();
  GenericValue gv = initWithMax<T>();

  uint64_t idx0, idx1, idx2;//indexes if current WI
  uint64_t size0, size1, size2;//global work sizes

  //NOTE: Take local sizes instead of global
  //it's because in our model globalid = tid_x + lid_x
  //tid_x - start of wg
  //lid_x - local id
  idx0 = pI->GetLocalId(0);
  idx1 = pI->GetLocalId(1);
  idx2 = pI->GetLocalId(2);

  size0 = pI->GetLocalSize(0);
  size1 = pI->GetLocalSize(1);
  size2 = pI->GetLocalSize(2);

  //create linear order
  uint64_t idx = idx2*size1*size0 + idx1*size0 + idx0;
  typename retType<T>::type Value;
  typename retType<T>::type Min;

  for(uint32_t i = 0; (isExclusive?i<idx:i<=idx); ++i)
  {
      Value = getRef<T, NULL>(pG->GetValueForPrefixSumBuiltin(), i);
      Min = getRef<T>(gv);
      getRef<T>(gv) =  predLess<T>(Value, Min)?Value:Min;
  }

  pG->DecRef();
  return gv;
}

template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_inclusive_min(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_min_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_inclusive_max(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_max_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_inclusive_add(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_add_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_exclusive_min(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_min_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_exclusive_max(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_max_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_exclusive_add(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_add_impl<T, isExclusive>(FT, Args);
}

template<typename T>
GenericValue lle_X_work_group_prefixsum_inclusive_min_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_inclusive_max_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_inclusive_add_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_exclusive_min_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_exclusive_max_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_exclusive_add_pre_exec(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}

GenericValue lle_X_work_group_broadcast_1D(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_broadcast(FT, Args);
}
GenericValue lle_X_work_group_broadcast_2D(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_broadcast(FT, Args);
}
GenericValue lle_X_work_group_broadcast_3D(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  return lle_X_work_group_broadcast(FT, Args);
}

} // namespace OCLBuiltins
} // namespace Validation

extern "C" {
BUILTINS_API void initOCLBuiltinsWorkGroup() {return;}

  }];

  code Epilog = [{

}

  }];
}
  def inc : Generic;
