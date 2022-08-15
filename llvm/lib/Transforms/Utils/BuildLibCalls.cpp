//===- BuildLibCalls.cpp - Utility builder for libcalls -------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements some functions that will create standard C libcalls.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/BuildLibCalls.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/Triple.h" // INTEL
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/TypeSize.h"

using namespace llvm;

#define DEBUG_TYPE "build-libcalls"

//- Infer Attributes ---------------------------------------------------------//

STATISTIC(NumReadNone, "Number of functions inferred as readnone");
STATISTIC(NumInaccessibleMemOnly,
          "Number of functions inferred as inaccessiblememonly");
STATISTIC(NumReadOnly, "Number of functions inferred as readonly");
STATISTIC(NumWriteOnly, "Number of functions inferred as writeonly");
STATISTIC(NumArgMemOnly, "Number of functions inferred as argmemonly");
STATISTIC(NumInaccessibleMemOrArgMemOnly,
          "Number of functions inferred as inaccessiblemem_or_argmemonly");
STATISTIC(NumNoUnwind, "Number of functions inferred as nounwind");
STATISTIC(NumNoCapture, "Number of arguments inferred as nocapture");
STATISTIC(NumWriteOnlyArg, "Number of arguments inferred as writeonly");
STATISTIC(NumReadOnlyArg, "Number of arguments inferred as readonly");
STATISTIC(NumNoAlias, "Number of function returns inferred as noalias");
STATISTIC(NumNoUndef, "Number of function returns inferred as noundef returns");
STATISTIC(NumReturnedArg, "Number of arguments inferred as returned");
#if INTEL_CUSTOMIZATION
STATISTIC(NumNoReturn, "Number of functions inferred as noreturn");
STATISTIC(NumFortran, "Number of functions inferred as Fortran");
STATISTIC(NumMustProgress, "Number of functions inferred as mustprogress");
#endif // INTEL_CUSTOMIZATION
STATISTIC(NumWillReturn, "Number of functions inferred as willreturn");

static bool setDoesNotAccessMemory(Function &F) {
  if (F.doesNotAccessMemory())
    return false;
  F.setDoesNotAccessMemory();
  ++NumReadNone;
  return true;
}

static bool setOnlyAccessesInaccessibleMemory(Function &F) {
  if (F.onlyAccessesInaccessibleMemory())
    return false;
  F.setOnlyAccessesInaccessibleMemory();
  ++NumInaccessibleMemOnly;
  return true;
}

static bool setOnlyReadsMemory(Function &F) {
  if (F.onlyReadsMemory())
    return false;
  F.setOnlyReadsMemory();
  ++NumReadOnly;
  return true;
}

static bool setOnlyWritesMemory(Function &F) {
  if (F.onlyWritesMemory()) // writeonly or readnone
    return false;
  // Turn readonly and writeonly into readnone.
  if (F.hasFnAttribute(Attribute::ReadOnly)) {
    F.removeFnAttr(Attribute::ReadOnly);
    return setDoesNotAccessMemory(F);
  }
  ++NumWriteOnly;
  F.setOnlyWritesMemory();
  return true;
}

static bool setOnlyAccessesArgMemory(Function &F) {
  if (F.onlyAccessesArgMemory())
    return false;
  F.setOnlyAccessesArgMemory();
  ++NumArgMemOnly;
  return true;
}

static bool setOnlyAccessesInaccessibleMemOrArgMem(Function &F) {
  if (F.onlyAccessesInaccessibleMemOrArgMem())
    return false;
  F.setOnlyAccessesInaccessibleMemOrArgMem();
  ++NumInaccessibleMemOrArgMemOnly;
  return true;
}

static bool setDoesNotThrow(Function &F) {
  if (F.doesNotThrow())
    return false;
  F.setDoesNotThrow();
  ++NumNoUnwind;
  return true;
}

static bool setRetDoesNotAlias(Function &F) {
  if (F.hasRetAttribute(Attribute::NoAlias))
    return false;
  F.addRetAttr(Attribute::NoAlias);
  ++NumNoAlias;
  return true;
}

static bool setDoesNotCapture(Function &F, unsigned ArgNo) {
  if (F.hasParamAttribute(ArgNo, Attribute::NoCapture))
    return false;
  F.addParamAttr(ArgNo, Attribute::NoCapture);
  ++NumNoCapture;
  return true;
}

static bool setDoesNotAlias(Function &F, unsigned ArgNo) {
  if (F.hasParamAttribute(ArgNo, Attribute::NoAlias))
    return false;
  F.addParamAttr(ArgNo, Attribute::NoAlias);
  ++NumNoAlias;
  return true;
}

static bool setOnlyReadsMemory(Function &F, unsigned ArgNo) {
  if (F.hasParamAttribute(ArgNo, Attribute::ReadOnly))
    return false;
  F.addParamAttr(ArgNo, Attribute::ReadOnly);
  ++NumReadOnlyArg;
  return true;
}

static bool setOnlyWritesMemory(Function &F, unsigned ArgNo) {
  if (F.hasParamAttribute(ArgNo, Attribute::WriteOnly))
    return false;
  F.addParamAttr(ArgNo, Attribute::WriteOnly);
  ++NumWriteOnlyArg;
  return true;
}

static bool setRetNoUndef(Function &F) {
  if (!F.getReturnType()->isVoidTy() &&
      !F.hasRetAttribute(Attribute::NoUndef)) {
    F.addRetAttr(Attribute::NoUndef);
    ++NumNoUndef;
    return true;
  }
  return false;
}

static bool setArgsNoUndef(Function &F) {
  bool Changed = false;
  for (unsigned ArgNo = 0; ArgNo < F.arg_size(); ++ArgNo) {
    if (!F.hasParamAttribute(ArgNo, Attribute::NoUndef)) {
      F.addParamAttr(ArgNo, Attribute::NoUndef);
      ++NumNoUndef;
      Changed = true;
    }
  }
  return Changed;
}

static bool setArgNoUndef(Function &F, unsigned ArgNo) {
  if (F.hasParamAttribute(ArgNo, Attribute::NoUndef))
    return false;
  F.addParamAttr(ArgNo, Attribute::NoUndef);
  ++NumNoUndef;
  return true;
}

static bool setRetAndArgsNoUndef(Function &F) {
  bool UndefAdded = false;
  UndefAdded |= setRetNoUndef(F);
  UndefAdded |= setArgsNoUndef(F);
  return UndefAdded;
}

static bool setReturnedArg(Function &F, unsigned ArgNo) {
  if (F.hasParamAttribute(ArgNo, Attribute::Returned))
    return false;
  F.addParamAttr(ArgNo, Attribute::Returned);
  ++NumReturnedArg;
  return true;
}

static bool setNonLazyBind(Function &F) {
  if (F.hasFnAttribute(Attribute::NonLazyBind))
    return false;
  F.addFnAttr(Attribute::NonLazyBind);
  return true;
}

static bool setDoesNotFreeMemory(Function &F) {
  if (F.hasFnAttribute(Attribute::NoFree))
    return false;
  F.addFnAttr(Attribute::NoFree);
  return true;
}

#if INTEL_CUSTOMIZATION
static bool setDoesNotReturn(Function &F) {
  if (F.doesNotReturn())
    return false;
  F.setDoesNotReturn();
  ++NumNoReturn;
  return true;
}

static bool setFortran(Function &F) {
  if (F.isFortran())
    return false;
  F.setFortran();
  ++NumFortran;
  return true;
}

static bool setMustProgress(Function &F) {
  if (F.hasFnAttribute(Attribute::MustProgress))
    return false;
  F.addFnAttr(Attribute::MustProgress);
  ++NumMustProgress;
  return true;
}
#endif // INTEL_CUSTOMIZATION

static bool setWillReturn(Function &F) {
  if (F.hasFnAttribute(Attribute::WillReturn))
    return false;
  F.addFnAttr(Attribute::WillReturn);
  ++NumWillReturn;
  return true;
}

static bool setAlignedAllocParam(Function &F, unsigned ArgNo) {
  if (F.hasParamAttribute(ArgNo, Attribute::AllocAlign))
    return false;
  F.addParamAttr(ArgNo, Attribute::AllocAlign);
  return true;
}

static bool setAllocatedPointerParam(Function &F, unsigned ArgNo) {
  if (F.hasParamAttribute(ArgNo, Attribute::AllocatedPointer))
    return false;
  F.addParamAttr(ArgNo, Attribute::AllocatedPointer);
  return true;
}

static bool setAllocSize(Function &F, unsigned ElemSizeArg,
                         Optional<unsigned> NumElemsArg) {
  if (F.hasFnAttribute(Attribute::AllocSize))
    return false;
  F.addFnAttr(Attribute::getWithAllocSizeArgs(F.getContext(), ElemSizeArg,
                                              NumElemsArg));
  return true;
}

static bool setAllocFamily(Function &F, StringRef Family) {
  if (F.hasFnAttribute("alloc-family"))
    return false;
  F.addFnAttr("alloc-family", Family);
  return true;
}

static bool setAllocKind(Function &F, AllocFnKind K) {
  if (F.hasFnAttribute(Attribute::AllocKind))
    return false;
  F.addFnAttr(
      Attribute::get(F.getContext(), Attribute::AllocKind, uint64_t(K)));
  return true;
}

bool llvm::inferNonMandatoryLibFuncAttrs(Module *M, StringRef Name,
                                         const TargetLibraryInfo &TLI) {
  Function *F = M->getFunction(Name);
  if (!F)
    return false;
  return inferNonMandatoryLibFuncAttrs(*F, TLI);
}

#if INTEL_CUSTOMIZATION
static bool isAllUsersFast(Function &F) {
  for (auto I = F.use_begin(), E = F.use_end(); I != E; ++I) {
    Instruction *Inst = dyn_cast_or_null<Instruction>(I->getUser());
    if (!Inst || !isa<FPMathOperator>(Inst) || !Inst->isFast())
      return false;
  }
  return true;
}
#endif // INTEL_CUSTOMIZATION

