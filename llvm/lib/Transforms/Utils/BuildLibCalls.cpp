//===- BuildLibCalls.cpp - Utility builder for libcalls -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements some functions that will create standard C libcalls.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/BuildLibCalls.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

using namespace llvm;

#define DEBUG_TYPE "build-libcalls"

//- Infer Attributes ---------------------------------------------------------//

STATISTIC(NumReadNone, "Number of functions inferred as readnone");
STATISTIC(NumReadOnly, "Number of functions inferred as readonly");
STATISTIC(NumArgMemOnly, "Number of functions inferred as argmemonly");
STATISTIC(NumNoUnwind, "Number of functions inferred as nounwind");
STATISTIC(NumNoCapture, "Number of arguments inferred as nocapture");
STATISTIC(NumReadOnlyArg, "Number of arguments inferred as readonly");
STATISTIC(NumNoAlias, "Number of function returns inferred as noalias");
STATISTIC(NumNonNull, "Number of function returns inferred as nonnull returns");
#if INTEL_CUSTOMIZATION
STATISTIC(NumNoReturn, "Number of function inferred as noreturn");
#endif // INTEL_CUSTOMIZATION

static bool setDoesNotAccessMemory(Function &F) {
  if (F.doesNotAccessMemory())
    return false;
  F.setDoesNotAccessMemory();
  ++NumReadNone;
  return true;
}

static bool setOnlyReadsMemory(Function &F) {
  if (F.onlyReadsMemory())
    return false;
  F.setOnlyReadsMemory();
  ++NumReadOnly;
  return true;
}

