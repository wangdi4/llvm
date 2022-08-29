//===-------DTransLibraryInfo.cpp - Library function information-----------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides DTrans type information about library and intrinsic
// functions to support using them without having DTrans metadata on them. This
// is used because these functions have standardized type interfaces, but may
// be inserted by arbitrary passes which won't need to know about DTrans
// metadata.

#include "Intel_DTrans/Analysis/DTransLibraryInfo.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "llvm/IR/Intrinsics.h"

namespace llvm {
namespace dtransOP {

DTransLibraryInfo::DTransLibraryInfo(
    DTransTypeManager &TM,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI)
    : TM(TM), GetTLI(GetTLI) {}

void DTransLibraryInfo::initialize(Module &M) {
  LLVMContext &Ctx = TM.getContext();
  llvm::Type *LLVMI1Type = llvm::Type::getInt1Ty(Ctx);
  llvm::Type *LLVMI8Type = llvm::Type::getInt8Ty(Ctx);
  llvm::Type *LLVMI32Type = llvm::Type::getInt32Ty(Ctx);
  llvm::Type *LLVMI64Type = llvm::Type::getInt64Ty(Ctx);
  llvm::Type *LLVMVoidType = llvm::Type::getVoidTy(Ctx);
  llvm::Type *LLVMMDType = llvm::Type::getMetadataTy(Ctx);

  DTransI1Type = TM.getOrCreateAtomicType(LLVMI1Type);
  DTransI8Type = TM.getOrCreateAtomicType(LLVMI8Type);
  DTransI32Type = TM.getOrCreateAtomicType(LLVMI32Type);
  DTransI64Type = TM.getOrCreateAtomicType(LLVMI64Type);
  DTransVoidType = TM.getOrCreateAtomicType(LLVMVoidType);
  DTransMDType = TM.getOrCreateAtomicType(LLVMMDType);

  // TODO: For 32-bit compilations we need to set this to DTransI32Type.
  DTransSizeType = DTransI64Type;

  DTransI8PtrType = TM.getOrCreatePointerType(DTransI8Type);
  DTransI32PtrType = TM.getOrCreatePointerType(DTransI32Type);
  DTransI64PtrType = TM.getOrCreatePointerType(DTransI64Type);

  DTransIOPtrType = findIOPtrType(M);
  DTransStructType *DTransIdentType = findIdentTStructType(M);
  DTransIdentTPtrType =
      DTransIdentType ? TM.getOrCreatePointerType(DTransIdentType) : nullptr;

  DTransKMPCriticalNameType =
      TM.getOrCreateArrayType(DTransI32Type, KMPCritcalNameArrayLen);
  DTransKMPCriticalNamePtrType =
      TM.getOrCreatePointerType(DTransKMPCriticalNameType);
}

// Try to find the structure type inserted by PAROPT, %struct.ident_t"
DTransStructType *DTransLibraryInfo::findIdentTStructType(Module &M) {
  // Look for a ".kmpc_loc" variable created by PAROPT to determine the
  // actual name of the %struct.ident_t structure even when a numeric
  // extension is appended onto it.
  for (auto &GV : M.globals()) {
    if (!GV.hasName())
      continue;

    StringRef Name = GV.getName();
    if (Name.startswith(".kmpc_loc")) {
      auto *StTy = dyn_cast<llvm::StructType>(GV.getValueType());
      if (StTy && StTy->hasName())
        return TM.getStructType(StTy->getName());
    }
  }

  return nullptr;
}

DTransPointerType *DTransLibraryInfo::findIOPtrType(Module &M) {
  // Look for the Linux mangled name.
  DTransType *StTy = TM.getStructType("struct._ZTS8_IO_FILE._IO_FILE");
  if (StTy)
    return TM.getOrCreatePointerType(StTy);

  // Look for the Windows mangled name.
  StTy = TM.getStructType("struct..?AU_iobuf@@._iobuf");
  if (StTy)
    return TM.getOrCreatePointerType(StTy);

  return nullptr;
}

DTransFunctionType *
DTransLibraryInfo::getDTransFunctionType(const Function *F) {
  assert(DTransI1Type &&
         "DTransLibraryInfo class must be initialized before querying");
  auto It = FunctionCache.find(F);
  if (It != FunctionCache.end())
    return It->second;

  DTransFunctionType *DTy = getDTransFunctionTypeImpl(F);
  if (DTy)
    FunctionCache[F] = DTy;
  return DTy;
}

DTransType *DTransLibraryInfo::getFunctionReturnType(const Function *F) {
  assert(DTransI1Type &&
         "DTransLibraryInfo class must be initialized before querying");
  DTransFunctionType *DTy = getDTransFunctionType(F);
  if (!DTy)
    return nullptr;
  return DTy->getReturnType();
}

DTransType *DTransLibraryInfo::getFunctionArgumentType(const Function *F,
                                                       unsigned Idx) {
  assert(DTransI1Type &&
         "DTransLibraryInfo class must be initialized before querying");
  DTransFunctionType *DTy = getDTransFunctionType(F);
  if (!DTy)
    return nullptr;
  assert(Idx < DTy->getNumArgs() && "Invalid index");
  return DTy->getArgType(Idx);
}

DTransFunctionType *
DTransLibraryInfo::getDTransFunctionTypeImpl(const Function *F) {
  if (F->isIntrinsic())
    return getDTransFunctionTypeImpl(F->getIntrinsicID());

  // When the call is to a library function, we need to find a caller of the
  // function in order to get the TargetLibraryInfo class to lookup the LibFunc.
  const CallBase *Call = nullptr;
  for (auto &U : F->uses())
    if (const auto *Tmp = dyn_cast<CallBase>(U.getUser())) {
      Call = Tmp;
      break;
    }

  if (Call) {
    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    LibFunc TheLibFunc;
    if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc))
      return getDTransFunctionTypeImpl(TheLibFunc);
  }

