/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*Reference OpenCL Builtins                                                   *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/



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
                         const std::vector<GenericValue> &Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv;

  gv.IntVal = pG->GetValueForWorkGroupAllAnyBuiltin().IntVal;

  pG->DecRef();
  return gv;
}

GenericValue lle_X_work_group_all_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv;

  gv.IntVal = APInt(32, !pG->GetValueForWorkGroupAllAnyBuiltin().IntVal, true);

  pG->DecRef();
  return gv;
}

GenericValue lle_X_work_group_any_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
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
    const std::vector<GenericValue> &Args) {
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
    const std::vector<GenericValue> &Args) {
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
    const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv = pG->GetValueForBroadcastBuiltin();

  pG->DecRef();
  return gv;
}

template<typename T>
GenericValue lle_X_work_group_reduce_add_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv = pG->GetValueForReduceBuiltin();

  pG->DecRef();
  return gv;
}

template<typename T>
GenericValue lle_X_work_group_reduce_min_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv = pG->GetValueForReduceBuiltin();

  pG->DecRef();
  return gv;
}

template<typename T>
GenericValue lle_X_work_group_reduce_max_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
  IWorkGroupBuiltins * pG = WorkItemInterfaceSetter::inst()->GetWorkGroupInterface();
  GenericValue gv = pG->GetValueForReduceBuiltin();

  pG->DecRef();
  return gv;
}