static bool setOnlyAccessesArgMemory(Function &F) {
  if (F.onlyAccessesArgMemory())
    return false;
  F.setOnlyAccessesArgMemory();
  ++NumArgMemOnly;
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
  if (F.hasAttribute(AttributeList::ReturnIndex, Attribute::NoAlias))
    return false;
  F.addAttribute(AttributeList::ReturnIndex, Attribute::NoAlias);
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

static bool setOnlyReadsMemory(Function &F, unsigned ArgNo) {
  if (F.hasParamAttribute(ArgNo, Attribute::ReadOnly))
    return false;
  F.addParamAttr(ArgNo, Attribute::ReadOnly);
  ++NumReadOnlyArg;
  return true;
}

static bool setRetNonNull(Function &F) {
  assert(F.getReturnType()->isPointerTy() &&
         "nonnull applies only to pointers");
  if (F.hasAttribute(AttributeList::ReturnIndex, Attribute::NonNull))
    return false;
  F.addAttribute(AttributeList::ReturnIndex, Attribute::NonNull);
  ++NumNonNull;
  return true;
}

static bool setNonLazyBind(Function &F) {
  if (F.hasFnAttribute(Attribute::NonLazyBind))
    return false;
  F.addFnAttr(Attribute::NonLazyBind);
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
#endif // INTEL_CUSTOMIZATION

bool llvm::inferLibFuncAttributes(Function &F, const TargetLibraryInfo &TLI) {
  LibFunc TheLibFunc;
  if (!(TLI.getLibFunc(F, TheLibFunc) && TLI.has(TheLibFunc)))
    return false;

  bool Changed = false;

  if (F.getParent() != nullptr && F.getParent()->getRtLibUseGOT())
    Changed |= setNonLazyBind(F);

  switch (TheLibFunc) {
  case LibFunc_strlen:
  case LibFunc_wcslen:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_strchr:
  case LibFunc_strrchr:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_strtol:
  case LibFunc_strtod:
  case LibFunc_strtof:
  case LibFunc_strtoul:
  case LibFunc_strtoll:
  case LibFunc_strtold:
  case LibFunc_strtoull:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_strcpy:
  case LibFunc_stpcpy:
  case LibFunc_strcat:
  case LibFunc_strncat:
  case LibFunc_strncpy:
  case LibFunc_stpncpy:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_strxfrm:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_strcmp:      // 0,1
  case LibFunc_strspn:      // 0,1
  case LibFunc_strncmp:     // 0,1
  case LibFunc_strcspn:     // 0,1
  case LibFunc_strcoll:     // 0,1
  case LibFunc_strcasecmp:  // 0,1
  case LibFunc_strncasecmp: //
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_strstr:
  case LibFunc_strpbrk:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_strtok:
  case LibFunc_strtok_r:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_scanf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_setbuf:
  case LibFunc_setvbuf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_strdup:
  case LibFunc_strndup:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_stat:
  case LibFunc_statvfs:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_sscanf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_sprintf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_snprintf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 2);
    Changed |= setOnlyReadsMemory(F, 2);
    return Changed;
  case LibFunc_setitimer:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setDoesNotCapture(F, 2);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_system:
    // May throw; "system" is a valid pthread cancellation point.
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_malloc:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_memcmp:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_memchr:
  case LibFunc_memrchr:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_modf:
  case LibFunc_modff:
  case LibFunc_modfl:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_memcpy:
  case LibFunc_mempcpy:
  case LibFunc_memccpy:
  case LibFunc_memmove:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_memcpy_chk:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_memalign:
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_mkdir:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_mktime:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_realloc:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_read:
    // May throw; "read" is a valid pthread cancellation point.
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_rewind:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_rmdir:
  case LibFunc_remove:
  case LibFunc_realpath:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_rename:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_readlink:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_write:
    // May throw; "write" is a valid pthread cancellation point.
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_bcopy:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_bcmp:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_bzero:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_calloc:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_chmod:
  case LibFunc_chown:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_ctermid:
  case LibFunc_clearerr:
  case LibFunc_closedir:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_atoi:
  case LibFunc_atol:
  case LibFunc_atof:
  case LibFunc_atoll:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_access:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_fopen:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_fdopen:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_feof:
  case LibFunc_free:
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
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_ferror:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F);
    return Changed;
  case LibFunc_fputc:
  case LibFunc_fputc_unlocked:
  case LibFunc_fstat:
  case LibFunc_frexp:
  case LibFunc_frexpf:
  case LibFunc_frexpl:
  case LibFunc_fstatvfs:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_fgets:
  case LibFunc_fgets_unlocked:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 2);
    return Changed;
  case LibFunc_fread:
  case LibFunc_fread_unlocked:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 3);
    return Changed;
  case LibFunc_fwrite:
  case LibFunc_fwrite_unlocked:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 3);
    // FIXME: readonly #1?
    return Changed;
  case LibFunc_fputs:
  case LibFunc_fputs_unlocked:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_fscanf:
  case LibFunc_fprintf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_fgetpos:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_getc:
  case LibFunc_getlogin_r:
  case LibFunc_getc_unlocked:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_getenv:
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_gets:
  case LibFunc_getchar:
  case LibFunc_getchar_unlocked:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getitimer:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_getpwnam:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_ungetc:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_uname:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_unlink:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_unsetenv:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_utime:
  case LibFunc_utimes:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_putc:
  case LibFunc_putc_unlocked:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_puts:
  case LibFunc_printf:
  case LibFunc_perror:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_pread:
    // May throw; "pread" is a valid pthread cancellation point.
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_pwrite:
    // May throw; "pwrite" is a valid pthread cancellation point.
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_putchar:
  case LibFunc_putchar_unlocked:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_popen:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_pclose:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_vscanf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_vsscanf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_vfscanf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_valloc:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_vprintf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_vfprintf:
  case LibFunc_vsprintf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_vsnprintf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 2);
    Changed |= setOnlyReadsMemory(F, 2);
    return Changed;
  case LibFunc_open:
    // May throw; "open" is a valid pthread cancellation point.
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_opendir:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_tmpfile:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_times:
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
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_lchown:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_qsort:
    // May throw; places call through function pointer.
    Changed |= setDoesNotCapture(F, 3);
    return Changed;
  case LibFunc_dunder_strdup:
  case LibFunc_dunder_strndup:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_dunder_strtok_r:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_under_IO_getc:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_under_IO_putc:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_dunder_isoc99_scanf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_stat64:
  case LibFunc_lstat64:
  case LibFunc_statvfs64:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_dunder_isoc99_sscanf:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_fopen64:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 0);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  case LibFunc_fseeko64:
  case LibFunc_ftello64:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    return Changed;
  case LibFunc_tmpfile64:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  case LibFunc_fstat64:
  case LibFunc_fstatvfs64:
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_open64:
    // May throw; "open" is a valid pthread cancellation point.
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setOnlyReadsMemory(F, 0);
    return Changed;
  case LibFunc_gettimeofday:
    // Currently some platforms have the restrict keyword on the arguments to
    // gettimeofday. To be conservative, do not add noalias to gettimeofday's
    // arguments.
    Changed |= setDoesNotThrow(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    return Changed;
  case LibFunc_Znwj: // new(unsigned int)
  case LibFunc_Znwm: // new(unsigned long)
  case LibFunc_Znaj: // new[](unsigned int)
  case LibFunc_Znam: // new[](unsigned long)
  case LibFunc_msvc_new_int: // new(unsigned int)
  case LibFunc_msvc_new_longlong: // new(unsigned long long)
  case LibFunc_msvc_new_array_int: // new[](unsigned int)
  case LibFunc_msvc_new_array_longlong: // new[](unsigned long long)
    // Operator new always returns a nonnull noalias pointer
    Changed |= setRetNonNull(F);
    Changed |= setRetDoesNotAlias(F);
    return Changed;
  // TODO: add LibFunc entries for:
  // case LibFunc_memset_pattern4:
  // case LibFunc_memset_pattern8:
  case LibFunc_memset_pattern16:
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotCapture(F, 0);
    Changed |= setDoesNotCapture(F, 1);
    Changed |= setOnlyReadsMemory(F, 1);
    return Changed;
  // int __nvvm_reflect(const char *)
  case LibFunc_nvvm_reflect:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;

#ifdef INTEL_CUSTOMIZATION
  case LibFunc_assert_fail:
    Changed |= setDoesNotReturn(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_clang_call_terminate:
    return Changed;
  case LibFunc_ctype_b_loc:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetNonNull(F);
    return Changed;
  case LibFunc_ctype_get_mb_cur_max:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ctype_tolower_loc:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetNonNull(F);
    return Changed;
  case LibFunc_ctype_toupper_loc:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetNonNull(F);
    return Changed;
  case LibFunc_cxa_allocate_exception:
    Changed |= setRetDoesNotAlias(F);
    Changed |= setRetNonNull(F);
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
    Changed |= setRetNonNull(F);
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
    Changed |= setDoesNotThrow(F);
    Changed |= setOnlyReadsMemory(F);
    return Changed;
  case LibFunc_errno_location:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetNonNull(F);
    return Changed;
  case LibFunc_fxstat:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_fxstat64:
    Changed |= setDoesNotThrow(F);
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
  case LibFunc_kmpc_barrier:
    return Changed;
  case LibFunc_kmpc_critical:
    return Changed;
  case LibFunc_kmpc_dispatch_init_4:
    return Changed;
  case LibFunc_kmpc_dispatch_next_4:
    return Changed;
  case LibFunc_kmpc_end_critical:
    return Changed;
  case LibFunc_kmpc_end_reduce_nowait:
    return Changed;
  case LibFunc_kmpc_end_serialized_parallel:
    return Changed;
  case LibFunc_kmpc_for_static_fini:
    return Changed;
  case LibFunc_kmpc_for_static_init_4:
    return Changed;
  case LibFunc_kmpc_for_static_init_8:
    return Changed;
  case LibFunc_kmpc_fork_call:
    return Changed;
  case LibFunc_kmpc_global_thread_num:
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
  case LibFunc_kmpc_master:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kmpc_end_master:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_lxstat:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_sigsetjmp:
    return Changed;
  case LibFunc_sysv_signal:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_under_exit:
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
  case LibFunc_ZNKSt9exception4whatEv:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    Changed |= setRetDoesNotAlias(F);
    Changed |= setRetNonNull(F);
    return Changed;
  case LibFunc_ZNSi10_M_extractIdEERSiRT_:
    return Changed;
  case LibFunc_ZNSi10_M_extractIfEERSiRT_:
    return Changed;
  case LibFunc_ZNSi10_M_extractIlEERSiRT_:
    return Changed;
  case LibFunc_ZNSi10_M_extractImEERSiRT_:
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
  case LibFunc_ZNSt13runtime_errorC1ERKSs:
    return Changed;
  case LibFunc_ZNSt13runtime_errorC2ERKSs:
    return Changed;
  case LibFunc_ZNSt13runtime_errorD0Ev:
    return Changed;
  case LibFunc_ZNSt13runtime_errorD1Ev:
    return Changed;
  case LibFunc_ZNSt13runtime_errorD2Ev:
    return Changed;
  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl:
    return Changed;
  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsputnEPKcl:
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
  case LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE4initEPSt15basic_streambufIcS1_E:
    return Changed;
  case LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate:
    return Changed;
  case LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE5rdbufEPSt15basic_streambufIcS1_E:
    return Changed;
  case LibFunc_ZNSt9exceptionD1Ev:
    return Changed;
  case LibFunc_ZNSt9exceptionD2Ev:
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
  case LibFunc_ZSt28_Rb_tree_rebalance_for_erasePSt18_Rb_tree_node_baseRS_:
    return Changed;
  case LibFunc_ZSt29_Rb_tree_insert_and_rebalancebPSt18_Rb_tree_node_baseS0_RS_:
    return Changed;
  case LibFunc_ZSt7getlineIcSt11char_traitsIcESaIcEERSt13basic_istreamIT_T0_ES7_RSbIS4_S5_T1_ES4_:
    return Changed;
  case LibFunc_ZSt9terminatev:
    Changed |= setDoesNotReturn(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ZStrsIcSt11char_traitsIcEERSt13basic_istreamIT_T0_ES6_RS3_:
    return Changed;
  case LibFunc_abort:
    Changed |= setDoesNotReturn(F);
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
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_close:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_ctime:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_difftime:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_div:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_dup:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_erfc:
    Changed |= setDoesNotAccessMemory(F);
    Changed |= setDoesNotThrow(F);
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
  case LibFunc_fnmatch:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_fork:
    Changed |= setDoesNotThrow(F);
    return Changed;
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
    Changed |= setOnlyAccessesArgMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getpid:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getpwuid:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getrlimit:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_getrusage:
    Changed |= setOnlyReadsMemory(F);
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
  case LibFunc_j0:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_j1:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_kill:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_link:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_localeconv:
    Changed |= setDoesNotThrow(F);
    Changed |= setRetNonNull(F);
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
  case LibFunc_mblen:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_mbstowcs:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_mkdtemp:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_mmap:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_munmap:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_obstack_free:
    return Changed;
  case LibFunc_omp_destroy_lock:
    return Changed;
  case LibFunc_omp_get_max_threads:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_num_threads:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_get_thread_num:
    Changed |= setOnlyReadsMemory(F);
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_init_lock:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_lock:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_unset_lock:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_num_threads:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_omp_set_nested:
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
  case LibFunc_sleep:
    Changed |= setDoesNotThrow(F);
    return Changed;
  case LibFunc_srand:
    Changed |= setDoesNotThrow(F);
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
  case LibFunc_time:
    Changed |= setDoesNotThrow(F);
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
#endif //INTEL_CUSTOMIZATION

  default:
    // FIXME: It'd be really nice to cover all the library functions we're
    // aware of here.
    return false;
  }
}

bool llvm::hasUnaryFloatFn(const TargetLibraryInfo *TLI, Type *Ty,
                           LibFunc DoubleFn, LibFunc FloatFn,
                           LibFunc LongDoubleFn) {
  switch (Ty->getTypeID()) {
  case Type::FloatTyID:
    return TLI->has(FloatFn);
  case Type::DoubleTyID:
    return TLI->has(DoubleFn);
  default:
    return TLI->has(LongDoubleFn);
  }
}

//- Emit LibCalls ------------------------------------------------------------//

Value *llvm::castToCStr(Value *V, IRBuilder<> &B) {
  unsigned AS = V->getType()->getPointerAddressSpace();
  return B.CreateBitCast(V, B.getInt8PtrTy(AS), "cstr");
}

Value *llvm::emitStrLen(Value *Ptr, IRBuilder<> &B, const DataLayout &DL,
                        const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_strlen))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  Constant *StrLen = M->getOrInsertFunction("strlen", DL.getIntPtrType(Context),
                                            B.getInt8PtrTy());
  inferLibFuncAttributes(*M->getFunction("strlen"), *TLI);
  CallInst *CI = B.CreateCall(StrLen, castToCStr(Ptr, B), "strlen");
  if (const Function *F = dyn_cast<Function>(StrLen->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitStrChr(Value *Ptr, char C, IRBuilder<> &B,
                        const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_strchr))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  Type *I8Ptr = B.getInt8PtrTy();
  Type *I32Ty = B.getInt32Ty();
  Constant *StrChr =
      M->getOrInsertFunction("strchr", I8Ptr, I8Ptr, I32Ty);
  inferLibFuncAttributes(*M->getFunction("strchr"), *TLI);
  CallInst *CI = B.CreateCall(
      StrChr, {castToCStr(Ptr, B), ConstantInt::get(I32Ty, C)}, "strchr");
  if (const Function *F = dyn_cast<Function>(StrChr->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitStrNCmp(Value *Ptr1, Value *Ptr2, Value *Len, IRBuilder<> &B,
                         const DataLayout &DL, const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_strncmp))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  Value *StrNCmp = M->getOrInsertFunction("strncmp", B.getInt32Ty(),
                                          B.getInt8PtrTy(), B.getInt8PtrTy(),
                                          DL.getIntPtrType(Context));
  inferLibFuncAttributes(*M->getFunction("strncmp"), *TLI);
  CallInst *CI = B.CreateCall(
      StrNCmp, {castToCStr(Ptr1, B), castToCStr(Ptr2, B), Len}, "strncmp");

  if (const Function *F = dyn_cast<Function>(StrNCmp->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitStrCpy(Value *Dst, Value *Src, IRBuilder<> &B,
                        const TargetLibraryInfo *TLI, StringRef Name) {
  if (!TLI->has(LibFunc_strcpy))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  Type *I8Ptr = B.getInt8PtrTy();
  Value *StrCpy = M->getOrInsertFunction(Name, I8Ptr, I8Ptr, I8Ptr);
  inferLibFuncAttributes(*M->getFunction(Name), *TLI);
  CallInst *CI =
      B.CreateCall(StrCpy, {castToCStr(Dst, B), castToCStr(Src, B)}, Name);
  if (const Function *F = dyn_cast<Function>(StrCpy->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitStrNCpy(Value *Dst, Value *Src, Value *Len, IRBuilder<> &B,
                         const TargetLibraryInfo *TLI, StringRef Name) {
  if (!TLI->has(LibFunc_strncpy))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  Type *I8Ptr = B.getInt8PtrTy();
  Value *StrNCpy = M->getOrInsertFunction(Name, I8Ptr, I8Ptr, I8Ptr,
                                          Len->getType());
  inferLibFuncAttributes(*M->getFunction(Name), *TLI);
  CallInst *CI = B.CreateCall(
      StrNCpy, {castToCStr(Dst, B), castToCStr(Src, B), Len}, "strncpy");
  if (const Function *F = dyn_cast<Function>(StrNCpy->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitMemCpyChk(Value *Dst, Value *Src, Value *Len, Value *ObjSize,
                           IRBuilder<> &B, const DataLayout &DL,
                           const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_memcpy_chk))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  AttributeList AS;
  AS = AttributeList::get(M->getContext(), AttributeList::FunctionIndex,
                          Attribute::NoUnwind);
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  Value *MemCpy = M->getOrInsertFunction(
      "__memcpy_chk", AttributeList::get(M->getContext(), AS), B.getInt8PtrTy(),
      B.getInt8PtrTy(), B.getInt8PtrTy(), DL.getIntPtrType(Context),
      DL.getIntPtrType(Context));
  Dst = castToCStr(Dst, B);
  Src = castToCStr(Src, B);
  CallInst *CI = B.CreateCall(MemCpy, {Dst, Src, Len, ObjSize});
  if (const Function *F = dyn_cast<Function>(MemCpy->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitMemChr(Value *Ptr, Value *Val, Value *Len, IRBuilder<> &B,
                        const DataLayout &DL, const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_memchr))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  Value *MemChr = M->getOrInsertFunction("memchr", B.getInt8PtrTy(),
                                         B.getInt8PtrTy(), B.getInt32Ty(),
                                         DL.getIntPtrType(Context));
  inferLibFuncAttributes(*M->getFunction("memchr"), *TLI);
  CallInst *CI = B.CreateCall(MemChr, {castToCStr(Ptr, B), Val, Len}, "memchr");

  if (const Function *F = dyn_cast<Function>(MemChr->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitMemCmp(Value *Ptr1, Value *Ptr2, Value *Len, IRBuilder<> &B,
                        const DataLayout &DL, const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_memcmp))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  Value *MemCmp = M->getOrInsertFunction("memcmp", B.getInt32Ty(),
                                         B.getInt8PtrTy(), B.getInt8PtrTy(),
                                         DL.getIntPtrType(Context));
  inferLibFuncAttributes(*M->getFunction("memcmp"), *TLI);
  CallInst *CI = B.CreateCall(
      MemCmp, {castToCStr(Ptr1, B), castToCStr(Ptr2, B), Len}, "memcmp");

  if (const Function *F = dyn_cast<Function>(MemCmp->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
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

Value *llvm::emitUnaryFloatFnCall(Value *Op, StringRef Name, IRBuilder<> &B,
                                  const AttributeList &Attrs) {
  SmallString<20> NameBuffer;
  appendTypeSuffix(Op, Name, NameBuffer);

  Module *M = B.GetInsertBlock()->getModule();
  Value *Callee = M->getOrInsertFunction(Name, Op->getType(),
                                         Op->getType());
  CallInst *CI = B.CreateCall(Callee, Op, Name);

  // The incoming attribute set may have come from a speculatable intrinsic, but
  // is being replaced with a library call which is not allowed to be
  // speculatable.
  CI->setAttributes(Attrs.removeAttribute(B.getContext(),
                                          AttributeList::FunctionIndex,
                                          Attribute::Speculatable));
  if (const Function *F = dyn_cast<Function>(Callee->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitBinaryFloatFnCall(Value *Op1, Value *Op2, StringRef Name,
                                   IRBuilder<> &B, const AttributeList &Attrs) {
  SmallString<20> NameBuffer;
  appendTypeSuffix(Op1, Name, NameBuffer);

  Module *M = B.GetInsertBlock()->getModule();
  Value *Callee = M->getOrInsertFunction(Name, Op1->getType(), Op1->getType(),
                                         Op2->getType());
  CallInst *CI = B.CreateCall(Callee, {Op1, Op2}, Name);
  CI->setAttributes(Attrs);
  if (const Function *F = dyn_cast<Function>(Callee->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitPutChar(Value *Char, IRBuilder<> &B,
                         const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_putchar))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  Value *PutChar = M->getOrInsertFunction("putchar", B.getInt32Ty(), B.getInt32Ty());
  inferLibFuncAttributes(*M->getFunction("putchar"), *TLI);
  CallInst *CI = B.CreateCall(PutChar,
                              B.CreateIntCast(Char,
                              B.getInt32Ty(),
                              /*isSigned*/true,
                              "chari"),
                              "putchar");

  if (const Function *F = dyn_cast<Function>(PutChar->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitPutS(Value *Str, IRBuilder<> &B,
                      const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_puts))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  Value *PutS =
      M->getOrInsertFunction("puts", B.getInt32Ty(), B.getInt8PtrTy());
  inferLibFuncAttributes(*M->getFunction("puts"), *TLI);
  CallInst *CI = B.CreateCall(PutS, castToCStr(Str, B), "puts");
  if (const Function *F = dyn_cast<Function>(PutS->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());
  return CI;
}

Value *llvm::emitFPutC(Value *Char, Value *File, IRBuilder<> &B,
                       const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_fputc))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  Constant *F = M->getOrInsertFunction("fputc", B.getInt32Ty(), B.getInt32Ty(),
                                       File->getType());
  if (File->getType()->isPointerTy())
    inferLibFuncAttributes(*M->getFunction("fputc"), *TLI);
  Char = B.CreateIntCast(Char, B.getInt32Ty(), /*isSigned*/true,
                         "chari");
  CallInst *CI = B.CreateCall(F, {Char, File}, "fputc");

  if (const Function *Fn = dyn_cast<Function>(F->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitFPutCUnlocked(Value *Char, Value *File, IRBuilder<> &B,
                               const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_fputc_unlocked))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  Constant *F = M->getOrInsertFunction("fputc_unlocked", B.getInt32Ty(),
                                       B.getInt32Ty(), File->getType());
  if (File->getType()->isPointerTy())
    inferLibFuncAttributes(*M->getFunction("fputc_unlocked"), *TLI);
  Char = B.CreateIntCast(Char, B.getInt32Ty(), /*isSigned*/ true, "chari");
  CallInst *CI = B.CreateCall(F, {Char, File}, "fputc_unlocked");

  if (const Function *Fn = dyn_cast<Function>(F->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitFPutS(Value *Str, Value *File, IRBuilder<> &B,
                       const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_fputs))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  StringRef FPutsName = TLI->getName(LibFunc_fputs);
  Constant *F = M->getOrInsertFunction(
      FPutsName, B.getInt32Ty(), B.getInt8PtrTy(), File->getType());
  if (File->getType()->isPointerTy())
    inferLibFuncAttributes(*M->getFunction(FPutsName), *TLI);
  CallInst *CI = B.CreateCall(F, {castToCStr(Str, B), File}, "fputs");

  if (const Function *Fn = dyn_cast<Function>(F->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitFPutSUnlocked(Value *Str, Value *File, IRBuilder<> &B,
                               const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_fputs_unlocked))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  StringRef FPutsUnlockedName = TLI->getName(LibFunc_fputs_unlocked);
  Constant *F = M->getOrInsertFunction(FPutsUnlockedName, B.getInt32Ty(),
                                       B.getInt8PtrTy(), File->getType());
  if (File->getType()->isPointerTy())
    inferLibFuncAttributes(*M->getFunction(FPutsUnlockedName), *TLI);
  CallInst *CI = B.CreateCall(F, {castToCStr(Str, B), File}, "fputs_unlocked");

  if (const Function *Fn = dyn_cast<Function>(F->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitFWrite(Value *Ptr, Value *Size, Value *File, IRBuilder<> &B,
                        const DataLayout &DL, const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_fwrite))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  StringRef FWriteName = TLI->getName(LibFunc_fwrite);
  Constant *F = M->getOrInsertFunction(
      FWriteName, DL.getIntPtrType(Context), B.getInt8PtrTy(),
      DL.getIntPtrType(Context), DL.getIntPtrType(Context), File->getType());

  if (File->getType()->isPointerTy())
    inferLibFuncAttributes(*M->getFunction(FWriteName), *TLI);
  CallInst *CI =
      B.CreateCall(F, {castToCStr(Ptr, B), Size,
                       ConstantInt::get(DL.getIntPtrType(Context), 1), File});

  if (const Function *Fn = dyn_cast<Function>(F->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitMalloc(Value *Num, IRBuilder<> &B, const DataLayout &DL,
                        const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_malloc))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  Value *Malloc = M->getOrInsertFunction("malloc", B.getInt8PtrTy(),
                                         DL.getIntPtrType(Context));
  inferLibFuncAttributes(*M->getFunction("malloc"), *TLI);
  CallInst *CI = B.CreateCall(Malloc, Num, "malloc");

  if (const Function *F = dyn_cast<Function>(Malloc->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitCalloc(Value *Num, Value *Size, const AttributeList &Attrs,
                        IRBuilder<> &B, const TargetLibraryInfo &TLI) {
  if (!TLI.has(LibFunc_calloc))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  const DataLayout &DL = M->getDataLayout();
  IntegerType *PtrType = DL.getIntPtrType((B.GetInsertBlock()->getContext()));
  Value *Calloc = M->getOrInsertFunction("calloc", Attrs, B.getInt8PtrTy(),
                                         PtrType, PtrType);
  inferLibFuncAttributes(*M->getFunction("calloc"), TLI);
  CallInst *CI = B.CreateCall(Calloc, {Num, Size}, "calloc");

  if (const auto *F = dyn_cast<Function>(Calloc->stripPointerCasts()))
    CI->setCallingConv(F->getCallingConv());

  return CI;
}

Value *llvm::emitFWriteUnlocked(Value *Ptr, Value *Size, Value *N, Value *File,
                                IRBuilder<> &B, const DataLayout &DL,
                                const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_fwrite_unlocked))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  StringRef FWriteUnlockedName = TLI->getName(LibFunc_fwrite_unlocked);
  Constant *F = M->getOrInsertFunction(
      FWriteUnlockedName, DL.getIntPtrType(Context), B.getInt8PtrTy(),
      DL.getIntPtrType(Context), DL.getIntPtrType(Context), File->getType());

  if (File->getType()->isPointerTy())
    inferLibFuncAttributes(*M->getFunction(FWriteUnlockedName), *TLI);
  CallInst *CI = B.CreateCall(F, {castToCStr(Ptr, B), Size, N, File});

  if (const Function *Fn = dyn_cast<Function>(F->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitFGetCUnlocked(Value *File, IRBuilder<> &B,
                               const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_fgetc_unlocked))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  Constant *F =
      M->getOrInsertFunction("fgetc_unlocked", B.getInt32Ty(), File->getType());
  if (File->getType()->isPointerTy())
    inferLibFuncAttributes(*M->getFunction("fgetc_unlocked"), *TLI);
  CallInst *CI = B.CreateCall(F, File, "fgetc_unlocked");

  if (const Function *Fn = dyn_cast<Function>(F->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitFGetSUnlocked(Value *Str, Value *Size, Value *File,
                               IRBuilder<> &B, const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_fgets_unlocked))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  Constant *F =
      M->getOrInsertFunction("fgets_unlocked", B.getInt8PtrTy(),
                             B.getInt8PtrTy(), B.getInt32Ty(), File->getType());
  inferLibFuncAttributes(*M->getFunction("fgets_unlocked"), *TLI);
  CallInst *CI =
      B.CreateCall(F, {castToCStr(Str, B), Size, File}, "fgets_unlocked");

  if (const Function *Fn = dyn_cast<Function>(F->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}

Value *llvm::emitFReadUnlocked(Value *Ptr, Value *Size, Value *N, Value *File,
                               IRBuilder<> &B, const DataLayout &DL,
                               const TargetLibraryInfo *TLI) {
  if (!TLI->has(LibFunc_fread_unlocked))
    return nullptr;

  Module *M = B.GetInsertBlock()->getModule();
  LLVMContext &Context = B.GetInsertBlock()->getContext();
  StringRef FReadUnlockedName = TLI->getName(LibFunc_fread_unlocked);
  Constant *F = M->getOrInsertFunction(
      FReadUnlockedName, DL.getIntPtrType(Context), B.getInt8PtrTy(),
      DL.getIntPtrType(Context), DL.getIntPtrType(Context), File->getType());

  if (File->getType()->isPointerTy())
    inferLibFuncAttributes(*M->getFunction(FReadUnlockedName), *TLI);
  CallInst *CI = B.CreateCall(F, {castToCStr(Ptr, B), Size, N, File});

  if (const Function *Fn = dyn_cast<Function>(F->stripPointerCasts()))
    CI->setCallingConv(Fn->getCallingConv());
  return CI;
}