bool llvm::inferNonMandatoryLibFuncAttrs(Function &F,
                                         const TargetLibraryInfo &TLI) {
  LibFunc TheLibFunc;
  if (!(TLI.getLibFunc(F, TheLibFunc) && TLI.has(TheLibFunc)))
    return false;

  bool Changed = false;

#if INTEL_CUSTOMIZATION
  // CMPLRLLVM-23978:
  // SPIR target can't have unreachable code. SYCL library functions that
  // would normally have the "noreturn" attribute should be left with the
  // default attributes. We check only SYCL supported library functions below.
  bool IsSpirFunc = Triple(F.getParent()->getTargetTriple()).isSPIR();
#endif // INTEL_CUSTOMIZATION

  if(!isLibFreeFunction(&F, TheLibFunc) && !isReallocLikeFn(&F,  &TLI))
    Changed |= setDoesNotFreeMemory(F);

  if (F.getParent() != nullptr && F.getParent()->getRtLibUseGOT())
    Changed |= setNonLazyBind(F);

  switch (TheLibFunc) {
  case LibFunc_strlen:
  case LibFunc_strnlen:
  case LibFunc_wcslen:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_strchr:
  case LibFunc_strrchr:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    return Changed;
  case LibFunc_strtol:
  case LibFunc_strtod:
  case LibFunc_strtof:
  case LibFunc_strtoul:
  case LibFunc_strtoll:
  case LibFunc_strtold:
  case LibFunc_strtoull:
  case LibFunc_under_strtoi64:              // INTEL
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_strcat:
  case LibFunc_strncat:
  case LibFunc_wcscpy:                      // INTEL
  case LibFunc_wcsncat:                     // INTEL
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setReturnedArg(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    Changed |= setDoesNotAlias(F, 0);
    Changed |= setDoesNotAlias(F, 1);
    return Changed;
  case LibFunc_strcpy:
  case LibFunc_strncpy:
    Changed |= setReturnedArg(F, 0);
    LLVM_FALLTHROUGH;
  case LibFunc_stpcpy:
  case LibFunc_stpncpy:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyWritesMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    Changed |= setDoesNotAlias(F, 0);
    Changed |= setDoesNotAlias(F, 1);
    return Changed;
  case LibFunc_strxfrm:
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_strcmp:      // 0,1
  case LibFunc_strspn:      // 0,1
  case LibFunc_strncmp:     // 0,1
  case LibFunc_strcspn:     // 0,1
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setWillReturn(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_strcoll:
  case LibFunc_strcasecmp:  // 0,1
  case LibFunc_strncasecmp: //
  case LibFunc_under_stricmp:  // INTEL
  case LibFunc_under_strnicmp: // INTEL
    // Those functions may depend on the locale, which may be accessed through
    // global memory.
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_strstr:
  case LibFunc_strpbrk:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_strtok:
  case LibFunc_strtok_r:
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_scanf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_setbuf:
  case LibFunc_setvbuf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_strndup:
    Changed |= setArgNoUndef(F, 1);
    LLVM_FALLTHROUGH;
  case LibFunc_strdup:
    Changed |= setAllocFamily(F, "malloc");
    Changed |= setOnlyAccessesInaccessibleMemOrArgMem(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_stat:
  case LibFunc_statvfs:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_sscanf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_sprintf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotAlias(F, 0);
    Changed |= setOnlyWritesMemory(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_snprintf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotAlias(F, 0);
    Changed |= setOnlyWritesMemory(F, 0);
    Changed |= setDoesNotCapture(F, 2);
    Changed |= setOnlyReadsMemory(F, 2);
    return Changed;
  case LibFunc_setitimer:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setDoesNotCapture(F, 2);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_system:
    // May throw; "system" is a valid pthread cancellation point.
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_aligned_alloc:
    Changed |= setAlignedAllocParam(F, 0);
    Changed |= setAllocSize(F, 1, None);
    Changed |= setAllocKind(F, AllocFnKind::Alloc | AllocFnKind::Uninitialized | AllocFnKind::Aligned);
    LLVM_FALLTHROUGH;
  case LibFunc_valloc:
  case LibFunc_malloc:
  case LibFunc_vec_malloc:
    Changed |= setAllocFamily(F, TheLibFunc == LibFunc_vec_malloc ? "vec_malloc"
                                                                  : "malloc");
    Changed |= setAllocKind(F, AllocFnKind::Alloc | AllocFnKind::Uninitialized);
    Changed |= setAllocSize(F, 0, None);
    Changed |= setOnlyAccessesInaccessibleMemory(F);
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setWillReturn(F);
    return Changed;
  case LibFunc_memcmp:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_dunder_rawmemchr:                           // INTEL
  case LibFunc_memchr:
  case LibFunc_memrchr:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setWillReturn(F);
    return Changed;
  case LibFunc_modf:
  case LibFunc_modff:
  case LibFunc_modfl:
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_memcpy:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotAlias(F, 0);
    Changed |= setReturnedArg(F, 0);
    Changed |= setOnlyWritesMemory(F, 0);
    Changed |= setDoesNotAlias(F, 1);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_memmove:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setWillReturn(F);
    Changed |= setReturnedArg(F, 0);
    Changed |= setOnlyWritesMemory(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_mempcpy:
  case LibFunc_memccpy:
    Changed |= setWillReturn(F);
    LLVM_FALLTHROUGH;
  case LibFunc_memcpy_chk:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotAlias(F, 0);
    Changed |= setOnlyWritesMemory(F, 0);
    Changed |= setDoesNotAlias(F, 1);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_memalign:
    Changed |= setAllocFamily(F, "malloc");
    Changed |= setAllocKind(F, AllocFnKind::Alloc | AllocFnKind::Aligned |
                                   AllocFnKind::Uninitialized);
    Changed |= setAllocSize(F, 1, None);
    Changed |= setAlignedAllocParam(F, 0);
    Changed |= setOnlyAccessesInaccessibleMemory(F);
    Changed |= setRetNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setWillReturn(F);
    return Changed;
  case LibFunc_mkdir:
  case LibFunc_under_mkdir:                   // INTEL
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_mktime:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_realloc:
  case LibFunc_reallocf:
  case LibFunc_vec_realloc:
    Changed |= setAllocFamily(
        F, TheLibFunc == LibFunc_vec_realloc ? "vec_malloc" : "malloc");
    Changed |= setAllocKind(F, AllocFnKind::Realloc);
    Changed |= setAllocatedPointerParam(F, 0);
    Changed |= setAllocSize(F, 1, None);
    Changed |= setOnlyAccessesInaccessibleMemOrArgMem(F);
    Changed |= setRetNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setArgNoUndef(F, 1);
    return Changed;
#if INTEL_CUSTOMIZATION
  case LibFunc_re_compile_fastmap:
    return Changed;
  case LibFunc_re_search_2:
    return Changed;
#endif // INTEL_CUSTOMIZATION
  case LibFunc_read:
#if INTEL_CUSTOMIZATION
  // NOTE: The libfunc read is an alias to _read in Windows
  // (LibFunc_under_read)
  case LibFunc_under_read:
#endif // INTEL_CUSTOMIZATION
    // May throw; "read" is a valid pthread cancellation point.
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_rewind:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_rmdir:
  case LibFunc_remove:
  case LibFunc_under_wremove:                  // INTEL
  case LibFunc_realpath:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_rename:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_readlink:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_write:
    // May throw; "write" is a valid pthread cancellation point.
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_bcopy:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyWritesMemory(F, 1);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_bcmp:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_bzero:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyWritesMemory(F, 0);
    return Changed;
#if INTEL_CUSTOMIZATION
  case LibFunc_cexp:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
#endif // INTEL_CUSTOMIZATION
  case LibFunc_calloc:
  case LibFunc_vec_calloc:
    Changed |= setAllocFamily(F, TheLibFunc == LibFunc_vec_calloc ? "vec_malloc"
                                                                  : "malloc");
    Changed |= setAllocKind(F, AllocFnKind::Alloc | AllocFnKind::Zeroed);
    Changed |= setAllocSize(F, 0, 1);
    Changed |= setOnlyAccessesInaccessibleMemory(F);
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setWillReturn(F);
    return Changed;
  case LibFunc_chmod:
  case LibFunc_chown:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_ctermid:
  case LibFunc_clearerr:
  case LibFunc_closedir:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
#if INTEL_CUSTOMIZATION
  case LibFunc_cpow:
  case LibFunc_cpowf:
  case LibFunc_csqrt:
  case LibFunc_csqrtf:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setWillReturn(F);
    return Changed;
  case LibFunc_atexit:
    return Changed;
#endif // INTEL_CUSTOMIZATION
  case LibFunc_atoi:
  case LibFunc_atol:
  case LibFunc_atof:
  case LibFunc_atoll:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_access:
  case LibFunc_under_waccess:                   // INTEL
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_fopen:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_fdopen:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_feof:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_free:
  case LibFunc_vec_free:
    Changed |= setAllocFamily(F, TheLibFunc == LibFunc_vec_free ? "vec_malloc"
                                                                : "malloc");
    Changed |= setAllocKind(F, AllocFnKind::Free);
    Changed |= setAllocatedPointerParam(F, 0);
    Changed |= setOnlyAccessesInaccessibleMemOrArgMem(F);
    Changed |= setArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_fseek:
  case LibFunc_ftell:
  case LibFunc_fgetc:
  case LibFunc_fgetc_unlocked:
  case LibFunc_fseeko:
  case LibFunc_ftello:
  case LibFunc_fileno:
  case LibFunc_fflush:
  case LibFunc_fclose:
  case LibFunc_fsetpos:
  case LibFunc_flockfile:
  case LibFunc_funlockfile:
  case LibFunc_ftrylockfile:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_ferror:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F);
    return Changed;
  case LibFunc_fputc:
  case LibFunc_fputc_unlocked:
  case LibFunc_fstat:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_frexp:
  case LibFunc_frexpf:
  case LibFunc_frexpl:
    Changed |= setDoesNotThrow(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_fstatvfs:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_fgets:
  case LibFunc_fgets_unlocked:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 2);
    return Changed;
  case LibFunc_fread:
  case LibFunc_fread_unlocked:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 3);
    return Changed;
  case LibFunc_fwrite:
  case LibFunc_fwrite_unlocked:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 3);
    // FIXME: readonly #1?
    return Changed;
  case LibFunc_fputs:
  case LibFunc_fputs_unlocked:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_fscanf:
  case LibFunc_fprintf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_fgetpos:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_getc:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_getlogin_r:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_getc_unlocked:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_getenv:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_gets:
  case LibFunc_getchar:
  case LibFunc_getchar_unlocked:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getitimer:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_getpwnam:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_ungetc:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_uname:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
#if INTEL_CUSTOMIZATION
  case LibFunc_under_unlink:
#endif // INTEL_CUSTOMIZATION
  case LibFunc_unlink:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_unsetenv:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_utime:
  case LibFunc_utimes:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_putc:
  case LibFunc_putc_unlocked:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_puts:
  case LibFunc_printf:
  case LibFunc_perror:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_pread:
    // May throw; "pread" is a valid pthread cancellation point.
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_pwrite:
    // May throw; "pwrite" is a valid pthread cancellation point.
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_putchar:
  case LibFunc_putchar_unlocked:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_popen:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_pclose:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_vscanf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_vsscanf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_vfscanf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_vprintf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_vfprintf:
  case LibFunc_vsprintf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_vsnprintf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 2);
    Changed |= setOnlyReadsMemory(F, 2);
    return Changed;
  case LibFunc_open:
    // May throw; "open" is a valid pthread cancellation point.
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_opendir:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_tmpfile:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_times:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_htonl:
  case LibFunc_htons:
  case LibFunc_ntohl:
  case LibFunc_ntohs:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotAccessMemory(F);
    return Changed;
  case LibFunc_lstat:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_lchown:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_qsort:
    // May throw; places call through function pointer.
    // Cannot give undef pointer/size
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotCapture(F, 3);
    return Changed;
  case LibFunc_dunder_strndup:
    Changed |= setArgNoUndef(F, 1);
    LLVM_FALLTHROUGH;
  case LibFunc_dunder_strdup:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setWillReturn(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_dunder_strtok_r:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_under_IO_getc:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_under_IO_putc:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_dunder_isoc99_scanf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_stat64:
  case LibFunc_lstat64:
  case LibFunc_statvfs64:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
#if INTEL_CUSTOMIZATION
  // stat and its other form can throw an exception handler in Windows
  case LibFunc_under_stat64:
  case LibFunc_under_stat64i32:
   case LibFunc_under_wstat64:
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
#endif // INTEL_CUSTOMIZATION
  case LibFunc_dunder_isoc99_sscanf:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_fopen64:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_fseeko64:
  case LibFunc_ftello64:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_tmpfile64:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_fstat64:
  case LibFunc_fstatvfs64:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_open64:
    // May throw; "open" is a valid pthread cancellation point.
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_gettimeofday:
    // Currently some platforms have the restrict keyword on the arguments to
    // gettimeofday. To be conservative, do not add noalias to gettimeofday's
    // arguments.
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
#if INTEL_CUSTOMIZATION
  case LibFunc_msvc_std_bad_alloc_ctor:
  case LibFunc_msvc_std_bad_alloc_scalar_deleting_dtor:
  case LibFunc_msvc_std_bad_array_new_length_ctor:
  case LibFunc_msvc_std_bad_array_new_length_scalar_deleting_dtor:
  case LibFunc_msvc_std_basic_filebuf_scalar_deleting_dtor:
  case LibFunc_msvc_std_basic_filebuf_dtor:
  case LibFunc_msvc_std_basic_filebuf_imbue:
  case LibFunc_msvc_std_basic_filebuf_overflow:
  case LibFunc_msvc_std_basic_filebuf_pbackfail:
  case LibFunc_msvc_std_basic_filebuf_setbuf:
  case LibFunc_msvc_std_basic_filebuf_std_fpos_seekoff:
  case LibFunc_msvc_std_basic_filebuf_std_fpos_seekpos:
  case LibFunc_msvc_std_basic_filebuf_sync:
  case LibFunc_msvc_std_basic_filebuf_uflow:
    return Changed;
  case LibFunc_msvc_std_basic_filebuf_under_Endwrite:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_msvc_std_basic_filebuf_underflow:
  case LibFunc_msvc_std_basic_filebuf_under_Lock:
  case LibFunc_msvc_std_basic_filebuf_under_Unlock:
  case LibFunc_msvc_std_basic_filebuf_xsgetn:
  case LibFunc_msvc_std_basic_filebuf_xsputn:
  case LibFunc_msvc_std_basic_ios_scalar_deleting_dtor:
  case LibFunc_msvc_std_basic_istream_vector_deleting_dtor:
  case LibFunc_msvc_std_basic_ostream_vector_deleting_dtor:
  case LibFunc_msvc_std_basic_streambuf_dtor:
  case LibFunc_msvc_std_basic_streambuf_imbue:
  case LibFunc_msvc_std_basic_streambuf_under_Lock:
  case LibFunc_msvc_std_basic_streambuf_overflow:
  case LibFunc_msvc_std_basic_streambuf_pbackfail:
  case LibFunc_msvc_std_basic_streambuf_scalar_deleting_dtor:
  case LibFunc_msvc_std_basic_streambuf_setbuf:
  case LibFunc_msvc_std_basic_ios_setstate:
  case LibFunc_msvc_std_basic_streambuf_showmanyc:
  case LibFunc_msvc_std_basic_streambuf_std_fpos_seekoff:
  case LibFunc_msvc_std_basic_streambuf_std_fpos_seekpos:
  case LibFunc_msvc_std_basic_streambuf_sync:
  case LibFunc_msvc_std_basic_streambuf_uflow:
  case LibFunc_msvc_std_basic_streambuf_under_Unlock:
  case LibFunc_msvc_std_basic_streambuf_underflow:
  case LibFunc_msvc_std_basic_streambuf_xsgetn:
  case LibFunc_msvc_std_basic_streambuf_xsputn:
  case LibFunc_msvc_std_basic_string_append:
  case LibFunc_msvc_std_basic_string_append_size_value:
  case LibFunc_msvc_std_basic_string_assign_const_ptr:
  case LibFunc_msvc_std_basic_string_assign_const_ptr_size:
  case LibFunc_msvc_std_basic_string_ctor:
  case LibFunc_msvc_std_basic_string_ptr64_ctor:
  case LibFunc_msvc_std_basic_string_dtor:
  case LibFunc_msvc_std_basic_string_insert:
  case LibFunc_msvc_std_basic_string_operator_equal_const_ptr:
  case LibFunc_msvc_std_basic_string_operator_equal_ptr64:
  case LibFunc_msvc_std_basic_string_operator_plus_equal_char:
  case LibFunc_msvc_std_basic_string_push_back:
  case LibFunc_msvc_std_basic_string_resize:
    return Changed;
  case LibFunc_msvc_std_basic_string_under_xlen:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_msvc_std_codecvt_do_always_noconv:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_msvc_std_codecvt_scalar_deleting_dtor:
  case LibFunc_msvc_std_codecvt_use_facet:
    return Changed;
  case LibFunc_msvc_std_codecvt_do_encoding:
  case LibFunc_msvc_std_codecvt_do_in:
  case LibFunc_msvc_std_codecvt_do_length:
  case LibFunc_msvc_std_codecvt_do_max_length:
  case LibFunc_msvc_std_ctype_do_narrow_char_char:
  case LibFunc_msvc_std_ctype_do_narrow_ptr_ptr_char_ptr:
  case LibFunc_msvc_std_codecvt_do_out:
  case LibFunc_msvc_std_ctype_do_tolower_char:
  case LibFunc_msvc_std_ctype_do_tolower_ptr_ptr:
  case LibFunc_msvc_std_ctype_do_toupper_char:
  case LibFunc_msvc_std_ctype_do_toupper_ptr_ptr:
  case LibFunc_msvc_std_codecvt_do_unshift:
  case LibFunc_msvc_std_ctype_do_widen_char:
  case LibFunc_msvc_std_ctype_do_widen_ptr_ptr_ptr:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_msvc_std_ctype_scalar_deleting_dtor:
    return Changed;
  case LibFunc_msvc_std_error_category_default_error:
  case LibFunc_msvc_std_error_category_equivalent_error_code:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_msvc_std_ctype_use_facet:
  case LibFunc_msvc_std_CxxThrowException:
  case LibFunc_msvc_std_error_category_equivalent_error_condition:
  case LibFunc_msvc_std_error_code_make_error_code:
  case LibFunc_msvc_std_Execute_once:
  case LibFunc_msvc_std_exception_const_ptr_ctor:
  case LibFunc_msvc_std_exception_dtor:
  case LibFunc_msvc_std_exception_scalar_deleting_dtor:
  case LibFunc_msvc_std_exception_what:
  case LibFunc_msvc_std_facet_register:
  case LibFunc_msvc_std_under_Fiopen:
  case LibFunc_msvc_std_ios_base_under_Ios_base_dtor:
  case LibFunc_msvc_std_ios_base_failure:
  case LibFunc_msvc_std_ios_base_failure_const_ptr_ctor:
  case LibFunc_msvc_std_ios_base_failure_scalar_deleting_dtor:
  case LibFunc_msvc_std_ios_base_scalar_deleting_dtor:
  case LibFunc_msvc_std_istreambuf_iterator_operator_plus_plus:
  case LibFunc_msvc_std_locale_facet_decref:
  case LibFunc_msvc_std_locale_facet_incref:
  case LibFunc_msvc_std_locale_under_Init:
  case LibFunc_msvc_std_locimp_Getgloballocale:
  case LibFunc_msvc_std_locinfo_ctor:
  case LibFunc_msvc_std_locinfo_dtor:
  case LibFunc_msvc_std_lockit:
  case LibFunc_msvc_std_lockit_dtor:
  case LibFunc_msvc_std_num_get_do_get_bool_ptr:
  case LibFunc_msvc_std_num_get_do_get_double_ptr:
  case LibFunc_msvc_std_num_get_do_get_float_ptr:
  case LibFunc_msvc_std_num_get_do_get_long_double_ptr:
  case LibFunc_msvc_std_num_get_do_get_long_long_ptr:
  case LibFunc_msvc_std_num_get_do_get_long_ptr:
  case LibFunc_msvc_std_num_get_do_get_unsigned_int_ptr:
  case LibFunc_msvc_std_num_get_do_get_unsigned_long_long_ptr:
  case LibFunc_msvc_std_num_get_do_get_unsigned_long_ptr:
  case LibFunc_msvc_std_num_get_do_get_unsigned_short_ptr:
  case LibFunc_msvc_std_num_get_do_get_void_ptr:
  case LibFunc_msvc_std_num_get_scalar_deleting_dtor:
  case LibFunc_msvc_std_num_get_under_Getffld:
  case LibFunc_msvc_std_num_get_under_Getifld:
  case LibFunc_msvc_std_num_get_use_facet:
  case LibFunc_msvc_std_numpunct_Tidy:
  case LibFunc_msvc_std_runtime_error_ctor:
  case LibFunc_msvc_std_runtime_error_char_ctor:
  case LibFunc_msvc_std_runtime_error_ptr64_ctor:
  case LibFunc_msvc_std_runtime_error_scalar_deleting_dtor:
  case LibFunc_msvc_std_Syserror_map:
  case LibFunc_msvc_std_under_system_error_const_ptr_ctor:
  case LibFunc_msvc_std_uncaught_exception:
  case LibFunc_msvc_std_under_locinfo_ctor:
  case LibFunc_msvc_std_under_locinfo_dtor:
  case LibFunc_msvc_std_system_error_const_ptr_ctor:
  case LibFunc_msvc_std_system_error_scalar_deleting_dtor:
  case LibFunc_msvc_std_under_generic_error_category_message:
  case LibFunc_msvc_std_under_iostreamer_error_category_message:
  case LibFunc_msvc_std_under_iostreamer_error_category2_message:
    return Changed;
  case LibFunc_msvc_std_under_iostreamer_error_category_name:
  case LibFunc_msvc_std_under_iostreamer_error_category2_name:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_msvc_std_under_iostreamer_error_category_scalar_deleting_dtor:
  case LibFunc_msvc_std_under_iostreamer_error_category2_scalar_deleting_dtor:
  case LibFunc_msvc_std_under_system_error_scalar_deleting_dtor:
  case LibFunc_msvc_std_under_Throw_bad_array_new_length:
  case LibFunc_msvc_std_under_Xlen_string:
  case LibFunc_msvc_std_Xbad_alloc:
  case LibFunc_msvc_std_yarn_dtor:
  case LibFunc_msvc_std_yarn_wchar_dtor:
    return Changed;
  case LibFunc_msvc_std_Xlength_error:
  case LibFunc_msvc_std_Xout_of_range:
  case LibFunc_msvc_std_Xran:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_msvc_std_Xruntime_error:
    return Changed;
  case LibFunc_msvc_std_numpunct_do_decimal_point:
  case LibFunc_msvc_std_numpunct_do_thousands_sep:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_msvc_std_numpunct_do_falsename:
  case LibFunc_msvc_std_numpunct_do_grouping:
  case LibFunc_msvc_std_numpunct_use_facet:
  case LibFunc_msvc_std_numpunct_do_truename:
  case LibFunc_msvc_std_num_put_do_put_bool:
  case LibFunc_msvc_std_num_put_do_put_double:
  case LibFunc_msvc_std_num_put_do_put_long:
  case LibFunc_msvc_std_num_put_do_put_long_double:
  case LibFunc_msvc_std_num_put_do_put_long_long:
  case LibFunc_msvc_std_num_put_do_put_ulong:
  case LibFunc_msvc_std_num_put_do_put_ulong_long:
  case LibFunc_msvc_std_num_put_do_put_void_ptr:
  case LibFunc_msvc_std_num_put_ostreambuf_iterator_Fput:
  case LibFunc_msvc_std_num_put_ostreambuf_iterator_iput:
  case LibFunc_msvc_std_num_put_ostreambuf_iterator_scalar_deleting_dtor:
  case LibFunc_msvc_std_num_put_use_facet:
  case LibFunc_msvc_std_numpunct_scalar_deleting_dtor:
    return Changed;
  case LibFunc_msvc_std_under_immortalize_impl:
    Changed |= setDoesNotThrow(F);
    return Changed;
#endif // INTEL_CUSTOMIZATION
  case LibFunc_memset_pattern4:
  case LibFunc_memset_pattern8:
  case LibFunc_memset_pattern16:
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    LLVM_FALLTHROUGH;
  case LibFunc_memset:
    Changed |= setWillReturn(F);
    LLVM_FALLTHROUGH;
  case LibFunc_memset_chk:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setOnlyWritesMemory(F, 0);
    Changed |= setDoesNotThrow(F);
    return Changed;
  // int __nvvm_reflect(const char *)
  case LibFunc_nvvm_reflect:
    Changed |= setRetAndArgsNoUndef(F);
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
#if INTEL_CUSTOMIZATION
  case LibFunc_assert_fail:
    if (!IsSpirFunc)
      Changed |= setDoesNotReturn(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_clang_call_terminate:
    return Changed;
  case LibFunc_ctype_b_loc:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ctype_get_mb_cur_max:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ctype_tolower_loc:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ctype_toupper_loc:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_cxa_allocate_exception:
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_cxa_bad_typeid:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_cxa_bad_cast:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_cxa_begin_catch:
    return Changed;
  case LibFunc_cxa_call_unexpected:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_cxa_end_catch:
    return Changed;
  case LibFunc_cxa_free_exception:
    return Changed;
  case LibFunc_cxa_get_exception_ptr:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_cxa_rethrow:
    return Changed;
  case LibFunc_cxa_pure_virtual:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_cxa_throw:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_dynamic_cast:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setMustProgress(F);
    Changed |= setWillReturn(F);
    return Changed;
  case LibFunc_errno_location:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_fxstat:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_fxstat64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_gnu_std_basic_filebuf_dtor:
    return Changed;
  case LibFunc_gnu_std_cxx11_basic_stringstream_ctor:
    return Changed;
  case LibFunc_ZNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcESaIcEED1Ev:
    return Changed;
  case LibFunc_gxx_personality_v0:
    return Changed;
  case LibFunc_isinf:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_isnan:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_isnanf:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_atomic_fixed4_add:
  case LibFunc_kmpc_atomic_float8_add:
    return Changed;
  case LibFunc_kmpc_critical:
  case LibFunc_kmpc_critical_with_hint:
    return Changed;
  case LibFunc_kmpc_dispatch_init_4:
  case LibFunc_kmpc_dispatch_init_4u:
  case LibFunc_kmpc_dispatch_init_8:
  case LibFunc_kmpc_dispatch_init_8u:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_dispatch_next_4:
  case LibFunc_kmpc_dispatch_next_4u:
  case LibFunc_kmpc_dispatch_next_8:
  case LibFunc_kmpc_dispatch_next_8u:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_end_critical:
    return Changed;
  case LibFunc_kmpc_end_reduce_nowait:
    return Changed;
  case LibFunc_kmpc_end_serialized_parallel:
    return Changed;
  case LibFunc_kmpc_flush:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_for_static_fini:
    return Changed;
  case LibFunc_kmpc_for_static_init_4:
  case LibFunc_kmpc_for_static_init_4u:
    return Changed;
  case LibFunc_kmpc_for_static_init_8:
  case LibFunc_kmpc_for_static_init_8u:
    return Changed;
  case LibFunc_kmpc_fork_call:
    return Changed;
  case LibFunc_kmpc_global_thread_num:
    return Changed;
  case LibFunc_kmpc_ok_to_fork:
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_omp_task:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_omp_task_alloc:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_omp_task_begin_if0:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_omp_task_complete_if0:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_omp_taskwait:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_push_num_threads:
    return Changed;
  case LibFunc_kmpc_reduce_nowait:
    return Changed;
  case LibFunc_kmpc_serialized_parallel:
    return Changed;
  case LibFunc_kmpc_single:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_end_single:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_masked:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_end_masked:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_master:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_end_master:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_threadprivate_cached:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_lxstat:
  case LibFunc_lxstat64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_regcomp:
  case LibFunc_regerror:
  case LibFunc_regexec:
  case LibFunc_regfree:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_sigsetjmp:
    return Changed;
  case LibFunc_std_exception_copy:
    return Changed;
  case LibFunc_std_exception_destroy:
    return Changed;
  case LibFunc_std_reverse_trivially_swappable_8:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_strncpy_s:
    Changed |= setDoesNotCapture(F, 2);
    Changed |= setOnlyReadsMemory(F, 2);
    return Changed;
  case LibFunc_tunder_mb_cur_max_func:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_dunder_CxxFrameHandler3:
    return Changed;
  case LibFunc_dunder_RTDynamicCast:
    return Changed;
  case LibFunc_dunder_RTtypeid:
    return Changed;
  case LibFunc_dunder_std_terminate:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_dunder_std_type_info_compare:
    return Changed;
  case LibFunc_dunder_std_type_info_name:
    return Changed;
  case LibFunc_sysv_signal:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_Getctype:
    return Changed;
  case LibFunc_under_Getcvt:
    return Changed;
  case LibFunc_under_Init_thread_abort:
    return Changed;
  case LibFunc_under_get_stream_buffer_pointers:
    return Changed;
  // Get working directory in Windows can throw an exception
  // compared to the Linux version (LibFunc_getcwd)
  case LibFunc_under_getcwd:
  case LibFunc_under_getdcwd:
    return Changed;
  case LibFunc_under_getdrive:
    Changed |= setDoesNotThrow(F);
    return Changed;
  // gmtime in Windows can throw an exception as opposed to the
  // Linux version (LibFunc_gmtime)
  case LibFunc_under_gmtime64:
    return Changed;
  case LibFunc_under_Stollx:
  case LibFunc_under_Stolx:
  case LibFunc_under_Stoullx:
  case LibFunc_under_Stoulx:
    return Changed;
  case LibFunc_under_Tolower:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_Toupper:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_chdir:
    return Changed;
  case LibFunc_under_commit:
    return Changed;
  // _close in Windows can throw an exception compared to
  // the Linux version (LibFunc_close)
  case LibFunc_under_close:
    return Changed;
  case LibFunc_under_errno:
    return Changed;
  case LibFunc_under_fdopen:
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_under_fseeki64:
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_under_fstat64:
  case LibFunc_under_fstat64i32:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_exit:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_under_fileno:
    return Changed;
  case LibFunc_under_findclose:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_findfirst64i32:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_findnext64i32:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_ftelli64:
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_under_Init_thread_header:
  case LibFunc_under_Init_thread_footer:
    return Changed;
  case LibFunc_under_invalid_parameter_noinfo_noreturn:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_under_localtime64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_lock_file:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_lseeki64:
    return Changed;
  case LibFunc_under_set_errno:
    return Changed;
  case LibFunc_under_setmode:
    return Changed;
  case LibFunc_under_purecall:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_obstack_begin:
    return Changed;
  case LibFunc_obstack_memory_used:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_obstack_newchunk:
    return Changed;
  case LibFunc_setjmp:
    return Changed;
  case LibFunc_ZNKSs17find_first_not_ofEPKcmm:
    return Changed;
  case LibFunc_ZNKSs4findEcm:
    return Changed;
  case LibFunc_ZNKSs4findEPKcmm:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    return Changed;
  case LibFunc_ZNKSs5rfindEcm:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    return Changed;
  case LibFunc_ZNKSs5rfindEPKcmm:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    return Changed;
  case LibFunc_ZNKSs7compareEPKc:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ZNKSt13runtime_error4whatEv:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ZNKSt5ctypeIcE13_M_widen_initEv:
    return Changed;
  case LibFunc_std_cxx11_basic_string_find_first_not_of:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    return Changed;
  case LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE4findEPKcmm:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    return Changed;
  case LibFunc_std_cxx11_basic_string_find_char_unsigned_long:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    return Changed;
  case LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5rfindEPKcmm:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    return Changed;
  case LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5rfindEcm:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    return Changed;
  case LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7compareEPKc:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ZNKSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE3strEv:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    return Changed;
  case LibFunc_ZNKSt9bad_alloc4whatEv:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ZNKSt9basic_iosIcSt11char_traitsIcEE5widenEc:
    return Changed;
  case LibFunc_ZNKSt9exception4whatEv:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ZNSi10_M_extractIdEERSiRT_:
    return Changed;
  case LibFunc_ZNSi10_M_extractIfEERSiRT_:
    return Changed;
  case LibFunc_ZNSi10_M_extractIlEERSiRT_:
    return Changed;
  case LibFunc_ZNSi10_M_extractImEERSiRT_:
    return Changed;
  case LibFunc_ZNSi4readEPci:
    return Changed;
  case LibFunc_ZNSi4readEPcl:
    return Changed;
  case LibFunc_ZNSi5tellgEv:
    Changed |= setOnlyReadsMemory(F);
    return Changed;
  case LibFunc_ZNSi5ungetEv:
    return Changed;
  case LibFunc_ZNSirsERi:
    return Changed;
  case LibFunc_ZNSo3putEc:
    return Changed;
  case LibFunc_ZNSo5flushEv:
    return Changed;
  case LibFunc_ZNSo5writeEPKci:
    return Changed;
  case LibFunc_ZNSo5writeEPKcl:
    return Changed;
  case LibFunc_ZNSo9_M_insertIbEERSoT_:
    return Changed;
  case LibFunc_ZNSo9_M_insertIdEERSoT_:
    return Changed;
  case LibFunc_ZNSo9_M_insertIlEERSoT_:
    return Changed;
  case LibFunc_ZNSo9_M_insertImEERSoT_:
    return Changed;
  case LibFunc_ZNSo9_M_insertIPKvEERSoT_:
    return Changed;
  case LibFunc_ZNSolsEi:
    return Changed;
  case LibFunc_ZNSs12_M_leak_hardEv:
    return Changed;
  case LibFunc_ZNSs4_Rep10_M_destroyERKSaIcE:
    return Changed;
  case LibFunc_ZNSs4_Rep9_S_createEmmRKSaIcE:
    return Changed;
  case LibFunc_ZNSs6appendEmc:
    return Changed;
  case LibFunc_ZNSs6appendEPKcm:
    return Changed;
  case LibFunc_ZNSs6appendERKSs:
    return Changed;
  case LibFunc_ZNSs6assignEPKcm:
    return Changed;
  case LibFunc_ZNSs6assignERKSs:
    return Changed;
  case LibFunc_ZNSs6insertEmPKcm:
    return Changed;
  case LibFunc_ZNSs6resizeEmc:
    return Changed;
  case LibFunc_ZNSs7replaceEmmPKcm:
    return Changed;
  case LibFunc_ZNSs7reserveEm:
    return Changed;
  case LibFunc_ZNSs9_M_mutateEmmm:
    return Changed;
  case LibFunc_ZNSsC1EPKcmRKSaIcE:
    return Changed;
  case LibFunc_ZNSsC1EPKcRKSaIcE:
    return Changed;
  case LibFunc_ZNSsC1ERKSs:
    return Changed;
  case LibFunc_ZNSsC1ERKSsmm:
    return Changed;
  case LibFunc_ZNSt12__basic_fileIcED1Ev:
    return Changed;
  case LibFunc_ZNSt13basic_filebufIcSt11char_traitsIcEE4openEPKcSt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt13basic_filebufIcSt11char_traitsIcEE5closeEv:
    return Changed;
  case LibFunc_ZNSt13basic_filebufIcSt11char_traitsIcEEC1Ev:
    return Changed;
  case LibFunc_ZNSt13runtime_errorC1EPKc:
    return Changed;
  case LibFunc_ZNSt13runtime_errorC1ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE:
    return Changed;
  case LibFunc_ZNSt13runtime_errorC1ERKSs:
    return Changed;
  case LibFunc_ZNSt13runtime_errorC1ERKS_:
    return Changed;
  case LibFunc_ZNSt13runtime_errorC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE:
    return Changed;
  case LibFunc_std_runtime_error_std_runtime_error_const:
    return Changed;
  case LibFunc_ZNSt13runtime_errorC2ERKSs:
    return Changed;
  case LibFunc_ZNSt13runtime_errorC2EPKc:
  case LibFunc_ZNSt13runtime_errorD0Ev:
    return Changed;
  case LibFunc_ZNSt13runtime_errorD1Ev:
    return Changed;
  case LibFunc_ZNSt13runtime_errorD2Ev:
    return Changed;
  case LibFunc_ZNSt14basic_ifstreamIcSt11char_traitsIcEEC1EPKcSt13_Ios_Openmode:
    return Changed;
  case LibFunc_std_basic_ifstream_ctor:
    return Changed;
  case LibFunc_ZNSt14basic_ifstreamIcSt11char_traitsIcEED1Ev:
    return Changed;
  case LibFunc_ZNSt14basic_ifstreamIcSt11char_traitsIcEED2Ev:
    return Changed;
  case LibFunc_ZNSt14basic_ofstreamIcSt11char_traitsIcEEC1EPKcSt13_Ios_Openmode:
    return Changed;
  case LibFunc_std_cxx11_basic_ostringstream_ctor:
    return Changed;
  case LibFunc_std_basic_ofstream_dtor:
    return Changed;
  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE5imbueERKSt6locale:
    return Changed;
  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE5uflowEv:
    return Changed;
  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl:
    return Changed;
  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsputnEPKcl:
    return Changed;
  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEED2Ev:
    return Changed;
  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE7_M_syncEPcmm:
    return Changed;
  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE7seekoffElSt12_Ios_SeekdirSt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE7seekposESt4fposI11__mbstate_tESt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE8overflowEi:
    return Changed;
  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE9pbackfailEi:
    return Changed;
  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE9underflowEv:
    return Changed;
  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEEC2ERKSsSt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt6localeC1Ev:
    return Changed;
  case LibFunc_ZNSt6localeD1Ev:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6resizeEmc:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7reserveEm:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE8_M_eraseEmm:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcm:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_assignERKS4_:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERmm:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEmmPKcm:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_replaceEmmPKcm:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE14_M_replace_auxEmmmc:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5eraseEmm:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2EPKcRKS3_:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2ERKS4_mm:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED2Ev:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2EOS4:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE6setbufEPcl:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7_M_syncEPcmm:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7seekoffElSt12_Ios_SeekdirSt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7seekposESt4fposI11__mbstate_tESt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE8overflowEi:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE9pbackfailEi:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE9showmanycEv:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE9underflowEv:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEEC2ERKNS_12basic_stringIcS2_S3_EESt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEED2Ev:
    return Changed;
  case LibFunc_ZNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcESaIcEEC1ERKNS_12basic_stringIcS2_S3_EESt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcESaIcEEC1ESt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEC1ERKNS_12basic_stringIcS2_S3_EESt13_Ios_Openmode:
    return Changed;
  case LibFunc_ZNSt7__cxx1119basic_ostringstreamIcSt11char_traitsIcESaIcEEC1ESt13_Ios_Openmode:
    return Changed;
  case LibFunc_std_cxx11_basic_ostringstream_dtor:
    return Changed;
  case LibFunc_ZNSt8__detail15_List_node_base11_M_transferEPS0_S1_:
    return Changed;
  case LibFunc_ZNSt8__detail15_List_node_base7_M_hookEPS0_:
    return Changed;
  case LibFunc_ZNSt8__detail15_List_node_base9_M_unhookEv:
    return Changed;
  case LibFunc_ZNSt8ios_base4InitC1Ev:
    return Changed;
  case LibFunc_ZNSt8ios_base4InitD1Ev:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ZNSt8ios_baseC2Ev:
    return Changed;
  case LibFunc_ZNSt8ios_baseD2Ev:
    return Changed;
  case LibFunc_ZNSt9bad_allocD0Ev:
    return Changed;
  case LibFunc_ZNSt9bad_allocD1Ev:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2ERKS4_:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6appendEPKc:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6appendERKS4:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6assignEPKc:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6insertEmRKS4_:
    return Changed; 
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEOS4_:
    return Changed;
  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEPKc:
    return Changed;
  case LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE4initEPSt15basic_streambufIcS1_E:
    return Changed;
  case LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate:
    return Changed;
  case LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE5rdbufEPSt15basic_streambufIcS1_E:
    return Changed;
  case LibFunc_ZNSt9exceptionD0Ev:
    return Changed;
  case LibFunc_ZNSt9exceptionD1Ev:
    return Changed;
  case LibFunc_ZNSt9exceptionD2Ev:
    return Changed;
  case LibFunc_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_i:
    return Changed;
  case LibFunc_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l:
    return Changed;
  case LibFunc_ZSt16__throw_bad_castv:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_ZSt17__throw_bad_allocv:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_ZSt18_Rb_tree_decrementPKSt18_Rb_tree_node_base:
    return Changed;
  case LibFunc_ZSt18_Rb_tree_decrementPSt18_Rb_tree_node_base:
    return Changed;
  case LibFunc_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base:
    return Changed;
  case LibFunc_ZSt18_Rb_tree_incrementPSt18_Rb_tree_node_base:
    return Changed;
  case LibFunc_ZSt19__throw_logic_errorPKc:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_ZSt20__throw_length_errorPKc:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_ZSt20__throw_out_of_rangePKc:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_ZSt24__throw_out_of_range_fmtPKcz:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_ZSt28_Rb_tree_rebalance_for_erasePSt18_Rb_tree_node_baseRS_:
    return Changed;
  case LibFunc_ZSt28__throw_bad_array_new_lengthv:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_ZSt29_Rb_tree_insert_and_rebalancebPSt18_Rb_tree_node_baseS0_RS_:
  case LibFunc_std_basic_ostream_std_endl:
  case LibFunc_std_basic_ostream_std_flush:
  case LibFunc_ZSt7getlineIcSt11char_traitsIcESaIcEERSt13basic_istreamIT_T0_ES7_RSbIS4_S5_T1_ES4_:
  case LibFunc_std_basic_istream_getline_cxx11_char:
    return Changed;
  case LibFunc_ZSt9terminatev:
    Changed |= setDoesNotReturn(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc:
    return Changed;
  case LibFunc_ZStrsIcSt11char_traitsIcEERSt13basic_istreamIT_T0_ES6_RS3_:
    return Changed;
  case LibFunc_abort:
    if (!IsSpirFunc)
      Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_acrt_iob_func:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_alphasort:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_asctime:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_asprintf:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_backtrace:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_backtrace_symbols:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_bsearch:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_chdir:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_clock:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_close:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ctime:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_CloseHandle:
    return Changed;
  case LibFunc_ConvertThreadToFiber:
    return Changed;
  case LibFunc_CreateFiber:
    return Changed;
  case LibFunc_CreateFileA:
  case LibFunc_CreateFileW:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_DeleteCriticalSection:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_DeleteFiber:
    return Changed;
  case LibFunc_difftime:
  case LibFunc_under_difftime64:  // INTEL
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_div:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_dup:
  case LibFunc_dup2:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_EnterCriticalSection:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_erfc:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_error:
    if (!IsSpirFunc)
      Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_execl:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_execv:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_execvp:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_fcntl:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_fcntl64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_fnmatch:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_for_adjustl:
  case LibFunc_for_alloc_allocatable:
  case LibFunc_for_alloc_allocatable_handle:
  case LibFunc_for_allocate:
  case LibFunc_for_allocate_handle:
  case LibFunc_for_array_copy_in:
  case LibFunc_for_array_copy_out:
  case LibFunc_for_backspace:
  case LibFunc_for_check_mult_overflow64:
  case LibFunc_for_close:
  case LibFunc_for_concat:
  case LibFunc_for_contig_array:
  case LibFunc_for_cpstr:
  case LibFunc_for_cpystr:
  case LibFunc_for_date_and_time:
  case LibFunc_for_dealloc_allocatable:
  case LibFunc_for_dealloc_allocatable_handle:
  case LibFunc_for_deallocate:
  case LibFunc_for_endfile:
  case LibFunc_for_exponent8_v:
  case LibFunc_for_f90_index:
  case LibFunc_for_f90_scan:
  case LibFunc_for_f90_verify:
  case LibFunc_for_fraction8_v:
  case LibFunc_for_getcmd_arg_err:
  case LibFunc_for_iargc:
  case LibFunc_for_inquire:
  case LibFunc_for_len_trim:
  case LibFunc_for_open:
  case LibFunc_for_random_number:
  case LibFunc_for_random_seed_bit_size:
  case LibFunc_for_random_seed_put:
  case LibFunc_for_read_int_fmt:
  case LibFunc_for_read_seq_fmt:
  case LibFunc_for_read_seq_lis:
  case LibFunc_for_read_seq_lis_xmit:
  case LibFunc_for_read_seq_nml:
  case LibFunc_for_realloc_lhs:
  case LibFunc_for_rewind:
  case LibFunc_for_scale8_v:
  case LibFunc_for_set_reentrancy:
  case LibFunc_for_setexp8_v:
  case LibFunc_for_stop_core_quiet:
  case LibFunc_for_system_clock_count:
  case LibFunc_for_trim:
  case LibFunc_for_write_int_fmt:
  case LibFunc_for_write_int_fmt_xmit:
  case LibFunc_for_write_int_lis:
  case LibFunc_for_write_int_lis_xmit:
  case LibFunc_for_write_seq:
  case LibFunc_for_write_seq_fmt:
  case LibFunc_for_write_seq_fmt_xmit:
  case LibFunc_for_write_seq_lis:
  case LibFunc_for_write_seq_lis_xmit:
  case LibFunc_for_write_seq_xmit:
    Changed |= setFortran(F);
    return Changed;
  case LibFunc_fork:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_freopen:
  case LibFunc_freopen64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_fsync:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ftruncate64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getcwd:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getegid:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_geteuid:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getgid:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getopt_long:
  case LibFunc_getopt_long_only:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_getpid:
  case LibFunc_getpid:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getpwuid:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getrlimit:
  case LibFunc_getrlimit64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getrusage:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_FindClose:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_FindFirstFileA:
  case LibFunc_FindFirstFileW:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_FindNextFileA:
  case LibFunc_FindNextFileW:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_FindResourceA:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_FormatMessageA:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_FreeResource:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetCurrentDirectoryA:
  case LibFunc_GetCurrentDirectoryW:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetCurrentProcess:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetCurrentThreadId:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetFullPathNameA:
  case LibFunc_GetFullPathNameW:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetLastError:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetModuleFileNameA:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetModuleHandleA:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetProcAddress:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetProcessTimes:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetShortPathNameW:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetSystemTime:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GetVersionExA:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_GlobalMemoryStatus:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getuid:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_glob:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_globfree:
    return Changed;
  case LibFunc_gmtime:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_gmtime_r:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_hypot:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_hypotf:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_InitializeCriticalSection:
  case LibFunc_InitializeCriticalSectionAndSpinCount:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ioctl:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_isalnum:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_isalpha:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_isatty:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_iscntrl:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_islower:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_isprint:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_isspace:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_isupper:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_iswspace:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_isxdigit:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_j0:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_j1:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kill:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmp_set_blocktime:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_LeaveCriticalSection:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_link:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_LoadLibraryA:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_LoadResource:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_LocalFree:
     Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_LockResource:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_local_stdio_printf_options:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_local_stdio_scanf_options:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_localeconv:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_localtime:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_localtime_r:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_longjmp:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_lseek:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_lseek64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_mallopt:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_mblen:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_mbstowcs:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_mkdtemp:
  case LibFunc_mkstemps:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_mmap:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_MultiByteToWideChar:
  case LibFunc_WideCharToMultiByte:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_munmap:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_obstack_free:
    return Changed;
  case LibFunc_omp_destroy_lock:
  case LibFunc_omp_destroy_nest_lock:
    return Changed;
  case LibFunc_omp_get_active_level:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_ancestor_thread_num:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_cancellation:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_default_device:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_dynamic:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_initial_device:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_level:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_max_active_levels:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_max_task_priority:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_max_threads:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_nested:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_num_devices:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_num_procs:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_num_teams:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_num_threads:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_proc_bind:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_schedule:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_omp_get_team_num:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_team_size:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_thread_limit:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_thread_num:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_wtick:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_wtime:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_in_final:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_in_parallel:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_is_initial_device:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_init_lock:
  case LibFunc_omp_init_lock_with_hint:
  case LibFunc_omp_init_nest_lock:
  case LibFunc_omp_init_nest_lock_with_hint:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_lock:
  case LibFunc_omp_set_nest_lock:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_test_lock:
  case LibFunc_omp_test_nest_lock:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_unset_lock:
  case LibFunc_omp_unset_nest_lock:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_default_device:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_dynamic:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_max_active_levels:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_num_threads:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_nested:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_schedule:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_pipe:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_pthread_key_create:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_pthread_self:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_putenv:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_qsort_r:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_QueryPerformanceCounter:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_raise:
    return Changed;
  case LibFunc_rand:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_readdir:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_readdir64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_scandir:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_select:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_setgid:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_setlocale:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_setrlimit:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_setuid:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_siglongjmp:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_signal:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_signbit:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ReadFile:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_SetFilePointer:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_SizeofResource:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_Sleep:
  case LibFunc_under_sleep:
  case LibFunc_sleep:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_SwitchToFiber:
    return Changed;
  case LibFunc_srand:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_stdio_common_vfprintf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setDoesNotCapture(F, 3);
    Changed |= setOnlyReadsMemory(F, 2);
    return Changed;
  case LibFunc_stdio_common_vfscanf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setDoesNotCapture(F, 3);
    Changed |= setOnlyReadsMemory(F, 2);
    return Changed;
  case LibFunc_stdio_common_vsprintf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setDoesNotCapture(F, 3);
    Changed |= setOnlyReadsMemory(F, 3);
    return Changed;
  // NOTE: sprintf_s in Windows is a wrapper to
  // __stdio_common_vsprintf_s
  case LibFunc_sprintf_s:
  case LibFunc_dunder_stdio_common_vsprintf_s:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setDoesNotCapture(F, 3);
    Changed |= setOnlyReadsMemory(F, 3);
    return Changed;
  case LibFunc_stdio_common_vsscanf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setDoesNotCapture(F, 3);
    Changed |= setOnlyReadsMemory(F, 3);
    return Changed;
  case LibFunc_strerror:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_strftime:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_strsignal:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_symlink:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_sysconf:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_SystemTimeToFileTime:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_terminate:
    Changed |= setDoesNotReturn(F);
    return Changed;
  case LibFunc_time:
    Changed |= setDoesNotThrow(F);
    return Changed;
  // _ftime64 validates the parameters, it can throw
  // an exception
  case LibFunc_under_ftime64:
    return Changed;
  case LibFunc_under_time64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_unlock_file:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_wassert:
    Changed |= setDoesNotReturn(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_wfopen:
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_under_wopen:
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  // _write can throw an exception in Windows
  case LibFunc_under_write:
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_tolower:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_toupper:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_towlower:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_towupper:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_truncate64:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_usleep:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_vasprintf:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_waitpid:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_wcstombs:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_WriteFile:
    Changed |= setDoesNotThrow(F);
    return Changed;
#endif //INTEL_CUSTOMIZATION

  case LibFunc_ldexp:
  case LibFunc_ldexpf:
  case LibFunc_ldexpl:
    Changed |= setWillReturn(F);
    if (!F.onlyReadsMemory() && isAllUsersFast(F)) // INTEL
      Changed |= setOnlyReadsMemory(F);            // INTEL
    return Changed;
  case LibFunc_abs:
  case LibFunc_acos:
  case LibFunc_acosf:
  case LibFunc_acosh:
  case LibFunc_acoshf:
  case LibFunc_acoshl:
  case LibFunc_acosl:
  case LibFunc_asin:
  case LibFunc_asinf:
  case LibFunc_asinh:
  case LibFunc_asinhf:
  case LibFunc_asinhl:
  case LibFunc_asinl:
  case LibFunc_atan:
  case LibFunc_atan2:
  case LibFunc_atan2f:
  case LibFunc_atan2l:
  case LibFunc_atanf:
  case LibFunc_atanh:
  case LibFunc_atanhf:
  case LibFunc_atanhl:
  case LibFunc_atanl:
  case LibFunc_cbrt:
  case LibFunc_cbrtf:
  case LibFunc_cbrtl:
  case LibFunc_ceil:
  case LibFunc_ceilf:
  case LibFunc_ceill:
  case LibFunc_copysign:
  case LibFunc_copysignf:
  case LibFunc_copysignl:
  case LibFunc_cos:
  case LibFunc_cosh:
  case LibFunc_coshf:
  case LibFunc_coshl:
  case LibFunc_cosf:
  case LibFunc_cosl:
  case LibFunc_cospi:
  case LibFunc_cospif:
  case LibFunc_exp:
  case LibFunc_expf:
  case LibFunc_expl:
  case LibFunc_exp2:
  case LibFunc_exp2f:
  case LibFunc_exp2l:
  case LibFunc_expm1:
  case LibFunc_expm1f:
  case LibFunc_expm1l:
  case LibFunc_fabs:
  case LibFunc_fabsf:
  case LibFunc_fabsl:
  case LibFunc_ffs:
  case LibFunc_ffsl:
  case LibFunc_ffsll:
  case LibFunc_floor:
  case LibFunc_floorf:
  case LibFunc_floorl:
  case LibFunc_fls:
  case LibFunc_flsl:
  case LibFunc_flsll:
  case LibFunc_fmax:
  case LibFunc_fmaxf:
  case LibFunc_fmaxl:
  case LibFunc_fmin:
  case LibFunc_fminf:
  case LibFunc_fminl:
  case LibFunc_fmod:
  case LibFunc_fmodf:
  case LibFunc_fmodl:
  case LibFunc_isascii:
  case LibFunc_isdigit:
  case LibFunc_labs:
  case LibFunc_llabs:
  case LibFunc_log:
  case LibFunc_log10:
  case LibFunc_log10f:
  case LibFunc_log10l:
  case LibFunc_log1p:
  case LibFunc_log1pf:
  case LibFunc_log1pl:
  case LibFunc_log2:
  case LibFunc_log2f:
  case LibFunc_log2l:
  case LibFunc_logb:
  case LibFunc_logbf:
  case LibFunc_logbl:
  case LibFunc_logf:
  case LibFunc_logl:
  case LibFunc_nearbyint:
  case LibFunc_nearbyintf:
  case LibFunc_nearbyintl:
  case LibFunc_pow:
  case LibFunc_powf:
  case LibFunc_powl:
  case LibFunc_rint:
  case LibFunc_rintf:
  case LibFunc_rintl:
  case LibFunc_round:
  case LibFunc_roundf:
  case LibFunc_roundl:
  case LibFunc_sin:
  case LibFunc_sincospif_stret:
  case LibFunc_sinf:
  case LibFunc_sinh:
  case LibFunc_sinhf:
  case LibFunc_sinhl:
  case LibFunc_sinl:
  case LibFunc_sinpi:
  case LibFunc_sinpif:
  case LibFunc_sqrt:
  case LibFunc_sqrtf:
  case LibFunc_sqrtl:
  case LibFunc_tan:
  case LibFunc_tanf:
  case LibFunc_tanh:
  case LibFunc_tanhf:
  case LibFunc_tanhl:
  case LibFunc_tanl:
  case LibFunc_toascii:
  case LibFunc_trunc:
  case LibFunc_truncf:
  case LibFunc_truncl:
#if INTEL_CUSTOMIZATION
  case LibFunc_dunder_powi4i4:
  case LibFunc_dunder_powr8i8:
#endif //INTEL_CUSTOMIZATION
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotFreeMemory(F);
    if (!F.onlyReadsMemory() && isAllUsersFast(F)) // INTEL
      Changed |= setOnlyReadsMemory(F);            // INTEL
    Changed |= setOnlyWritesMemory(F);
    Changed |= setWillReturn(F);
    return Changed;
  default:
    // FIXME: It'd be really nice to cover all the library functions we're
    // aware of here.
    return false;
  }
}

static void setArgExtAttr(Function &F, unsigned ArgNo,
                          const TargetLibraryInfo &TLI, bool Signed = true) {
  Attribute::AttrKind ExtAttr = TLI.getExtAttrForI32Param(Signed);
  if (ExtAttr != Attribute::None && !F.hasParamAttribute(ArgNo, ExtAttr))
    F.addParamAttr(ArgNo, ExtAttr);
}

// Modeled after X86TargetLowering::markLibCallAttributes.
static void markRegisterParameterAttributes(Function *F) {
  if (!F->arg_size() || F->isVarArg())
    return;

  const CallingConv::ID CC = F->getCallingConv();
  if (CC != CallingConv::C && CC != CallingConv::X86_StdCall)
    return;

  const Module *M = F->getParent();
  unsigned N = M->getNumberRegisterParameters();
  if (!N)
    return;

  const DataLayout &DL = M->getDataLayout();

  for (Argument &A : F->args()) {
    Type *T = A.getType();
    if (!T->isIntOrPtrTy())
      continue;

    const TypeSize &TS = DL.getTypeAllocSize(T);
    if (TS > 8)
      continue;

    assert(TS <= 4 && "Need to account for parameters larger than word size");
    const unsigned NumRegs = TS > 4 ? 2 : 1;
    if (N < NumRegs)
      return;

    N -= NumRegs;
    F->addParamAttr(A.getArgNo(), Attribute::InReg);
  }
}

FunctionCallee llvm::getOrInsertLibFunc(Module *M, const TargetLibraryInfo &TLI,
                                        LibFunc TheLibFunc, FunctionType *T,
                                        AttributeList AttributeList) {
  assert(TLI.has(TheLibFunc) &&
         "Creating call to non-existing library function.");
  StringRef Name = TLI.getName(TheLibFunc);
  FunctionCallee C = M->getOrInsertFunction(Name, T, AttributeList);

  // Make sure any mandatory argument attributes are added.

  // Any outgoing i32 argument should be handled with setArgExtAttr() which
  // will add an extension attribute if the target ABI requires it. Adding
  // argument extensions is typically done by the front end but when an
  // optimizer is building a library call on its own it has to take care of
  // this. Each such generated function must be handled here with sign or
  // zero extensions as needed.  F is retreived with cast<> because we demand
  // of the caller to have called isLibFuncEmittable() first.
  Function *F = cast<Function>(C.getCallee());
  assert(F->getFunctionType() == T && "Function type does not match.");
  switch (TheLibFunc) {
  case LibFunc_fputc:
  case LibFunc_putchar:
    setArgExtAttr(*F, 0, TLI);
    break;
  case LibFunc_ldexp:
  case LibFunc_ldexpf:
  case LibFunc_ldexpl:
  case LibFunc_memchr:
  case LibFunc_memrchr:
  case LibFunc_strchr:
    setArgExtAttr(*F, 1, TLI);
    break;
  case LibFunc_memccpy:
    setArgExtAttr(*F, 2, TLI);
    break;

    // These are functions that are known to not need any argument extension
    // on any target: A size_t argument (which may be an i32 on some targets)
    // should not trigger the assert below.
  case LibFunc_bcmp:
  case LibFunc_calloc:
  case LibFunc_fwrite:
  case LibFunc_malloc:
  case LibFunc_memcmp:
  case LibFunc_memcpy_chk:
  case LibFunc_mempcpy:
  case LibFunc_memset_pattern16:
  case LibFunc_snprintf:
  case LibFunc_stpncpy:
  case LibFunc_strlcat:
  case LibFunc_strlcpy:
  case LibFunc_strncat:
  case LibFunc_strncmp:
  case LibFunc_strncpy:
  case LibFunc_vsnprintf:
    break;

  default:
#ifndef NDEBUG
    for (unsigned i = 0; i < T->getNumParams(); i++)
      assert(!isa<IntegerType>(T->getParamType(i)) &&
             "Unhandled integer argument.");
#endif
    break;
  }

  markRegisterParameterAttributes(F);

  return C;
}

FunctionCallee llvm::getOrInsertLibFunc(Module *M, const TargetLibraryInfo &TLI,
                                        LibFunc TheLibFunc, FunctionType *T) {
  return getOrInsertLibFunc(M, TLI, TheLibFunc, T, AttributeList());
}

bool llvm::isLibFuncEmittable(const Module *M, const TargetLibraryInfo *TLI,
                              LibFunc TheLibFunc) {
  StringRef FuncName = TLI->getName(TheLibFunc);
  if (!TLI->has(TheLibFunc))
    return false;

  // Check if the Module already has a GlobalValue with the same name, in
  // which case it must be a Function with the expected type.
  if (GlobalValue *GV = M->getNamedValue(FuncName)) {
    if (auto *F = dyn_cast<Function>(GV))
      return TLI->isValidProtoForLibFunc(*F->getFunctionType(), TheLibFunc, *M);
    return false;
  }

  return true;
}

bool llvm::isLibFuncEmittable(const Module *M, const TargetLibraryInfo *TLI,
                              StringRef Name) {
  LibFunc TheLibFunc;
  return TLI->getLibFunc(Name, TheLibFunc) &&
         isLibFuncEmittable(M, TLI, TheLibFunc);
}

bool llvm::hasFloatFn(const Module *M, const TargetLibraryInfo *TLI, Type *Ty,
                      LibFunc DoubleFn, LibFunc FloatFn, LibFunc LongDoubleFn) {
  switch (Ty->getTypeID()) {
  case Type::HalfTyID:
    return false;
  case Type::FloatTyID:
    return isLibFuncEmittable(M, TLI, FloatFn);
  case Type::DoubleTyID:
    return isLibFuncEmittable(M, TLI, DoubleFn);
  default:
    return isLibFuncEmittable(M, TLI, LongDoubleFn);
  }
}

StringRef llvm::getFloatFn(const Module *M, const TargetLibraryInfo *TLI,
                           Type *Ty, LibFunc DoubleFn, LibFunc FloatFn,
                           LibFunc LongDoubleFn, LibFunc &TheLibFunc) {
  assert(hasFloatFn(M, TLI, Ty, DoubleFn, FloatFn, LongDoubleFn) &&
         "Cannot get name for unavailable function!");

  switch (Ty->getTypeID()) {
  case Type::HalfTyID:
    llvm_unreachable("No name for HalfTy!");
  case Type::FloatTyID:
    TheLibFunc = FloatFn;
    return TLI->getName(FloatFn);
  case Type::DoubleTyID:
    TheLibFunc = DoubleFn;
    return TLI->getName(DoubleFn);
  default:
    TheLibFunc = LongDoubleFn;
    return TLI->getName(LongDoubleFn);
  }
}

//- Emit LibCalls ------------------------------------------------------------//

Value *llvm::castToCStr(Value *V, IRBuilderBase &B) {
  unsigned AS = V->getType()->getPointerAddressSpace();
  return B.CreateBitCast(V, B.getInt8PtrTy(AS), "cstr");
}

static Value *emitLibCall(LibFunc TheLibFunc, Type *ReturnType,
                          ArrayRef<Type *> ParamTypes,
                          ArrayRef<Value *> Operands, IRBuilderBase &B,
                          const TargetLibraryInfo *TLI,
                          bool IsVaArgs = false) {
  Module *M = B.GetInsertBlock()->getModule();
  if (!isLibFuncEmittable(M, TLI, TheLibFunc))
    return nullptr;

  StringRef FuncName = TLI->getName(TheLibFunc);
  FunctionType *FuncType = FunctionType::get(ReturnType, ParamTypes, IsVaArgs);
  FunctionCallee Callee = getOrInsertLibFunc(M, *TLI, TheLibFunc, FuncType);
  inferNonMandatoryLibFuncAttrs(M, FuncName, *TLI);
  CallInst *CI = B.CreateCall(Callee, Operands, FuncName);
  if (const Function *F =
          dyn_cast<Function>(Callee.getCallee()->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitStrLen(Value *Ptr, IRBuilderBase &B, const DataLayout &DL,
                        const TargetLibraryInfo *TLI) {
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  return emitLibCall(LibFunc_strlen, DL.getIntPtrType(Context),
                     B.getInt8PtrTy(), castToCStr(Ptr, B), B, TLI);
}

Value *llvm::emitStrDup(Value *Ptr, IRBuilderBase &B,
                        const TargetLibraryInfo *TLI) {
  return emitLibCall(LibFunc_strdup, B.getInt8PtrTy(), B.getInt8PtrTy(),
                     castToCStr(Ptr, B), B, TLI);
}

Value *llvm::emitStrChr(Value *Ptr, char C, IRBuilderBase &B,
                        const TargetLibraryInfo *TLI) {
  Type *I8Ptr = B.getInt8PtrTy();
  Type *I32Ty = B.getInt32Ty();
  return emitLibCall(LibFunc_strchr, I8Ptr, {I8Ptr, I32Ty},
                     {castToCStr(Ptr, B), ConstantInt::get(I32Ty, C)}, B, TLI);
}

Value *llvm::emitStrNCmp(Value *Ptr1, Value *Ptr2, Value *Len, IRBuilderBase &B,
                         const DataLayout &DL, const TargetLibraryInfo *TLI) {
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  return emitLibCall(
      LibFunc_strncmp, B.getInt32Ty(),
      {B.getInt8PtrTy(), B.getInt8PtrTy(), DL.getIntPtrType(Context)},
      {castToCStr(Ptr1, B), castToCStr(Ptr2, B), Len}, B, TLI);
}

Value *llvm::emitStrCpy(Value *Dst, Value *Src, IRBuilderBase &B,
                        const TargetLibraryInfo *TLI) {
  Type *I8Ptr = Dst->getType();
  return emitLibCall(LibFunc_strcpy, I8Ptr, {I8Ptr, I8Ptr},
                     {castToCStr(Dst, B), castToCStr(Src, B)}, B, TLI);
}

Value *llvm::emitStpCpy(Value *Dst, Value *Src, IRBuilderBase &B,
                        const TargetLibraryInfo *TLI) {
  Type *I8Ptr = B.getInt8PtrTy();
  return emitLibCall(LibFunc_stpcpy, I8Ptr, {I8Ptr, I8Ptr},
                     {castToCStr(Dst, B), castToCStr(Src, B)}, B, TLI);
}

Value *llvm::emitStrNCpy(Value *Dst, Value *Src, Value *Len, IRBuilderBase &B,
                         const TargetLibraryInfo *TLI) {
  Type *I8Ptr = B.getInt8PtrTy();
  return emitLibCall(LibFunc_strncpy, I8Ptr, {I8Ptr, I8Ptr, Len->getType()},
                     {castToCStr(Dst, B), castToCStr(Src, B), Len}, B, TLI);
}

Value *llvm::emitStpNCpy(Value *Dst, Value *Src, Value *Len, IRBuilderBase &B,
                         const TargetLibraryInfo *TLI) {
  Type *I8Ptr = B.getInt8PtrTy();
  return emitLibCall(LibFunc_stpncpy, I8Ptr, {I8Ptr, I8Ptr, Len->getType()},
                     {castToCStr(Dst, B), castToCStr(Src, B), Len}, B, TLI);
}

Value *llvm::emitMemCpyChk(Value *Dst, Value *Src, Value *Len, Value *ObjSize,
                           IRBuilderBase &B, const DataLayout &DL,
                           const TargetLibraryInfo *TLI) {
  Module *M = B.GetInsertBlock()->getModule();
  if (!isLibFuncEmittable(M, TLI, LibFunc_memcpy_chk))
    return nullptr;

  AttributeList AS;
  AS = AttributeList::get(M->getContext(), AttributeList::FunctionIndex,
                          Attribute::NoUnwind);
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  FunctionCallee MemCpy = getOrInsertLibFunc(M, *TLI, LibFunc_memcpy_chk,
      AttributeList::get(M->getContext(), AS), B.getInt8PtrTy(),
      B.getInt8PtrTy(), B.getInt8PtrTy(), DL.getIntPtrType(Context),
      DL.getIntPtrType(Context));
  Dst = castToCStr(Dst, B);
  Src = castToCStr(Src, B);
  CallInst *CI = B.CreateCall(MemCpy, {Dst, Src, Len, ObjSize});
  if (const Function *F =
          dyn_cast<Function>(MemCpy.getCallee()->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitMemPCpy(Value *Dst, Value *Src, Value *Len, IRBuilderBase &B,
                         const DataLayout &DL, const TargetLibraryInfo *TLI) {
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  return emitLibCall(
      LibFunc_mempcpy, B.getInt8PtrTy(),
      {B.getInt8PtrTy(), B.getInt8PtrTy(), DL.getIntPtrType(Context)},
      {Dst, Src, Len}, B, TLI);
}

Value *llvm::emitMemChr(Value *Ptr, Value *Val, Value *Len, IRBuilderBase &B,
                        const DataLayout &DL, const TargetLibraryInfo *TLI) {
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  return emitLibCall(
      LibFunc_memchr, B.getInt8PtrTy(),
      {B.getInt8PtrTy(), B.getInt32Ty(), DL.getIntPtrType(Context)},
      {castToCStr(Ptr, B), Val, Len}, B, TLI);
}

Value *llvm::emitMemRChr(Value *Ptr, Value *Val, Value *Len, IRBuilderBase &B,
                        const DataLayout &DL, const TargetLibraryInfo *TLI) {
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  return emitLibCall(
      LibFunc_memrchr, B.getInt8PtrTy(),
      {B.getInt8PtrTy(), B.getInt32Ty(), DL.getIntPtrType(Context)},
      {castToCStr(Ptr, B), Val, Len}, B, TLI);
}

Value *llvm::emitMemCmp(Value *Ptr1, Value *Ptr2, Value *Len, IRBuilderBase &B,
                        const DataLayout &DL, const TargetLibraryInfo *TLI) {
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  return emitLibCall(
      LibFunc_memcmp, B.getInt32Ty(),
      {B.getInt8PtrTy(), B.getInt8PtrTy(), DL.getIntPtrType(Context)},
      {castToCStr(Ptr1, B), castToCStr(Ptr2, B), Len}, B, TLI);
}

Value *llvm::emitBCmp(Value *Ptr1, Value *Ptr2, Value *Len, IRBuilderBase &B,
                      const DataLayout &DL, const TargetLibraryInfo *TLI) {
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  return emitLibCall(
      LibFunc_bcmp, B.getInt32Ty(),
      {B.getInt8PtrTy(), B.getInt8PtrTy(), DL.getIntPtrType(Context)},
      {castToCStr(Ptr1, B), castToCStr(Ptr2, B), Len}, B, TLI);
}

Value *llvm::emitMemCCpy(Value *Ptr1, Value *Ptr2, Value *Val, Value *Len,
                         IRBuilderBase &B, const TargetLibraryInfo *TLI) {
  return emitLibCall(
      LibFunc_memccpy, B.getInt8PtrTy(),
      {B.getInt8PtrTy(), B.getInt8PtrTy(), B.getInt32Ty(), Len->getType()},
      {Ptr1, Ptr2, Val, Len}, B, TLI);
}

Value *llvm::emitSNPrintf(Value *Dest, Value *Size, Value *Fmt,
                          ArrayRef<Value *> VariadicArgs, IRBuilderBase &B,
                          const TargetLibraryInfo *TLI) {
  SmallVector<Value *, 8> Args{castToCStr(Dest, B), Size, castToCStr(Fmt, B)};
  llvm::append_range(Args, VariadicArgs);
  return emitLibCall(LibFunc_snprintf, B.getInt32Ty(),
                     {B.getInt8PtrTy(), Size->getType(), B.getInt8PtrTy()},
                     Args, B, TLI, /*IsVaArgs=*/true);
}

Value *llvm::emitSPrintf(Value *Dest, Value *Fmt,
                         ArrayRef<Value *> VariadicArgs, IRBuilderBase &B,
                         const TargetLibraryInfo *TLI) {
  SmallVector<Value *, 8> Args{castToCStr(Dest, B), castToCStr(Fmt, B)};
  llvm::append_range(Args, VariadicArgs);
  return emitLibCall(LibFunc_sprintf, B.getInt32Ty(),
                     {B.getInt8PtrTy(), B.getInt8PtrTy()}, Args, B, TLI,
                     /*IsVaArgs=*/true);
}

Value *llvm::emitStrCat(Value *Dest, Value *Src, IRBuilderBase &B,
                        const TargetLibraryInfo *TLI) {
  return emitLibCall(LibFunc_strcat, B.getInt8PtrTy(),
                     {B.getInt8PtrTy(), B.getInt8PtrTy()},
                     {castToCStr(Dest, B), castToCStr(Src, B)}, B, TLI);
}

Value *llvm::emitStrLCpy(Value *Dest, Value *Src, Value *Size, IRBuilderBase &B,
                         const TargetLibraryInfo *TLI) {
  return emitLibCall(LibFunc_strlcpy, Size->getType(),
                     {B.getInt8PtrTy(), B.getInt8PtrTy(), Size->getType()},
                     {castToCStr(Dest, B), castToCStr(Src, B), Size}, B, TLI);
}

Value *llvm::emitStrLCat(Value *Dest, Value *Src, Value *Size, IRBuilderBase &B,
                         const TargetLibraryInfo *TLI) {
  return emitLibCall(LibFunc_strlcat, Size->getType(),
                     {B.getInt8PtrTy(), B.getInt8PtrTy(), Size->getType()},
                     {castToCStr(Dest, B), castToCStr(Src, B), Size}, B, TLI);
}

Value *llvm::emitStrNCat(Value *Dest, Value *Src, Value *Size, IRBuilderBase &B,
                         const TargetLibraryInfo *TLI) {
  return emitLibCall(LibFunc_strncat, B.getInt8PtrTy(),
                     {B.getInt8PtrTy(), B.getInt8PtrTy(), Size->getType()},
                     {castToCStr(Dest, B), castToCStr(Src, B), Size}, B, TLI);
}

Value *llvm::emitVSNPrintf(Value *Dest, Value *Size, Value *Fmt, Value *VAList,
                           IRBuilderBase &B, const TargetLibraryInfo *TLI) {
  return emitLibCall(
      LibFunc_vsnprintf, B.getInt32Ty(),
      {B.getInt8PtrTy(), Size->getType(), B.getInt8PtrTy(), VAList->getType()},
      {castToCStr(Dest, B), Size, castToCStr(Fmt, B), VAList}, B, TLI);
}

Value *llvm::emitVSPrintf(Value *Dest, Value *Fmt, Value *VAList,
                          IRBuilderBase &B, const TargetLibraryInfo *TLI) {
  return emitLibCall(LibFunc_vsprintf, B.getInt32Ty(),
                     {B.getInt8PtrTy(), B.getInt8PtrTy(), VAList->getType()},
                     {castToCStr(Dest, B), castToCStr(Fmt, B), VAList}, B, TLI);
}

/// Append a suffix to the function name according to the type of 'Op'.
static void appendTypeSuffix(Value *Op, StringRef &Name,
                             SmallString<20> &NameBuffer) {
  if (!Op->getType()->isDoubleTy()) {
      NameBuffer += Name;

    if (Op->getType()->isFloatTy())
      NameBuffer += 'f';
    else
      NameBuffer += 'l';

    Name = NameBuffer;
  }
}

static Value *emitUnaryFloatFnCallHelper(Value *Op, LibFunc TheLibFunc,
                                         StringRef Name, IRBuilderBase &B,
                                         const AttributeList &Attrs,
                                         const TargetLibraryInfo *TLI) {
  assert((Name != "") && "Must specify Name to emitUnaryFloatFnCall");

  Module *M = B.GetInsertBlock()->getModule();
  FunctionCallee Callee = getOrInsertLibFunc(M, *TLI, TheLibFunc, Op->getType(),
                                             Op->getType());
  CallInst *CI = B.CreateCall(Callee, Op, Name);

  // The incoming attribute set may have come from a speculatable intrinsic, but
  // is being replaced with a library call which is not allowed to be
  // speculatable.
  CI->setAttributes(
      Attrs.removeFnAttribute(B.getContext(), Attribute::Speculatable));
  if (const Function *F =
          dyn_cast<Function>(Callee.getCallee()->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitUnaryFloatFnCall(Value *Op, const TargetLibraryInfo *TLI,
                                  StringRef Name, IRBuilderBase &B,
                                  const AttributeList &Attrs) {
  SmallString<20> NameBuffer;
  appendTypeSuffix(Op, Name, NameBuffer);

  LibFunc TheLibFunc;
  TLI->getLibFunc(Name, TheLibFunc);

  return emitUnaryFloatFnCallHelper(Op, TheLibFunc, Name, B, Attrs, TLI);
}

Value *llvm::emitUnaryFloatFnCall(Value *Op, const TargetLibraryInfo *TLI,
                                  LibFunc DoubleFn, LibFunc FloatFn,
                                  LibFunc LongDoubleFn, IRBuilderBase &B,
                                  const AttributeList &Attrs) {
  // Get the name of the function according to TLI.
  Module *M = B.GetInsertBlock()->getModule();
  LibFunc TheLibFunc;
  StringRef Name = getFloatFn(M, TLI, Op->getType(), DoubleFn, FloatFn,
                              LongDoubleFn, TheLibFunc);

  return emitUnaryFloatFnCallHelper(Op, TheLibFunc, Name, B, Attrs, TLI);
}

static Value *emitBinaryFloatFnCallHelper(Value *Op1, Value *Op2,
                                          LibFunc TheLibFunc,
                                          StringRef Name, IRBuilderBase &B,
                                          const AttributeList &Attrs,
                                          const TargetLibraryInfo *TLI) {
  assert((Name != "") && "Must specify Name to emitBinaryFloatFnCall");

  Module *M = B.GetInsertBlock()->getModule();
  FunctionCallee Callee = getOrInsertLibFunc(M, *TLI, TheLibFunc, Op1->getType(),
                                             Op1->getType(), Op2->getType());
  inferNonMandatoryLibFuncAttrs(M, Name, *TLI);
  CallInst *CI = B.CreateCall(Callee, { Op1, Op2 }, Name);

  // The incoming attribute set may have come from a speculatable intrinsic, but
  // is being replaced with a library call which is not allowed to be
  // speculatable.
  CI->setAttributes(
      Attrs.removeFnAttribute(B.getContext(), Attribute::Speculatable));
  if (const Function *F =
          dyn_cast<Function>(Callee.getCallee()->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitBinaryFloatFnCall(Value *Op1, Value *Op2,
                                   const TargetLibraryInfo *TLI,
                                   StringRef Name, IRBuilderBase &B,
                                   const AttributeList &Attrs) {
  assert((Name != "") && "Must specify Name to emitBinaryFloatFnCall");

  SmallString<20> NameBuffer;
  appendTypeSuffix(Op1, Name, NameBuffer);

  LibFunc TheLibFunc;
  TLI->getLibFunc(Name, TheLibFunc);

  return emitBinaryFloatFnCallHelper(Op1, Op2, TheLibFunc, Name, B, Attrs, TLI);
}

Value *llvm::emitBinaryFloatFnCall(Value *Op1, Value *Op2,
                                   const TargetLibraryInfo *TLI,
                                   LibFunc DoubleFn, LibFunc FloatFn,
                                   LibFunc LongDoubleFn, IRBuilderBase &B,
                                   const AttributeList &Attrs) {
  // Get the name of the function according to TLI.
  Module *M = B.GetInsertBlock()->getModule();
  LibFunc TheLibFunc;
  StringRef Name = getFloatFn(M, TLI, Op1->getType(), DoubleFn, FloatFn,
                              LongDoubleFn, TheLibFunc);

  return emitBinaryFloatFnCallHelper(Op1, Op2, TheLibFunc, Name, B, Attrs, TLI);
}

Value *llvm::emitPutChar(Value *Char, IRBuilderBase &B,
                         const TargetLibraryInfo *TLI) {
  Module *M = B.GetInsertBlock()->getModule();
  if (!isLibFuncEmittable(M, TLI, LibFunc_putchar))
    return nullptr;

  StringRef PutCharName = TLI->getName(LibFunc_putchar);
  FunctionCallee PutChar = getOrInsertLibFunc(M, *TLI, LibFunc_putchar,
                                              B.getInt32Ty(), B.getInt32Ty());
  inferNonMandatoryLibFuncAttrs(M, PutCharName, *TLI);
  CallInst *CI = B.CreateCall(PutChar,
                              B.CreateIntCast(Char,
                              B.getInt32Ty(),
                              /*isSigned*/true,
                              "chari"),
                              PutCharName);

  if (const Function *F =
          dyn_cast<Function>(PutChar.getCallee()->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitPutS(Value *Str, IRBuilderBase &B,
                      const TargetLibraryInfo *TLI) {
  Module *M = B.GetInsertBlock()->getModule();
  if (!isLibFuncEmittable(M, TLI, LibFunc_puts))
    return nullptr;

  StringRef PutsName = TLI->getName(LibFunc_puts);
  FunctionCallee PutS = getOrInsertLibFunc(M, *TLI, LibFunc_puts, B.getInt32Ty(),
                                           B.getInt8PtrTy());
  inferNonMandatoryLibFuncAttrs(M, PutsName, *TLI);
  CallInst *CI = B.CreateCall(PutS, castToCStr(Str, B), PutsName);
  if (const Function *F =
          dyn_cast<Function>(PutS.getCallee()->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitFPutC(Value *Char, Value *File, IRBuilderBase &B,
                       const TargetLibraryInfo *TLI) {
  Module *M = B.GetInsertBlock()->getModule();
  if (!isLibFuncEmittable(M, TLI, LibFunc_fputc))
    return nullptr;

  StringRef FPutcName = TLI->getName(LibFunc_fputc);
  FunctionCallee F = getOrInsertLibFunc(M, *TLI, LibFunc_fputc, B.getInt32Ty(),
                                        B.getInt32Ty(), File->getType());
  if (File->getType()->isPointerTy())
    inferNonMandatoryLibFuncAttrs(M, FPutcName, *TLI);
  Char = B.CreateIntCast(Char, B.getInt32Ty(), /*isSigned*/true,
                         "chari");
  CallInst *CI = B.CreateCall(F, {Char, File}, FPutcName);

  if (const Function *Fn =
          dyn_cast<Function>(F.getCallee()->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitFPutS(Value *Str, Value *File, IRBuilderBase &B,
                       const TargetLibraryInfo *TLI) {
  Module *M = B.GetInsertBlock()->getModule();
  if (!isLibFuncEmittable(M, TLI, LibFunc_fputs))
    return nullptr;

  StringRef FPutsName = TLI->getName(LibFunc_fputs);
  FunctionCallee F = getOrInsertLibFunc(M, *TLI, LibFunc_fputs, B.getInt32Ty(),
                                        B.getInt8PtrTy(), File->getType());
  if (File->getType()->isPointerTy())
    inferNonMandatoryLibFuncAttrs(M, FPutsName, *TLI);
  CallInst *CI = B.CreateCall(F, {castToCStr(Str, B), File}, FPutsName);

  if (const Function *Fn =
          dyn_cast<Function>(F.getCallee()->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitFWrite(Value *Ptr, Value *Size, Value *File, IRBuilderBase &B,
                        const DataLayout &DL, const TargetLibraryInfo *TLI) {
  Module *M = B.GetInsertBlock()->getModule();
  if (!isLibFuncEmittable(M, TLI, LibFunc_fwrite))
    return nullptr;

  LLVMContext &Context = B.GetInsertBlock()->getContext();
  StringRef FWriteName = TLI->getName(LibFunc_fwrite);
  FunctionCallee F = getOrInsertLibFunc(M, *TLI, LibFunc_fwrite,
       DL.getIntPtrType(Context), B.getInt8PtrTy(), DL.getIntPtrType(Context),
       DL.getIntPtrType(Context), File->getType());

  if (File->getType()->isPointerTy())
    inferNonMandatoryLibFuncAttrs(M, FWriteName, *TLI);
  CallInst *CI =
      B.CreateCall(F, {castToCStr(Ptr, B), Size,
                       ConstantInt::get(DL.getIntPtrType(Context), 1), File});

  if (const Function *Fn =
          dyn_cast<Function>(F.getCallee()->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitMalloc(Value *Num, IRBuilderBase &B, const DataLayout &DL,
                        const TargetLibraryInfo *TLI) {
  Module *M = B.GetInsertBlock()->getModule();
  if (!isLibFuncEmittable(M, TLI, LibFunc_malloc))
    return nullptr;

  StringRef MallocName = TLI->getName(LibFunc_malloc);
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  FunctionCallee Malloc = getOrInsertLibFunc(M, *TLI, LibFunc_malloc,
                                 B.getInt8PtrTy(), DL.getIntPtrType(Context));
  inferNonMandatoryLibFuncAttrs(M, MallocName, *TLI);
  CallInst *CI = B.CreateCall(Malloc, Num, MallocName);

  if (const Function *F =
          dyn_cast<Function>(Malloc.getCallee()->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitCalloc(Value *Num, Value *Size, IRBuilderBase &B,
                        const TargetLibraryInfo &TLI) {
  Module *M = B.GetInsertBlock()->getModule();
  if (!isLibFuncEmittable(M, &TLI, LibFunc_calloc))
    return nullptr;

  StringRef CallocName = TLI.getName(LibFunc_calloc);
  const DataLayout &DL = M->getDataLayout();
  IntegerType *PtrType = DL.getIntPtrType((B.GetInsertBlock()->getContext()));
  FunctionCallee Calloc = getOrInsertLibFunc(M, TLI, LibFunc_calloc,
                                             B.getInt8PtrTy(), PtrType, PtrType);
  inferNonMandatoryLibFuncAttrs(M, CallocName, TLI);
  CallInst *CI = B.CreateCall(Calloc, {Num, Size}, CallocName);

  if (const auto *F =
          dyn_cast<Function>(Calloc.getCallee()->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}