//common function for all prefixsum built-ins
template<typename T>
GenericValue lle_X_work_group_prefixsum_pre_exec_impl(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
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
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_min_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_inclusive_max(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_max_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_inclusive_add(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_add_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_exclusive_min(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_min_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_exclusive_max(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_max_impl<T, isExclusive>(FT, Args);
}
template<typename T, bool isExclusive>
GenericValue lle_X_work_group_prefixsum_exclusive_add(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_add_impl<T, isExclusive>(FT, Args);
}

template<typename T>
GenericValue lle_X_work_group_prefixsum_inclusive_min_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_inclusive_max_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_inclusive_add_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_exclusive_min_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_exclusive_max_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}
template<typename T>
GenericValue lle_X_work_group_prefixsum_exclusive_add_pre_exec(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_prefixsum_pre_exec_impl<T>(FT, Args);
}

GenericValue lle_X_work_group_broadcast_1D(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_broadcast(FT, Args);
}
GenericValue lle_X_work_group_broadcast_2D(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_broadcast(FT, Args);
}
GenericValue lle_X_work_group_broadcast_3D(FunctionType *FT,
                         const std::vector<GenericValue> &Args) {
  return lle_X_work_group_broadcast(FT, Args);
}

} // namespace OCLBuiltins
} // namespace Validation

extern "C" {
BUILTINS_API void initOCLBuiltinsWorkGroup() {return;}
 
  

  BUILTINS_API llvm::GenericValue lle_X__Z14work_group_alli( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_all(FT,Args);}//0
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_all_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_all_pre_exec(FT,Args);}//1
  BUILTINS_API llvm::GenericValue lle_X__Z14work_group_anyi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_any(FT,Args);}//2
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_any_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_any_pre_exec(FT,Args);}//3
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_1Dfm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D(FT,Args);}//4
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_1Ddm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D(FT,Args);}//5
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_1Dim( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D(FT,Args);}//6
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_1Djm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D(FT,Args);}//7
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_1Dlm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D(FT,Args);}//8
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_1Dmm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D(FT,Args);}//9
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_1D_pre_execfm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D_pre_exec(FT,Args);}//10
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_1D_pre_execdm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D_pre_exec(FT,Args);}//11
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_1D_pre_execim( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D_pre_exec(FT,Args);}//12
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_1D_pre_execjm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D_pre_exec(FT,Args);}//13
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_1D_pre_execlm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D_pre_exec(FT,Args);}//14
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_1D_pre_execmm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_1D_pre_exec(FT,Args);}//15
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_2DfPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D(FT,Args);}//16
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_2DdPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D(FT,Args);}//17
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_2DiPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D(FT,Args);}//18
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_2DjPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D(FT,Args);}//19
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_2DlPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D(FT,Args);}//20
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_2DmPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D(FT,Args);}//21
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_2D_pre_execfPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D_pre_exec(FT,Args);}//22
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_2D_pre_execdPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D_pre_exec(FT,Args);}//23
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_2D_pre_execiPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D_pre_exec(FT,Args);}//24
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_2D_pre_execjPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D_pre_exec(FT,Args);}//25
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_2D_pre_execlPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D_pre_exec(FT,Args);}//26
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_2D_pre_execmPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_2D_pre_exec(FT,Args);}//27
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_3DfPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D(FT,Args);}//28
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_3DdPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D(FT,Args);}//29
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_3DiPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D(FT,Args);}//30
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_3DjPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D(FT,Args);}//31
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_3DlPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D(FT,Args);}//32
  BUILTINS_API llvm::GenericValue lle_X__Z23work_group_broadcast_3DmPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D(FT,Args);}//33
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_3D_pre_execfPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D_pre_exec(FT,Args);}//34
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_3D_pre_execdPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D_pre_exec(FT,Args);}//35
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_3D_pre_execiPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D_pre_exec(FT,Args);}//36
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_3D_pre_execjPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D_pre_exec(FT,Args);}//37
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_3D_pre_execlPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D_pre_exec(FT,Args);}//38
  BUILTINS_API llvm::GenericValue lle_X__Z32work_group_broadcast_3D_pre_execmPm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_broadcast_3D_pre_exec(FT,Args);}//39
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_addf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add<float,true>(FT,Args);}//40
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_addd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add<double,true>(FT,Args);}//41
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_addi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add<int32_t,true>(FT,Args);}//42
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_addj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add<uint32_t,true>(FT,Args);}//43
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_addl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add<int64_t,true>(FT,Args);}//44
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_addm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add<uint64_t,true>(FT,Args);}//45
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_add_pre_execf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add_pre_exec<float>(FT,Args);}//46
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_add_pre_execd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add_pre_exec<double>(FT,Args);}//47
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_add_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add_pre_exec<int32_t>(FT,Args);}//48
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_add_pre_execj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add_pre_exec<uint32_t>(FT,Args);}//49
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_add_pre_execl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add_pre_exec<int64_t>(FT,Args);}//50
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_add_pre_execm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_add_pre_exec<uint64_t>(FT,Args);}//51
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_maxf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max<float,true>(FT,Args);}//52
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_maxd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max<double,true>(FT,Args);}//53
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_maxi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max<int32_t,true>(FT,Args);}//54
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_maxj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max<uint32_t,true>(FT,Args);}//55
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_maxl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max<int64_t,true>(FT,Args);}//56
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_maxm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max<uint64_t,true>(FT,Args);}//57
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_max_pre_execf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max_pre_exec<float>(FT,Args);}//58
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_max_pre_execd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max_pre_exec<double>(FT,Args);}//59
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_max_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max_pre_exec<int32_t>(FT,Args);}//60
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_max_pre_execj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max_pre_exec<uint32_t>(FT,Args);}//61
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_max_pre_execl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max_pre_exec<int64_t>(FT,Args);}//62
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_max_pre_execm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_max_pre_exec<uint64_t>(FT,Args);}//63
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_minf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min<float,true>(FT,Args);}//64
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_mind( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min<double,true>(FT,Args);}//65
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_mini( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min<int32_t,true>(FT,Args);}//66
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_minj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min<uint32_t,true>(FT,Args);}//67
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_minl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min<int64_t,true>(FT,Args);}//68
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_exclusive_minm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min<uint64_t,true>(FT,Args);}//69
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_min_pre_execf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min_pre_exec<float>(FT,Args);}//70
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_min_pre_execd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min_pre_exec<double>(FT,Args);}//71
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_min_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min_pre_exec<int32_t>(FT,Args);}//72
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_min_pre_execj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min_pre_exec<uint32_t>(FT,Args);}//73
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_min_pre_execl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min_pre_exec<int64_t>(FT,Args);}//74
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_exclusive_min_pre_execm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_exclusive_min_pre_exec<uint64_t>(FT,Args);}//75
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_addf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add<float,false>(FT,Args);}//76
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_addd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add<double,false>(FT,Args);}//77
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_addi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add<int32_t,false>(FT,Args);}//78
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_addj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add<uint32_t,false>(FT,Args);}//79
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_addl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add<int64_t,false>(FT,Args);}//80
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_addm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add<uint64_t,false>(FT,Args);}//81
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_add_pre_execf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add_pre_exec<float>(FT,Args);}//82
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_add_pre_execd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add_pre_exec<double>(FT,Args);}//83
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_add_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add_pre_exec<int32_t>(FT,Args);}//84
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_add_pre_execj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add_pre_exec<uint32_t>(FT,Args);}//85
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_add_pre_execl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add_pre_exec<int64_t>(FT,Args);}//86
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_add_pre_execm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_add_pre_exec<uint64_t>(FT,Args);}//87
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_maxf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max<float,false>(FT,Args);}//88
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_maxd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max<double,false>(FT,Args);}//89
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_maxi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max<int32_t,false>(FT,Args);}//90
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_maxj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max<uint32_t,false>(FT,Args);}//91
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_maxl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max<int64_t,false>(FT,Args);}//92
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_maxm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max<uint64_t,false>(FT,Args);}//93
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_max_pre_execf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max_pre_exec<float>(FT,Args);}//94
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_max_pre_execd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max_pre_exec<double>(FT,Args);}//95
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_max_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max_pre_exec<int32_t>(FT,Args);}//96
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_max_pre_execj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max_pre_exec<uint32_t>(FT,Args);}//97
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_max_pre_execl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max_pre_exec<int64_t>(FT,Args);}//98
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_max_pre_execm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_max_pre_exec<uint64_t>(FT,Args);}//99
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_minf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min<float,false>(FT,Args);}//100
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_mind( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min<double,false>(FT,Args);}//101
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_mini( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min<int32_t,false>(FT,Args);}//102
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_minj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min<uint32_t,false>(FT,Args);}//103
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_minl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min<int64_t,false>(FT,Args);}//104
  BUILTINS_API llvm::GenericValue lle_X__Z34work_group_prefixsum_inclusive_minm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min<uint64_t,false>(FT,Args);}//105
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_min_pre_execf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min_pre_exec<float>(FT,Args);}//106
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_min_pre_execd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min_pre_exec<double>(FT,Args);}//107
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_min_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min_pre_exec<int32_t>(FT,Args);}//108
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_min_pre_execj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min_pre_exec<uint32_t>(FT,Args);}//109
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_min_pre_execl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min_pre_exec<int64_t>(FT,Args);}//110
  BUILTINS_API llvm::GenericValue lle_X__Z43work_group_prefixsum_inclusive_min_pre_execm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_prefixsum_inclusive_min_pre_exec<uint64_t>(FT,Args);}//111
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_addf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add<float>(FT,Args);}//112
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_addd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add<double>(FT,Args);}//113
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_addi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add<int32_t>(FT,Args);}//114
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_addj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add<uint32_t>(FT,Args);}//115
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_addl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add<int64_t>(FT,Args);}//116
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_addm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add<uint64_t>(FT,Args);}//117
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_add_pre_execf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add_pre_exec<float>(FT,Args);}//118
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_add_pre_execd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add_pre_exec<double>(FT,Args);}//119
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_add_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add_pre_exec<int32_t>(FT,Args);}//120
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_add_pre_execj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add_pre_exec<uint32_t>(FT,Args);}//121
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_add_pre_execl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add_pre_exec<int64_t>(FT,Args);}//122
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_add_pre_execm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_add_pre_exec<uint64_t>(FT,Args);}//123
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_maxf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max<float>(FT,Args);}//124
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_maxd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max<double>(FT,Args);}//125
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_maxi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max<int32_t>(FT,Args);}//126
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_maxj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max<uint32_t>(FT,Args);}//127
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_maxl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max<int64_t>(FT,Args);}//128
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_maxm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max<uint64_t>(FT,Args);}//129
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_max_pre_execf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max_pre_exec<float>(FT,Args);}//130
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_max_pre_execd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max_pre_exec<double>(FT,Args);}//131
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_max_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max_pre_exec<int32_t>(FT,Args);}//132
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_max_pre_execj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max_pre_exec<uint32_t>(FT,Args);}//133
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_max_pre_execl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max_pre_exec<int64_t>(FT,Args);}//134
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_max_pre_execm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_max_pre_exec<uint64_t>(FT,Args);}//135
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_minf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min<float>(FT,Args);}//136
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_mind( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min<double>(FT,Args);}//137
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_mini( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min<int32_t>(FT,Args);}//138
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_minj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min<uint32_t>(FT,Args);}//139
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_minl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min<int64_t>(FT,Args);}//140
  BUILTINS_API llvm::GenericValue lle_X__Z21work_group_reduce_minm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min<uint64_t>(FT,Args);}//141
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_min_pre_execf( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min_pre_exec<float>(FT,Args);}//142
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_min_pre_execd( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min_pre_exec<double>(FT,Args);}//143
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_min_pre_execi( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min_pre_exec<int32_t>(FT,Args);}//144
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_min_pre_execj( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min_pre_exec<uint32_t>(FT,Args);}//145
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_min_pre_execl( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min_pre_exec<int64_t>(FT,Args);}//146
  BUILTINS_API llvm::GenericValue lle_X__Z30work_group_reduce_min_pre_execm( llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args) { return lle_X_work_group_reduce_min_pre_exec<uint64_t>(FT,Args);}//147



}

  