  return nullptr;
}

DTransFunctionType *
DTransLibraryInfo::getDTransFunctionTypeImpl(LibFunc TheLibFunc) {
  switch (TheLibFunc) {
  default:
    break;

  case LibFunc_bcmp: {
    //  i32 bcmp(i8*, i8*, size_t)
    DTransType *ArgTypes[] = {DTransI8PtrType, DTransI8PtrType, DTransSizeType};
    return TM.getOrCreateFunctionType(DTransI32Type, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_calloc: {
    // i8* calloc(size_t, size_t)
    DTransType *ArgTypes[] = {DTransSizeType, DTransSizeType};
    return TM.getOrCreateFunctionType(DTransI8PtrType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_clang_call_terminate: {
    // void __clang_call_terminate(i8*)
    DTransType *ArgTypes[] = {DTransI8PtrType};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_cxa_atexit: {
    //  i32 __cxa_atexit(void (i8*)*, i8*, i8*)
    DTransType *FnTy =
        TM.getOrCreateFunctionType(DTransVoidType, {DTransI8PtrType},
                                   /*IsVarArg=*/false);
    DTransType *ArgTypes[] = {TM.getOrCreatePointerType(FnTy), DTransI8PtrType,
                              DTransI8PtrType};
    return TM.getOrCreateFunctionType(DTransI32Type, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_cxa_allocate_exception: {
    // i8* __cxa_allocate_exception(i64)
    DTransType *ArgTypes[] = {DTransSizeType};
    return TM.getOrCreateFunctionType(DTransI8PtrType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_cxa_begin_catch: {
    //  i8* __cxa_begin_catch(i8*)
    DTransType *ArgTypes[] = {DTransI8PtrType};
    return TM.getOrCreateFunctionType(DTransI8PtrType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_cxa_free_exception: {
    // void __cxa_free_exception(i8*)
    DTransType *ArgTypes[] = {DTransI8PtrType};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_cxa_get_exception_ptr: {
    // i8* __cxa_get_exception_ptr(i8*)
    DTransType *ArgTypes[] = {DTransI8PtrType};
    return TM.getOrCreateFunctionType(DTransI8PtrType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_cxa_throw: {
    // void __cxa_throw(i8*, i8*, i8*)
    DTransType *ArgTypes[] = {DTransI8PtrType, DTransI8PtrType,
                              DTransI8PtrType};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_fwrite: {
    // [Linux] size_t fwrite(i8*, size_t, size_t,
    //                       %struct._ZTS8_IO_FILE._IO_FILE*)
    // [Windows] size_t @fwrite(i8*, size_t, size_t,
    //                          %"struct..?AU_iobuf@@._iobuf"*)
    if (!DTransIOPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransI8PtrType, DTransSizeType, DTransSizeType,
                              DTransIOPtrType};
    return TM.getOrCreateFunctionType(DTransSizeType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_fputc: {
    // [Linux] i32 fputc(i32, %struct._ZTS8_IO_FILE._IO_FILE*)
    // [Windows] i32 fputc(i32, %"struct..?AU_iobuf@@._iobuf"*)
    if (!DTransIOPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransI32Type, DTransIOPtrType};
    return TM.getOrCreateFunctionType(DTransI32Type, ArgTypes,
                                      /*IsVarArg=*/false);
  }

#if INTEL_COLLAB
  case LibFunc_kmpc_barrier: {
    // void __kmpc_barrier(ident_t *loc, kmp_int32 global_tid);
    if (!DTransIdentTPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransIdentTPtrType, DTransI32Type};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_kmpc_critical:
  case LibFunc_kmpc_end_critical: {
    // void __kmpc_critical(ident_t *loc, kmp_int32 global_tid,
    //                      kmp_critical_name *crit);
    // or
    //
    // void __kmpc_end_critical(ident_t *loc, kmp_int32 global_tid,
    //                          kmp_critical_name *crit);
    if (!DTransIdentTPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransIdentTPtrType, DTransI32Type,
                              DTransKMPCriticalNamePtrType};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_kmpc_end_masked: {
    // void __kmpc_end_masked(ident_t *loc, int32_t global_tid)
    if (!DTransIdentTPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransIdentTPtrType, DTransI32Type};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/
                                      false);
  }

  case LibFunc_kmpc_end_single: {
    // void __kmpc_end_single(ident_t *loc, int32_t global_tid);
    if (!DTransIdentTPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransIdentTPtrType, DTransI32Type};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_kmpc_for_static_init_8:
  case LibFunc_kmpc_for_static_init_8u: {
    // void __kmpc_for_static_init_8[u](ident_t *loc, kmp_int32 gtid,
    //                               kmp_int32 schedtype, kmp_int32 *plastiter,
    //                               kmp_int64 *plower, kmp_int64 *pupper,
    //                               kmp_int64 *pstride, kmp_int64 incr,
    //                               kmp_int64 chunk);
    if (!DTransIdentTPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {
        DTransIdentTPtrType, DTransI32Type,    DTransI32Type,
        DTransI32PtrType,    DTransI64PtrType, DTransI64PtrType,
        DTransI64PtrType,    DTransI64Type,    DTransI64Type};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_kmpc_for_static_fini: {
    // void __kmpc_for_static_fini(ident_t *loc, kmp_int32 global_tid);
    if (!DTransIdentTPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransIdentTPtrType, DTransI32Type};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_kmpc_global_thread_num: {
    if (!DTransIdentTPtrType)
      return nullptr;
    // kmp_int32 __kmpc_global_thread_num(ident_t *loc);
    DTransType *ArgTypes[] = {DTransIdentTPtrType};
    return TM.getOrCreateFunctionType(DTransI32Type, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_kmpc_masked: {
    // int32_t __kmpc_masked(ident_t *loc, int32_t global_tid,
    //                        int32_t filter)
    if (!DTransIdentTPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransIdentTPtrType, DTransI32Type,
                              DTransI32Type};
    return TM.getOrCreateFunctionType(DTransI32Type, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_kmpc_push_num_threads: {
    // void __kmpc_push_num_threads(ident_t *loc, kmp_int32 global_tid,
    //                              kmp_int32 num_threads);
    if (!DTransIdentTPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransIdentTPtrType, DTransI32Type,
                              DTransI32Type};
    return TM.getOrCreateFunctionType(DTransVoidType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_kmpc_single: {
    // int32_t __kmpc_single(ident_t *loc, int32_t global_tid);
    if (!DTransIdentTPtrType)
      return nullptr;
    DTransType *ArgTypes[] = {DTransIdentTPtrType, DTransI32Type};
    return TM.getOrCreateFunctionType(DTransI32Type, ArgTypes,
                                      /*IsVarArg=*/false);
  }
#endif // INTEL_COLLAB

  case LibFunc_memchr: {
    // i8* memchr(i8*, i32, size_t)
    DTransType *ArgTypes[] = {DTransI8PtrType, DTransI32Type, DTransSizeType};
    return TM.getOrCreateFunctionType(DTransI8PtrType, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_puts: {
    //  i32 puts(i8*)
    DTransType *ArgTypes[] = {DTransI8PtrType};
    return TM.getOrCreateFunctionType(DTransI32Type, ArgTypes,
                                      /*IsVarArg=*/false);
  }

  case LibFunc_stpcpy: {
    // i8* stpcpy(i8*, i8*)
    DTransType *ArgTypes[] = {DTransI8PtrType, DTransI8PtrType};
    return TM.getOrCreateFunctionType(DTransI8PtrType, ArgTypes,
                                      /*IsVarArg=*/false);
  }
  }

  return nullptr;
}

DTransFunctionType *
DTransLibraryInfo::getDTransFunctionTypeImpl(Intrinsic::ID Id) {
  switch (Id) {
  case Intrinsic::eh_typeid_for:
    // i32 @llvm.eh.typeid.for(i8*)
    return TM.getOrCreateFunctionType(DTransI32Type, {DTransI8PtrType},
                                      /*IsVarArg=*/false);
  case Intrinsic::icall_branch_funnel:
    // void @llvm.icall.branch.funnel(...)
    return TM.getOrCreateFunctionType(DTransVoidType, {}, /*IsVarArg=*/true);

  case Intrinsic::lifetime_end:
  case Intrinsic::lifetime_start:
    // void llvm.lifetime.end(i64, i8*)
    // void llvm.lifetime.start(i64, i8*)
    return TM.getOrCreateFunctionType(DTransVoidType,
                                      {DTransI64Type, DTransI8PtrType},
                                      /*IsVarArg=*/false);

  case Intrinsic::memcpy:
  case Intrinsic::memmove:
    // void @llvm.memcpy(i8*, i8*, size_t, i1)
    // void @llvm.memmove(i8*, i8*, size_t, i1)
    return TM.getOrCreateFunctionType(
        DTransVoidType,
        {DTransI8PtrType, DTransI8PtrType, DTransSizeType, DTransI1Type},
        /*IsVarArg=*/false);

  case Intrinsic::memset:
    // void @llvm.memset.p0i8.i64(i8*, i8, size_t, i1)
    return TM.getOrCreateFunctionType(
        DTransVoidType,
        {DTransI8PtrType, DTransI8Type, DTransSizeType, DTransI1Type},
        /*IsVarArg=*/false);

  case Intrinsic::prefetch:
    // void @llvm.prefetch(i8*, i32, i32, i32)
    return TM.getOrCreateFunctionType(
        DTransVoidType,
        {DTransI8PtrType, DTransI32Type, DTransI32Type, DTransI32Type},
        /*IsVarArg=*/false);

  case Intrinsic::stackrestore:
    // void @llvm.stackrestore(i8*)
    return TM.getOrCreateFunctionType(DTransVoidType, {DTransI8PtrType},
                                      /*IsVarArg=*/false);

  case Intrinsic::stacksave:
    // i8* llvm.stacksave()
    return TM.getOrCreateFunctionType(DTransI8PtrType, {},
                                      /*IsVarArg=*/false);

  case Intrinsic::type_test:
    // i1 @llvm.type.test(i8*, metadata)
    return TM.getOrCreateFunctionType(
        DTransI1Type, {DTransI8PtrType, DTransMDType}, /*IsVarArg=*/false);

  case Intrinsic::vacopy:
    // void @llvm.va_copy(i8*, i8*)
    return TM.getOrCreateFunctionType(DTransVoidType,
                                      {DTransI8PtrType, DTransI8PtrType},
                                      /*IsVarArg=*/false);

  case Intrinsic::vaend:
  case Intrinsic::vastart:
    // void @llvm.va_end(i8*)
    // void @llvm.va_start(i8*)
    return TM.getOrCreateFunctionType(DTransVoidType, {DTransI8PtrType},
                                      /*IsVarArg=*/false);
  }

  return nullptr;
}

} // end namespace dtransOP
} // end namespace llvm
