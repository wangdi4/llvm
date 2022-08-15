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
//===-- TargetLibraryInfo.cpp - Runtime library information ----------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the TargetLibraryInfo class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Constants.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;

static cl::opt<TargetLibraryInfoImpl::VectorLibrary> ClVectorLibrary(
    "vector-library", cl::Hidden, cl::desc("Vector functions library"),
    cl::init(TargetLibraryInfoImpl::NoLibrary),
    cl::values(clEnumValN(TargetLibraryInfoImpl::NoLibrary, "none",
                          "No vector functions library"),
               clEnumValN(TargetLibraryInfoImpl::Accelerate, "Accelerate",
                          "Accelerate framework"),
               clEnumValN(TargetLibraryInfoImpl::DarwinLibSystemM,
                          "Darwin_libsystem_m", "Darwin libsystem_m"),
               clEnumValN(TargetLibraryInfoImpl::LIBMVEC_X86, "LIBMVEC-X86",
                          "GLIBC Vector Math library"),
               clEnumValN(TargetLibraryInfoImpl::MASSV, "MASSV",
                          "IBM MASS vector library"),
               clEnumValN(TargetLibraryInfoImpl::SVML, "SVML",
                          "Intel SVML library"),
#if INTEL_CUSTOMIZATION
               clEnumValN(TargetLibraryInfoImpl::Libmvec, "Libmvec",
                          "Glibc vector math library")));
#endif

StringLiteral const TargetLibraryInfoImpl::StandardNames[LibFunc::NumLibFuncs] =
    {
#define TLI_DEFINE_STRING
#include "llvm/Analysis/TargetLibraryInfo.def"
};

static bool hasSinCosPiStret(const Triple &T) {
  // Only Darwin variants have _stret versions of combined trig functions.
  if (!T.isOSDarwin())
    return false;

  // The ABI is rather complicated on x86, so don't do anything special there.
  if (T.getArch() == Triple::x86)
    return false;

  if (T.isMacOSX() && T.isMacOSXVersionLT(10, 9))
    return false;

  if (T.isiOS() && T.isOSVersionLT(7, 0))
    return false;

  return true;
}

static bool hasBcmp(const Triple &TT) {
  // Posix removed support from bcmp() in 2001, but the glibc and several
  // implementations of the libc still have it.
  if (TT.isOSLinux())
    return TT.isGNUEnvironment() || TT.isMusl();
  // Both NetBSD and OpenBSD are planning to remove the function. Windows does
  // not have it.
  return TT.isOSFreeBSD() || TT.isOSSolaris();
}

static bool isCallingConvCCompatible(CallingConv::ID CC, StringRef TT,
                                     FunctionType *FuncTy) {
  switch (CC) {
  default:
    return false;
  case llvm::CallingConv::C:
    return true;
  case llvm::CallingConv::ARM_APCS:
  case llvm::CallingConv::ARM_AAPCS:
  case llvm::CallingConv::ARM_AAPCS_VFP: {

    // The iOS ABI diverges from the standard in some cases, so for now don't
    // try to simplify those calls.
    if (Triple(TT).isiOS())
      return false;

    if (!FuncTy->getReturnType()->isPointerTy() &&
        !FuncTy->getReturnType()->isIntegerTy() &&
        !FuncTy->getReturnType()->isVoidTy())
      return false;

    for (auto *Param : FuncTy->params()) {
      if (!Param->isPointerTy() && !Param->isIntegerTy())
        return false;
    }
    return true;
  }
  }
  return false;
}

bool TargetLibraryInfoImpl::isCallingConvCCompatible(CallBase *CI) {
  return ::isCallingConvCCompatible(CI->getCallingConv(),
                                    CI->getModule()->getTargetTriple(),
                                    CI->getFunctionType());
}

bool TargetLibraryInfoImpl::isCallingConvCCompatible(Function *F) {
  return ::isCallingConvCCompatible(F->getCallingConv(),
                                    F->getParent()->getTargetTriple(),
                                    F->getFunctionType());
}

/// Initialize the set of available library functions based on the specified
/// target triple. This should be carefully written so that a missing target
/// triple gets a sane set of defaults.
static void initialize(TargetLibraryInfoImpl &TLI, const Triple &T,
                       ArrayRef<StringLiteral> StandardNames) {
  // Verify that the StandardNames array is in alphabetical order.
  assert(
      llvm::is_sorted(StandardNames,
                      [](StringRef LHS, StringRef RHS) { return LHS < RHS; }) &&
      "TargetLibraryInfoImpl function names must be sorted");

  // Set IO unlocked variants as unavailable
  // Set them as available per system below
  TLI.setUnavailable(LibFunc_getc_unlocked);
  TLI.setUnavailable(LibFunc_getchar_unlocked);
  TLI.setUnavailable(LibFunc_putc_unlocked);
  TLI.setUnavailable(LibFunc_putchar_unlocked);
  TLI.setUnavailable(LibFunc_fputc_unlocked);
  TLI.setUnavailable(LibFunc_fgetc_unlocked);
  TLI.setUnavailable(LibFunc_fread_unlocked);
  TLI.setUnavailable(LibFunc_fwrite_unlocked);
  TLI.setUnavailable(LibFunc_fputs_unlocked);
  TLI.setUnavailable(LibFunc_fgets_unlocked);

  bool ShouldExtI32Param = false, ShouldExtI32Return = false,
       ShouldSignExtI32Param = false;
  // PowerPC64, Sparc64, SystemZ need signext/zeroext on i32 parameters and
  // returns corresponding to C-level ints and unsigned ints.
  if (T.isPPC64() || T.getArch() == Triple::sparcv9 ||
      T.getArch() == Triple::systemz) {
    ShouldExtI32Param = true;
    ShouldExtI32Return = true;
  }
  // Mips, on the other hand, needs signext on i32 parameters corresponding
  // to both signed and unsigned ints.
  if (T.isMIPS()) {
    ShouldSignExtI32Param = true;
  }
  TLI.setShouldExtI32Param(ShouldExtI32Param);
  TLI.setShouldExtI32Return(ShouldExtI32Return);
  TLI.setShouldSignExtI32Param(ShouldSignExtI32Param);

  // Let's assume by default that the size of int is 32 bits, unless the target
  // is a 16-bit architecture because then it most likely is 16 bits. If that
  // isn't true for a target those defaults should be overridden below.
  TLI.setIntSize(T.isArch16Bit() ? 16 : 32);

  // There is really no runtime library on AMDGPU, apart from
  // __kmpc_alloc/free_shared.
  if (T.isAMDGPU()) {
    TLI.disableAllFunctions();
    TLI.setAvailable(llvm::LibFunc___kmpc_alloc_shared);
    TLI.setAvailable(llvm::LibFunc___kmpc_free_shared);
    return;
  }

  // memset_pattern{4,8,16} is only available on iOS 3.0 and Mac OS X 10.5 and
  // later. All versions of watchOS support it.
  if (T.isMacOSX()) {
    // available IO unlocked variants on Mac OS X
    TLI.setAvailable(LibFunc_getc_unlocked);
    TLI.setAvailable(LibFunc_getchar_unlocked);
    TLI.setAvailable(LibFunc_putc_unlocked);
    TLI.setAvailable(LibFunc_putchar_unlocked);

    if (T.isMacOSXVersionLT(10, 5)) {
      TLI.setUnavailable(LibFunc_memset_pattern4);
      TLI.setUnavailable(LibFunc_memset_pattern8);
      TLI.setUnavailable(LibFunc_memset_pattern16);
    }
  } else if (T.isiOS()) {
    if (T.isOSVersionLT(3, 0)) {
      TLI.setUnavailable(LibFunc_memset_pattern4);
      TLI.setUnavailable(LibFunc_memset_pattern8);
      TLI.setUnavailable(LibFunc_memset_pattern16);
    }
  } else if (!T.isWatchOS()) {
    TLI.setUnavailable(LibFunc_memset_pattern4);
    TLI.setUnavailable(LibFunc_memset_pattern8);
    TLI.setUnavailable(LibFunc_memset_pattern16);
  }

  if (!hasSinCosPiStret(T)) {
    TLI.setUnavailable(LibFunc_sinpi);
    TLI.setUnavailable(LibFunc_sinpif);
    TLI.setUnavailable(LibFunc_cospi);
    TLI.setUnavailable(LibFunc_cospif);
    TLI.setUnavailable(LibFunc_sincospi_stret);
    TLI.setUnavailable(LibFunc_sincospif_stret);
  }

  if (!hasBcmp(T))
    TLI.setUnavailable(LibFunc_bcmp);

  if (T.isMacOSX() && T.getArch() == Triple::x86 &&
      !T.isMacOSXVersionLT(10, 7)) {
    // x86-32 OSX has a scheme where fwrite and fputs (and some other functions
    // we don't care about) have two versions; on recent OSX, the one we want
    // has a $UNIX2003 suffix. The two implementations are identical except
    // for the return value in some edge cases.  However, we don't want to
    // generate code that depends on the old symbols.
    TLI.setAvailableWithName(LibFunc_fwrite, "fwrite$UNIX2003");
    TLI.setAvailableWithName(LibFunc_fputs, "fputs$UNIX2003");
  }

  // iprintf and friends are only available on XCore, TCE, and Emscripten.
  if (T.getArch() != Triple::xcore && T.getArch() != Triple::tce &&
      T.getOS() != Triple::Emscripten) {
    TLI.setUnavailable(LibFunc_iprintf);
    TLI.setUnavailable(LibFunc_siprintf);
    TLI.setUnavailable(LibFunc_fiprintf);
  }

  // __small_printf and friends are only available on Emscripten.
  if (T.getOS() != Triple::Emscripten) {
    TLI.setUnavailable(LibFunc_small_printf);
    TLI.setUnavailable(LibFunc_small_sprintf);
    TLI.setUnavailable(LibFunc_small_fprintf);
  }

  if (T.isOSWindows() && !T.isOSCygMing()) {
    // XXX: The earliest documentation available at the moment is for VS2015/VC19:
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/floating-point-support?view=vs-2015
    // XXX: In order to use an MSVCRT older than VC19,
    // the specific library version must be explicit in the target triple,
    // e.g., x86_64-pc-windows-msvc18.
    bool hasPartialC99 = true;
    if (T.isKnownWindowsMSVCEnvironment()) {
      VersionTuple Version = T.getEnvironmentVersion();
      hasPartialC99 = (Version.getMajor() == 0 || Version.getMajor() >= 19);
    }

    // Latest targets support C89 math functions, in part.
    bool isARM = (T.getArch() == Triple::aarch64 ||
                  T.getArch() == Triple::arm);
    bool hasPartialFloat = (isARM ||
                            T.getArch() == Triple::x86_64);

    // Win32 does not support float C89 math functions, in general.
    if (!hasPartialFloat) {
      TLI.setUnavailable(LibFunc_acosf);
      TLI.setUnavailable(LibFunc_asinf);
      TLI.setUnavailable(LibFunc_atan2f);
      TLI.setUnavailable(LibFunc_atanf);
      TLI.setUnavailable(LibFunc_ceilf);
      TLI.setUnavailable(LibFunc_cosf);
      TLI.setUnavailable(LibFunc_coshf);
      TLI.setUnavailable(LibFunc_expf);
      TLI.setUnavailable(LibFunc_floorf);
      TLI.setUnavailable(LibFunc_fmodf);
      TLI.setUnavailable(LibFunc_log10f);
      TLI.setUnavailable(LibFunc_logf);
      TLI.setUnavailable(LibFunc_modff);
      TLI.setUnavailable(LibFunc_powf);
      TLI.setUnavailable(LibFunc_remainderf);
      TLI.setUnavailable(LibFunc_sinf);
      TLI.setUnavailable(LibFunc_sinhf);
      TLI.setUnavailable(LibFunc_sqrtf);
      TLI.setUnavailable(LibFunc_tanf);
      TLI.setUnavailable(LibFunc_tanhf);
    }
    if (!isARM)
      TLI.setUnavailable(LibFunc_fabsf);
    TLI.setUnavailable(LibFunc_frexpf);
    TLI.setUnavailable(LibFunc_ldexpf);

    // Win32 does not support long double C89 math functions.
    TLI.setUnavailable(LibFunc_acosl);
    TLI.setUnavailable(LibFunc_asinl);
    TLI.setUnavailable(LibFunc_atan2l);
    TLI.setUnavailable(LibFunc_atanl);
    TLI.setUnavailable(LibFunc_ceill);
    TLI.setUnavailable(LibFunc_cosl);
    TLI.setUnavailable(LibFunc_coshl);
    TLI.setUnavailable(LibFunc_expl);
    TLI.setUnavailable(LibFunc_fabsl);
    TLI.setUnavailable(LibFunc_floorl);
    TLI.setUnavailable(LibFunc_fmodl);
    TLI.setUnavailable(LibFunc_frexpl);
    TLI.setUnavailable(LibFunc_ldexpl);
    TLI.setUnavailable(LibFunc_log10l);
    TLI.setUnavailable(LibFunc_logl);
    TLI.setUnavailable(LibFunc_modfl);
    TLI.setUnavailable(LibFunc_powl);
    TLI.setUnavailable(LibFunc_remainderl);
    TLI.setUnavailable(LibFunc_sinl);
    TLI.setUnavailable(LibFunc_sinhl);
    TLI.setUnavailable(LibFunc_sqrtl);
    TLI.setUnavailable(LibFunc_tanl);
    TLI.setUnavailable(LibFunc_tanhl);

    // Win32 does not fully support C99 math functions.
    if (!hasPartialC99) {
      TLI.setUnavailable(LibFunc_acosh);
      TLI.setUnavailable(LibFunc_acoshf);
      TLI.setUnavailable(LibFunc_asinh);
      TLI.setUnavailable(LibFunc_asinhf);
      TLI.setUnavailable(LibFunc_atanh);
      TLI.setUnavailable(LibFunc_atanhf);
      TLI.setAvailableWithName(LibFunc_cabs, "_cabs");
      TLI.setUnavailable(LibFunc_cabsf);
      TLI.setUnavailable(LibFunc_cbrt);
      TLI.setUnavailable(LibFunc_cbrtf);
      TLI.setAvailableWithName(LibFunc_copysign, "_copysign");
      TLI.setAvailableWithName(LibFunc_copysignf, "_copysignf");
      TLI.setUnavailable(LibFunc_exp2);
      TLI.setUnavailable(LibFunc_exp2f);
      TLI.setUnavailable(LibFunc_expm1);
      TLI.setUnavailable(LibFunc_expm1f);
      TLI.setUnavailable(LibFunc_fmax);
      TLI.setUnavailable(LibFunc_fmaxf);
      TLI.setUnavailable(LibFunc_fmin);
      TLI.setUnavailable(LibFunc_fminf);
      TLI.setUnavailable(LibFunc_log1p);
      TLI.setUnavailable(LibFunc_log1pf);
      TLI.setUnavailable(LibFunc_log2);
      TLI.setUnavailable(LibFunc_log2f);
      TLI.setAvailableWithName(LibFunc_logb, "_logb");
      if (hasPartialFloat)
        TLI.setAvailableWithName(LibFunc_logbf, "_logbf");
      else
        TLI.setUnavailable(LibFunc_logbf);
      TLI.setUnavailable(LibFunc_rint);
      TLI.setUnavailable(LibFunc_rintf);
      TLI.setUnavailable(LibFunc_round);
      TLI.setUnavailable(LibFunc_roundf);
      TLI.setUnavailable(LibFunc_trunc);
      TLI.setUnavailable(LibFunc_truncf);
    }

    // Win32 does not support long double C99 math functions.
    TLI.setUnavailable(LibFunc_acoshl);
    TLI.setUnavailable(LibFunc_asinhl);
    TLI.setUnavailable(LibFunc_atanhl);
    TLI.setUnavailable(LibFunc_cabsl);
    TLI.setUnavailable(LibFunc_cbrtl);
    TLI.setUnavailable(LibFunc_copysignl);
    TLI.setUnavailable(LibFunc_exp2l);
    TLI.setUnavailable(LibFunc_expm1l);
    TLI.setUnavailable(LibFunc_fmaxl);
    TLI.setUnavailable(LibFunc_fminl);
    TLI.setUnavailable(LibFunc_log1pl);
    TLI.setUnavailable(LibFunc_log2l);
    TLI.setUnavailable(LibFunc_logbl);
    TLI.setUnavailable(LibFunc_nearbyintl);
    TLI.setUnavailable(LibFunc_rintl);
    TLI.setUnavailable(LibFunc_roundl);
    TLI.setUnavailable(LibFunc_truncl);

    // Win32 does not support these functions, but
    // they are generally available on POSIX-compliant systems.
#if INTEL_CUSTOMIZATION
    // Enabled for Windows
    // TLI.setUnavailable(LibFunc_access);
#endif // INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_access);
    TLI.setUnavailable(LibFunc_chmod);
    TLI.setUnavailable(LibFunc_closedir);
    TLI.setUnavailable(LibFunc_fdopen);
#if INTEL_CUSTOMIZATION
    // Enabled for Windows
    // TLI.setUnavailable(LibFunc_fileno);
#endif // INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_fseeko);
    TLI.setUnavailable(LibFunc_fstat);
    TLI.setUnavailable(LibFunc_ftello);
    TLI.setUnavailable(LibFunc_gettimeofday);
    TLI.setUnavailable(LibFunc_memccpy);
    TLI.setUnavailable(LibFunc_mkdir);
#if INTEL_CUSTOMIZATION
    // Enabled for Windows
    // TLI.setUnavailable(LibFunc_open);
#endif // INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_opendir);
    TLI.setUnavailable(LibFunc_pclose);
    TLI.setUnavailable(LibFunc_popen);
#if INTEL_CUSTOMIZATION
    // Enabled for Windows
    // TLI.setUnavailable(LibFunc_read);
#endif // INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_rmdir);
    TLI.setUnavailable(LibFunc_stat);
    TLI.setUnavailable(LibFunc_strcasecmp);
    TLI.setUnavailable(LibFunc_strncasecmp);
#if INTEL_CUSTOMIZATION
    // Enabled for Windows
    // TLI.setUnavailable(LibFunc_unlink);
#endif // INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_utime);
    TLI.setUnavailable(LibFunc_write);
  }

  if (T.isOSWindows() && !T.isWindowsCygwinEnvironment()) {
    // These functions aren't available in either MSVC or MinGW environments.
    TLI.setUnavailable(LibFunc_bcmp);
    TLI.setUnavailable(LibFunc_bcopy);
    TLI.setUnavailable(LibFunc_bzero);
    TLI.setUnavailable(LibFunc_chown);
    TLI.setUnavailable(LibFunc_ctermid);
#if INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_cpow);
    TLI.setUnavailable(LibFunc_cpowf);
    TLI.setUnavailable(LibFunc_csqrt);
    TLI.setUnavailable(LibFunc_csqrtf);
    // Enabled for Windows
    // TLI.setUnavailable(LibFunc_fdopen);
#endif // INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_ffs);
    TLI.setUnavailable(LibFunc_ffs);
    TLI.setUnavailable(LibFunc_flockfile);
    TLI.setUnavailable(LibFunc_fstatvfs);
    TLI.setUnavailable(LibFunc_ftrylockfile);
    TLI.setUnavailable(LibFunc_funlockfile);
    TLI.setUnavailable(LibFunc_getitimer);
    TLI.setUnavailable(LibFunc_getlogin_r);
    TLI.setUnavailable(LibFunc_getpwnam);
    TLI.setUnavailable(LibFunc_htonl);
    TLI.setUnavailable(LibFunc_htons);
    TLI.setUnavailable(LibFunc_lchown);
    TLI.setUnavailable(LibFunc_lstat);
    TLI.setUnavailable(LibFunc_ntohl);
    TLI.setUnavailable(LibFunc_ntohs);
    TLI.setUnavailable(LibFunc_opendir);
    TLI.setUnavailable(LibFunc_pclose);
    TLI.setUnavailable(LibFunc_popen);
    TLI.setUnavailable(LibFunc_pread);
    TLI.setUnavailable(LibFunc_pwrite);
#if INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_re_compile_fastmap);
    TLI.setUnavailable(LibFunc_re_search_2);
    // This function should be available on Windows.
    // TLI.setUnavailable(LibFunc_read);
#endif // INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_pread);
    TLI.setUnavailable(LibFunc_pwrite);
    TLI.setUnavailable(LibFunc_readlink);
    TLI.setUnavailable(LibFunc_realpath);
    TLI.setUnavailable(LibFunc_setitimer);
    TLI.setUnavailable(LibFunc_statvfs);
    TLI.setUnavailable(LibFunc_stpcpy);
    TLI.setUnavailable(LibFunc_stpncpy);
    TLI.setUnavailable(LibFunc_times);
    TLI.setUnavailable(LibFunc_uname);
    TLI.setUnavailable(LibFunc_unsetenv);
    TLI.setUnavailable(LibFunc_utimes);
#if INTEL_CUSTOMIZATION
    TLI.setUnavailable(LibFunc_cxa_pure_virtual);
    TLI.setUnavailable(LibFunc_fxstat);
    TLI.setUnavailable(LibFunc_fxstat64);
    TLI.setUnavailable(LibFunc_gxx_personality_v0);
    TLI.setUnavailable(LibFunc_lxstat);
    TLI.setUnavailable(LibFunc_lxstat64);
    TLI.setUnavailable(LibFunc_sysv_signal);
    TLI.setUnavailable(LibFunc_alphasort);
    TLI.setUnavailable(LibFunc_asprintf);
    TLI.setUnavailable(LibFunc_backtrace);
    TLI.setUnavailable(LibFunc_backtrace_symbols);
    TLI.setUnavailable(LibFunc_dunder_rawmemchr);
    TLI.setUnavailable(LibFunc_dup);
    TLI.setUnavailable(LibFunc_dup2);
    TLI.setUnavailable(LibFunc_error);
    TLI.setUnavailable(LibFunc_execl);
    TLI.setUnavailable(LibFunc_execv);
    TLI.setUnavailable(LibFunc_execvp);
    TLI.setUnavailable(LibFunc_fcntl);
    TLI.setUnavailable(LibFunc_fcntl64);
    TLI.setUnavailable(LibFunc_fnmatch);
    TLI.setUnavailable(LibFunc_fork);
    TLI.setUnavailable(LibFunc_freopen);
    TLI.setUnavailable(LibFunc_freopen64);
    TLI.setUnavailable(LibFunc_fsync);
    TLI.setUnavailable(LibFunc_ftruncate64);
    TLI.setUnavailable(LibFunc_getcwd);
    TLI.setUnavailable(LibFunc_getegid);
    TLI.setUnavailable(LibFunc_geteuid);
    TLI.setUnavailable(LibFunc_getgid);
    TLI.setUnavailable(LibFunc_getopt_long);
    TLI.setUnavailable(LibFunc_getopt_long_only);
    TLI.setUnavailable(LibFunc_getpwuid);
    TLI.setUnavailable(LibFunc_getrlimit);
    TLI.setUnavailable(LibFunc_getrlimit64);
    TLI.setUnavailable(LibFunc_getrusage);
    TLI.setUnavailable(LibFunc_getuid);
    TLI.setUnavailable(LibFunc_glob);
    TLI.setUnavailable(LibFunc_globfree);
    TLI.setUnavailable(LibFunc_gmtime_r);
    TLI.setUnavailable(LibFunc_gnu_std_basic_filebuf_dtor);
    TLI.setUnavailable(LibFunc_gnu_std_cxx11_basic_stringstream_ctor);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcESaIcEED1Ev);
    TLI.setUnavailable(LibFunc_ioctl);
    TLI.setUnavailable(LibFunc_isatty);
    TLI.setUnavailable(LibFunc_kill);
    TLI.setUnavailable(LibFunc_link);
    TLI.setUnavailable(LibFunc_localtime_r);
    TLI.setUnavailable(LibFunc_lseek64);
    TLI.setUnavailable(LibFunc_mallopt);
    TLI.setUnavailable(LibFunc_mkdtemp);
    TLI.setUnavailable(LibFunc_mkstemps);
    TLI.setUnavailable(LibFunc_mmap);
    TLI.setUnavailable(LibFunc_munmap);
    TLI.setUnavailable(LibFunc_pipe);
    TLI.setUnavailable(LibFunc_pthread_key_create);
    TLI.setUnavailable(LibFunc_pthread_self);
    TLI.setUnavailable(LibFunc_putenv);
    TLI.setUnavailable(LibFunc_qsort_r);
    TLI.setUnavailable(LibFunc_readdir);
    TLI.setUnavailable(LibFunc_readdir64);
    TLI.setUnavailable(LibFunc_regcomp);
    TLI.setUnavailable(LibFunc_regerror);
    TLI.setUnavailable(LibFunc_regexec);
    TLI.setUnavailable(LibFunc_regfree);
    TLI.setUnavailable(LibFunc_scandir);
    TLI.setUnavailable(LibFunc_select);
    TLI.setUnavailable(LibFunc_setgid);
    TLI.setUnavailable(LibFunc_setrlimit);
    TLI.setUnavailable(LibFunc_setuid);
    TLI.setUnavailable(LibFunc_siglongjmp);
    TLI.setUnavailable(LibFunc_signbit);
    TLI.setUnavailable(LibFunc_strsignal);
    TLI.setUnavailable(LibFunc_symlink);
    TLI.setUnavailable(LibFunc_sysconf);
    TLI.setUnavailable(LibFunc_truncate64);
    TLI.setUnavailable(LibFunc_usleep);
    TLI.setUnavailable(LibFunc_vasprintf);
    TLI.setUnavailable(LibFunc_waitpid);

    TLI.setUnavailable(LibFunc_ZNKSs17find_first_not_ofEPKcmm);
    TLI.setUnavailable(LibFunc_ZNKSs4findEPKcmm);
    TLI.setUnavailable(LibFunc_ZNKSs4findEcm);
    TLI.setUnavailable(LibFunc_ZNKSs5rfindEPKcmm);
    TLI.setUnavailable(LibFunc_ZNKSs5rfindEcm);
    TLI.setUnavailable(LibFunc_ZNKSs7compareEPKc);
    TLI.setUnavailable(LibFunc_ZNKSt13runtime_error4whatEv);
    TLI.setUnavailable(LibFunc_ZNKSt5ctypeIcE13_M_widen_initEv);
    TLI.setUnavailable(LibFunc_std_cxx11_basic_string_find_first_not_of);
    TLI.setUnavailable(LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE4findEPKcmm);
    TLI.setUnavailable(LibFunc_std_cxx11_basic_string_find_char_unsigned_long);
    TLI.setUnavailable(LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5rfindEcm);
    TLI.setUnavailable(LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5rfindEPKcmm);
    TLI.setUnavailable(LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7compareEPKc);
    TLI.setUnavailable(LibFunc_ZNKSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE3strEv);
    TLI.setUnavailable(LibFunc_ZNKSt9bad_alloc4whatEv);
    TLI.setUnavailable(LibFunc_ZNKSt9basic_iosIcSt11char_traitsIcEE5widenEc);
    TLI.setUnavailable(LibFunc_ZNKSt9exception4whatEv);
    TLI.setUnavailable(LibFunc_ZNSi10_M_extractIdEERSiRT_);
    TLI.setUnavailable(LibFunc_ZNSi10_M_extractIfEERSiRT_);
    TLI.setUnavailable(LibFunc_ZNSi10_M_extractIlEERSiRT_);
    TLI.setUnavailable(LibFunc_ZNSi10_M_extractImEERSiRT_);
    TLI.setUnavailable(LibFunc_ZNSi4readEPci);
    TLI.setUnavailable(LibFunc_ZNSi4readEPcl);
    TLI.setUnavailable(LibFunc_ZNSi5tellgEv);
    TLI.setUnavailable(LibFunc_ZNSi5ungetEv);
    TLI.setUnavailable(LibFunc_ZNSirsERi);
    TLI.setUnavailable(LibFunc_ZNSo3putEc);
    TLI.setUnavailable(LibFunc_ZNSo5flushEv);
    TLI.setUnavailable(LibFunc_ZNSo5writeEPKci);
    TLI.setUnavailable(LibFunc_ZNSo5writeEPKcl);
    TLI.setUnavailable(LibFunc_ZNSo9_M_insertIPKvEERSoT_);
    TLI.setUnavailable(LibFunc_ZNSo9_M_insertIbEERSoT_);
    TLI.setUnavailable(LibFunc_ZNSo9_M_insertIdEERSoT_);
    TLI.setUnavailable(LibFunc_ZNSo9_M_insertIlEERSoT_);
    TLI.setUnavailable(LibFunc_ZNSo9_M_insertImEERSoT_);
    TLI.setUnavailable(LibFunc_ZNSolsEi);
    TLI.setUnavailable(LibFunc_ZNSs12_M_leak_hardEv);
    TLI.setUnavailable(LibFunc_ZNSs4_Rep10_M_destroyERKSaIcE);
    TLI.setUnavailable(LibFunc_ZNSs4_Rep9_S_createEmmRKSaIcE);
    TLI.setUnavailable(LibFunc_ZNSs6appendEPKcm);
    TLI.setUnavailable(LibFunc_ZNSs6appendERKSs);
    TLI.setUnavailable(LibFunc_ZNSs6appendEmc);
    TLI.setUnavailable(LibFunc_ZNSs6assignEPKcm);
    TLI.setUnavailable(LibFunc_ZNSs6assignERKSs);
    TLI.setUnavailable(LibFunc_ZNSs6insertEmPKcm);
    TLI.setUnavailable(LibFunc_ZNSs6resizeEmc);
    TLI.setUnavailable(LibFunc_ZNSs7replaceEmmPKcm);
    TLI.setUnavailable(LibFunc_ZNSs7reserveEm);
    TLI.setUnavailable(LibFunc_ZNSs9_M_mutateEmmm);
    TLI.setUnavailable(LibFunc_ZNSsC1EPKcRKSaIcE);
    TLI.setUnavailable(LibFunc_ZNSsC1EPKcmRKSaIcE);
    TLI.setUnavailable(LibFunc_ZNSsC1ERKSs);
    TLI.setUnavailable(LibFunc_ZNSsC1ERKSsmm);
    TLI.setUnavailable(LibFunc_ZNSt12__basic_fileIcED1Ev);
    TLI.setUnavailable(LibFunc_ZNSt13basic_filebufIcSt11char_traitsIcEE4openEPKcSt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt13basic_filebufIcSt11char_traitsIcEE5closeEv);
    TLI.setUnavailable(LibFunc_ZNSt13basic_filebufIcSt11char_traitsIcEEC1Ev);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorC1EPKc);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorC1ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorC1ERKSs);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorC1ERKS_);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorC2EPKc);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE);
    TLI.setUnavailable(LibFunc_std_runtime_error_std_runtime_error_const);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorC2ERKSs);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorD0Ev);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorD1Ev);
    TLI.setUnavailable(LibFunc_ZNSt13runtime_errorD2Ev);
    TLI.setUnavailable(LibFunc_ZNSt14basic_ifstreamIcSt11char_traitsIcEEC1EPKcSt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_std_basic_ifstream_ctor);
    TLI.setUnavailable(LibFunc_ZNSt14basic_ifstreamIcSt11char_traitsIcEED1Ev);
    TLI.setUnavailable(LibFunc_ZNSt14basic_ifstreamIcSt11char_traitsIcEED2Ev);
    TLI.setUnavailable(LibFunc_ZNSt14basic_ofstreamIcSt11char_traitsIcEEC1EPKcSt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_std_basic_ofstream_dtor);
    TLI.setUnavailable(LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE5imbueERKSt6locale);
    TLI.setUnavailable(LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE5uflowEv);
    TLI.setUnavailable(LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl);
    TLI.setUnavailable(LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsputnEPKcl);
    TLI.setUnavailable(LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEED2Ev);
    TLI.setUnavailable(LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE7_M_syncEPcmm);
    TLI.setUnavailable(LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE7seekoffElSt12_Ios_SeekdirSt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE7seekposESt4fposI11__mbstate_tESt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE8overflowEi);
    TLI.setUnavailable(LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE9pbackfailEi);
    TLI.setUnavailable(LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE9underflowEv);
    TLI.setUnavailable(LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEEC2ERKSsSt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt6localeC1Ev);
    TLI.setUnavailable(LibFunc_ZNSt6localeD1Ev);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5eraseEmm);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6insertEmRKS4_);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6resizeEmc);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7reserveEm);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE8_M_eraseEmm);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcm);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_assignERKS4_);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERmm);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEmmPKcm);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_replaceEmmPKcm);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE14_M_replace_auxEmmmc);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2EOS4);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2EPKcRKS3_);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2ERKS4_);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2ERKS4_mm);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED2Ev);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE6setbufEPcl);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7_M_syncEPcmm);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7seekoffElSt12_Ios_SeekdirSt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7seekposESt4fposI11__mbstate_tESt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE8overflowEi);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE9pbackfailEi);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE9showmanycEv);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE9underflowEv);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEEC2ERKNS_12basic_stringIcS2_S3_EESt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEED2Ev);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcESaIcEEC1ERKNS_12basic_stringIcS2_S3_EESt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcESaIcEEC1ESt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEC1ERKNS_12basic_stringIcS2_S3_EESt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1119basic_ostringstreamIcSt11char_traitsIcESaIcEEC1ESt13_Ios_Openmode);
    TLI.setUnavailable(LibFunc_std_cxx11_basic_ostringstream_ctor);
    TLI.setUnavailable(LibFunc_std_cxx11_basic_ostringstream_dtor);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6appendEPKc);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6appendERKS4);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6assignEPKc);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEOS4_);
    TLI.setUnavailable(LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEPKc);
    TLI.setUnavailable(LibFunc_ZNSt8__detail15_List_node_base11_M_transferEPS0_S1_);
    TLI.setUnavailable(LibFunc_ZNSt8__detail15_List_node_base7_M_hookEPS0_);
    TLI.setUnavailable(LibFunc_ZNSt8__detail15_List_node_base9_M_unhookEv);
    TLI.setUnavailable(LibFunc_ZNSt8ios_base4InitC1Ev);
    TLI.setUnavailable(LibFunc_ZNSt8ios_base4InitD1Ev);
    TLI.setUnavailable(LibFunc_ZNSt8ios_baseC2Ev);
    TLI.setUnavailable(LibFunc_ZNSt8ios_baseD2Ev);
    TLI.setUnavailable(LibFunc_ZNSt9bad_allocD0Ev);
    TLI.setUnavailable(LibFunc_ZNSt9bad_allocD1Ev);
    TLI.setUnavailable(LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE4initEPSt15basic_streambufIcS1_E);
    TLI.setUnavailable(LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate);
    TLI.setUnavailable(LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE5rdbufEPSt15basic_streambufIcS1_E);
    TLI.setUnavailable(LibFunc_ZNSt9exceptionD0Ev);
    TLI.setUnavailable(LibFunc_ZNSt9exceptionD1Ev);
    TLI.setUnavailable(LibFunc_ZNSt9exceptionD2Ev);
    TLI.setUnavailable(LibFunc_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_i);
    TLI.setUnavailable(LibFunc_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l);
    TLI.setUnavailable(LibFunc_ZSt16__throw_bad_castv);
    TLI.setUnavailable(LibFunc_ZSt17__throw_bad_allocv);
    TLI.setUnavailable(LibFunc_ZSt18_Rb_tree_decrementPKSt18_Rb_tree_node_base);
    TLI.setUnavailable(LibFunc_ZSt18_Rb_tree_decrementPSt18_Rb_tree_node_base);
    TLI.setUnavailable(LibFunc_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base);
    TLI.setUnavailable(LibFunc_ZSt18_Rb_tree_incrementPSt18_Rb_tree_node_base);
    TLI.setUnavailable(LibFunc_ZSt19__throw_logic_errorPKc);
    TLI.setUnavailable(LibFunc_ZSt20__throw_length_errorPKc);
    TLI.setUnavailable(LibFunc_ZSt20__throw_out_of_rangePKc);
    TLI.setUnavailable(LibFunc_ZSt24__throw_out_of_range_fmtPKcz);
    TLI.setUnavailable(LibFunc_ZSt28_Rb_tree_rebalance_for_erasePSt18_Rb_tree_node_baseRS_);
    TLI.setUnavailable(LibFunc_ZSt28__throw_bad_array_new_lengthv);
    TLI.setUnavailable(LibFunc_ZSt29_Rb_tree_insert_and_rebalancebPSt18_Rb_tree_node_baseS0_RS_);
    TLI.setUnavailable(LibFunc_std_basic_ostream_std_endl);
    TLI.setUnavailable(LibFunc_std_basic_ostream_std_flush);
    TLI.setUnavailable(LibFunc_ZSt7getlineIcSt11char_traitsIcESaIcEERSt13basic_istreamIT_T0_ES7_RSbIS4_S5_T1_ES4_);
    TLI.setUnavailable(LibFunc_std_basic_istream_getline_cxx11_char);
    TLI.setUnavailable(LibFunc_ZSt9terminatev);
    TLI.setUnavailable(LibFunc_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc);
    TLI.setUnavailable(LibFunc_ZStrsIcSt11char_traitsIcEERSt13basic_istreamIT_T0_ES6_RS3_);
#endif // INTEL_CUSTOMIZATION
  }

  // Pick just one set of new/delete variants.
  if (T.isOSMSVCRT()) {
    // MSVC, doesn't have the Itanium new/delete.
    TLI.setUnavailable(LibFunc_ZdaPv);
    TLI.setUnavailable(LibFunc_ZdaPvRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZdaPvSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZdaPvSt11align_val_tRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZdaPvj);
    TLI.setUnavailable(LibFunc_ZdaPvjSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZdaPvm);
    TLI.setUnavailable(LibFunc_ZdaPvmSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZdlPv);
    TLI.setUnavailable(LibFunc_ZdlPvRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZdlPvSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZdlPvSt11align_val_tRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZdlPvj);
    TLI.setUnavailable(LibFunc_ZdlPvjSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZdlPvm);
    TLI.setUnavailable(LibFunc_ZdlPvmSt11align_val_t);
    TLI.setUnavailable(LibFunc_Znaj);
    TLI.setUnavailable(LibFunc_ZnajRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZnajSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZnajSt11align_val_tRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_Znam);
    TLI.setUnavailable(LibFunc_ZnamRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZnamSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZnamSt11align_val_tRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_Znwj);
    TLI.setUnavailable(LibFunc_ZnwjRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZnwjSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZnwjSt11align_val_tRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_Znwm);
    TLI.setUnavailable(LibFunc_ZnwmRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZnwmSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZnwmSt11align_val_tRKSt9nothrow_t);
  } else {
    // Not MSVC, assume it's Itanium.
    TLI.setUnavailable(LibFunc_msvc_new_int);
    TLI.setUnavailable(LibFunc_msvc_new_int_nothrow);
    TLI.setUnavailable(LibFunc_msvc_new_longlong);
    TLI.setUnavailable(LibFunc_msvc_new_longlong_nothrow);
    TLI.setUnavailable(LibFunc_msvc_delete_ptr32);
    TLI.setUnavailable(LibFunc_msvc_delete_ptr32_nothrow);
    TLI.setUnavailable(LibFunc_msvc_delete_ptr32_int);
    TLI.setUnavailable(LibFunc_msvc_delete_ptr64);
    TLI.setUnavailable(LibFunc_msvc_delete_ptr64_nothrow);
    TLI.setUnavailable(LibFunc_msvc_delete_ptr64_longlong);
    TLI.setUnavailable(LibFunc_msvc_new_array_int);
    TLI.setUnavailable(LibFunc_msvc_new_array_int_nothrow);
    TLI.setUnavailable(LibFunc_msvc_new_array_longlong);
    TLI.setUnavailable(LibFunc_msvc_new_array_longlong_nothrow);
    TLI.setUnavailable(LibFunc_msvc_delete_array_ptr32);
    TLI.setUnavailable(LibFunc_msvc_delete_array_ptr32_nothrow);
    TLI.setUnavailable(LibFunc_msvc_delete_array_ptr32_int);
    TLI.setUnavailable(LibFunc_msvc_delete_array_ptr64);
    TLI.setUnavailable(LibFunc_msvc_delete_array_ptr64_nothrow);
    TLI.setUnavailable(LibFunc_msvc_delete_array_ptr64_longlong);
  }

  switch (T.getOS()) {
  case Triple::MacOSX:
    // exp10 and exp10f are not available on OS X until 10.9 and iOS until 7.0
    // and their names are __exp10 and __exp10f. exp10l is not available on
    // OS X or iOS.
    TLI.setUnavailable(LibFunc_exp10l);
    if (T.isMacOSXVersionLT(10, 9)) {
      TLI.setUnavailable(LibFunc_exp10);
      TLI.setUnavailable(LibFunc_exp10f);
    } else {
      TLI.setAvailableWithName(LibFunc_exp10, "__exp10");
      TLI.setAvailableWithName(LibFunc_exp10f, "__exp10f");
    }
    break;
  case Triple::IOS:
  case Triple::TvOS:
  case Triple::WatchOS:
    TLI.setUnavailable(LibFunc_exp10l);
    if (!T.isWatchOS() &&
        (T.isOSVersionLT(7, 0) || (T.isOSVersionLT(9, 0) && T.isX86()))) {
      TLI.setUnavailable(LibFunc_exp10);
      TLI.setUnavailable(LibFunc_exp10f);
    } else {
      TLI.setAvailableWithName(LibFunc_exp10, "__exp10");
      TLI.setAvailableWithName(LibFunc_exp10f, "__exp10f");
    }
    break;
  case Triple::Linux:
    // exp10, exp10f, exp10l is available on Linux (GLIBC) but are extremely
    // buggy prior to glibc version 2.18. Until this version is widely deployed
    // or we have a reasonable detection strategy, we cannot use exp10 reliably
    // on Linux.
    //
    // Fall through to disable all of them.
    LLVM_FALLTHROUGH;
  default:
    TLI.setUnavailable(LibFunc_exp10);
    TLI.setUnavailable(LibFunc_exp10f);
    TLI.setUnavailable(LibFunc_exp10l);
  }

  // ffsl is available on at least Darwin, Mac OS X, iOS, FreeBSD, and
  // Linux (GLIBC):
  // http://developer.apple.com/library/mac/#documentation/Darwin/Reference/ManPages/man3/ffsl.3.html
  // http://svn.freebsd.org/base/head/lib/libc/string/ffsl.c
  // http://www.gnu.org/software/gnulib/manual/html_node/ffsl.html
  switch (T.getOS()) {
  case Triple::Darwin:
  case Triple::MacOSX:
  case Triple::IOS:
  case Triple::TvOS:
  case Triple::WatchOS:
  case Triple::FreeBSD:
  case Triple::Linux:
    break;
  default:
    TLI.setUnavailable(LibFunc_ffsl);
  }

  // ffsll is available on at least FreeBSD and Linux (GLIBC):
  // http://svn.freebsd.org/base/head/lib/libc/string/ffsll.c
  // http://www.gnu.org/software/gnulib/manual/html_node/ffsll.html
  switch (T.getOS()) {
  case Triple::Darwin:
  case Triple::MacOSX:
  case Triple::IOS:
  case Triple::TvOS:
  case Triple::WatchOS:
  case Triple::FreeBSD:
  case Triple::Linux:
    break;
  default:
    TLI.setUnavailable(LibFunc_ffsll);
  }

  // The following functions are available on at least FreeBSD:
  // http://svn.freebsd.org/base/head/lib/libc/string/fls.c
  // http://svn.freebsd.org/base/head/lib/libc/string/flsl.c
  // http://svn.freebsd.org/base/head/lib/libc/string/flsll.c
  if (!T.isOSFreeBSD()) {
    TLI.setUnavailable(LibFunc_fls);
    TLI.setUnavailable(LibFunc_flsl);
    TLI.setUnavailable(LibFunc_flsll);
  }

  // The following functions are only available on GNU/Linux (using glibc).
  // Linux variants without glibc (eg: bionic, musl) may have some subset.
  if (!T.isOSLinux() || !T.isGNUEnvironment()) {
    TLI.setUnavailable(LibFunc_dunder_strdup);
    TLI.setUnavailable(LibFunc_dunder_strtok_r);
    TLI.setUnavailable(LibFunc_dunder_isoc99_fscanf);                   // INTEL
    TLI.setUnavailable(LibFunc_dunder_isoc99_scanf);
    TLI.setUnavailable(LibFunc_dunder_isoc99_sscanf);
    TLI.setUnavailable(LibFunc_under_IO_getc);
    TLI.setUnavailable(LibFunc_under_IO_putc);
    // But, Android and musl have memalign.
    if (!T.isAndroid() && !T.isMusl())
      TLI.setUnavailable(LibFunc_memalign);
    TLI.setUnavailable(LibFunc_fopen64);
    TLI.setUnavailable(LibFunc_fseeko64);
    TLI.setUnavailable(LibFunc_fstat64);
    TLI.setUnavailable(LibFunc_fstatvfs64);
    TLI.setUnavailable(LibFunc_ftello64);
    TLI.setUnavailable(LibFunc_lstat64);
    TLI.setUnavailable(LibFunc_open64);
    TLI.setUnavailable(LibFunc_stat64);
    TLI.setUnavailable(LibFunc_statvfs64);
    TLI.setUnavailable(LibFunc_tmpfile64);

    // Relaxed math functions are included in math-finite.h on Linux (GLIBC).
    // Note that math-finite.h is no longer supported by top-of-tree GLIBC,
    // so we keep these functions around just so that they're recognized by
    // the ConstantFolder.
    TLI.setUnavailable(LibFunc_acos_finite);
    TLI.setUnavailable(LibFunc_acosf_finite);
    TLI.setUnavailable(LibFunc_acosl_finite);
    TLI.setUnavailable(LibFunc_acosh_finite);
    TLI.setUnavailable(LibFunc_acoshf_finite);
    TLI.setUnavailable(LibFunc_acoshl_finite);
    TLI.setUnavailable(LibFunc_asin_finite);
    TLI.setUnavailable(LibFunc_asinf_finite);
    TLI.setUnavailable(LibFunc_asinl_finite);
    TLI.setUnavailable(LibFunc_atan2_finite);
    TLI.setUnavailable(LibFunc_atan2f_finite);
    TLI.setUnavailable(LibFunc_atan2l_finite);
    TLI.setUnavailable(LibFunc_atanh_finite);
    TLI.setUnavailable(LibFunc_atanhf_finite);
    TLI.setUnavailable(LibFunc_atanhl_finite);
    TLI.setUnavailable(LibFunc_cosh_finite);
    TLI.setUnavailable(LibFunc_coshf_finite);
    TLI.setUnavailable(LibFunc_coshl_finite);
    TLI.setUnavailable(LibFunc_exp10_finite);
    TLI.setUnavailable(LibFunc_exp10f_finite);
    TLI.setUnavailable(LibFunc_exp10l_finite);
    TLI.setUnavailable(LibFunc_exp2_finite);
    TLI.setUnavailable(LibFunc_exp2f_finite);
    TLI.setUnavailable(LibFunc_exp2l_finite);
    TLI.setUnavailable(LibFunc_exp_finite);
    TLI.setUnavailable(LibFunc_expf_finite);
    TLI.setUnavailable(LibFunc_expl_finite);
    TLI.setUnavailable(LibFunc_log10_finite);
    TLI.setUnavailable(LibFunc_log10f_finite);
    TLI.setUnavailable(LibFunc_log10l_finite);
    TLI.setUnavailable(LibFunc_log2_finite);
    TLI.setUnavailable(LibFunc_log2f_finite);
    TLI.setUnavailable(LibFunc_log2l_finite);
    TLI.setUnavailable(LibFunc_log_finite);
    TLI.setUnavailable(LibFunc_logf_finite);
    TLI.setUnavailable(LibFunc_logl_finite);
    TLI.setUnavailable(LibFunc_pow_finite);
    TLI.setUnavailable(LibFunc_powf_finite);
    TLI.setUnavailable(LibFunc_powl_finite);
    TLI.setUnavailable(LibFunc_sinh_finite);
    TLI.setUnavailable(LibFunc_sinhf_finite);
    TLI.setUnavailable(LibFunc_sinhl_finite);
    TLI.setUnavailable(LibFunc_sqrt_finite);
    TLI.setUnavailable(LibFunc_sqrtf_finite);
    TLI.setUnavailable(LibFunc_sqrtl_finite);
  }

  if ((T.isOSLinux() && T.isGNUEnvironment()) ||
      (T.isAndroid() && !T.isAndroidVersionLT(28))) {
    // available IO unlocked variants on GNU/Linux and Android P or later
    TLI.setAvailable(LibFunc_getc_unlocked);
    TLI.setAvailable(LibFunc_getchar_unlocked);
    TLI.setAvailable(LibFunc_putc_unlocked);
    TLI.setAvailable(LibFunc_putchar_unlocked);
    TLI.setAvailable(LibFunc_fputc_unlocked);
    TLI.setAvailable(LibFunc_fgetc_unlocked);
    TLI.setAvailable(LibFunc_fread_unlocked);
    TLI.setAvailable(LibFunc_fwrite_unlocked);
    TLI.setAvailable(LibFunc_fputs_unlocked);
    TLI.setAvailable(LibFunc_fgets_unlocked);
  }

  if (T.isAndroid() && T.isAndroidVersionLT(21)) {
    TLI.setUnavailable(LibFunc_stpcpy);
    TLI.setUnavailable(LibFunc_stpncpy);
  }

  if (T.isPS()) {
    // PS4/PS5 do have memalign.
    TLI.setAvailable(LibFunc_memalign);

    // PS4/PS5 do not have new/delete with "unsigned int" size parameter;
    // they only have the "unsigned long" versions.
    TLI.setUnavailable(LibFunc_ZdaPvj);
    TLI.setUnavailable(LibFunc_ZdaPvjSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZdlPvj);
    TLI.setUnavailable(LibFunc_ZdlPvjSt11align_val_t);
    TLI.setUnavailable(LibFunc_Znaj);
    TLI.setUnavailable(LibFunc_ZnajRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZnajSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZnajSt11align_val_tRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_Znwj);
    TLI.setUnavailable(LibFunc_ZnwjRKSt9nothrow_t);
    TLI.setUnavailable(LibFunc_ZnwjSt11align_val_t);
    TLI.setUnavailable(LibFunc_ZnwjSt11align_val_tRKSt9nothrow_t);

    // None of the *_chk functions.
    TLI.setUnavailable(LibFunc_memccpy_chk);
    TLI.setUnavailable(LibFunc_memcpy_chk);
    TLI.setUnavailable(LibFunc_memmove_chk);
    TLI.setUnavailable(LibFunc_mempcpy_chk);
    TLI.setUnavailable(LibFunc_memset_chk);
    TLI.setUnavailable(LibFunc_snprintf_chk);
    TLI.setUnavailable(LibFunc_sprintf_chk);
    TLI.setUnavailable(LibFunc_stpcpy_chk);
    TLI.setUnavailable(LibFunc_stpncpy_chk);
    TLI.setUnavailable(LibFunc_strcat_chk);
    TLI.setUnavailable(LibFunc_strcpy_chk);
    TLI.setUnavailable(LibFunc_strlcat_chk);
    TLI.setUnavailable(LibFunc_strlcpy_chk);
    TLI.setUnavailable(LibFunc_strlen_chk);
    TLI.setUnavailable(LibFunc_strncat_chk);
    TLI.setUnavailable(LibFunc_strncpy_chk);
    TLI.setUnavailable(LibFunc_vsnprintf_chk);
    TLI.setUnavailable(LibFunc_vsprintf_chk);

    // Various Posix system functions.
    TLI.setUnavailable(LibFunc_access);
    TLI.setUnavailable(LibFunc_chmod);
    TLI.setUnavailable(LibFunc_chown);
    TLI.setUnavailable(LibFunc_closedir);
    TLI.setUnavailable(LibFunc_ctermid);
    TLI.setUnavailable(LibFunc_execl);
    TLI.setUnavailable(LibFunc_execle);
    TLI.setUnavailable(LibFunc_execlp);
    TLI.setUnavailable(LibFunc_execv);
    TLI.setUnavailable(LibFunc_execvP);
    TLI.setUnavailable(LibFunc_execve);
    TLI.setUnavailable(LibFunc_execvp);
    TLI.setUnavailable(LibFunc_execvpe);
    TLI.setUnavailable(LibFunc_fork);
    TLI.setUnavailable(LibFunc_fstat);
    TLI.setUnavailable(LibFunc_fstatvfs);
    TLI.setUnavailable(LibFunc_getenv);
    TLI.setUnavailable(LibFunc_getitimer);
    TLI.setUnavailable(LibFunc_getlogin_r);
    TLI.setUnavailable(LibFunc_getpwnam);
    TLI.setUnavailable(LibFunc_gettimeofday);
    TLI.setUnavailable(LibFunc_lchown);
    TLI.setUnavailable(LibFunc_lstat);
    TLI.setUnavailable(LibFunc_mkdir);
    TLI.setUnavailable(LibFunc_open);
    TLI.setUnavailable(LibFunc_opendir);
    TLI.setUnavailable(LibFunc_pclose);
    TLI.setUnavailable(LibFunc_popen);
    TLI.setUnavailable(LibFunc_pread);
    TLI.setUnavailable(LibFunc_pwrite);
    TLI.setUnavailable(LibFunc_read);
    TLI.setUnavailable(LibFunc_readlink);
    TLI.setUnavailable(LibFunc_realpath);
    TLI.setUnavailable(LibFunc_rename);
    TLI.setUnavailable(LibFunc_rmdir);
    TLI.setUnavailable(LibFunc_setitimer);
    TLI.setUnavailable(LibFunc_stat);
    TLI.setUnavailable(LibFunc_statvfs);
    TLI.setUnavailable(LibFunc_system);
    TLI.setUnavailable(LibFunc_times);
    TLI.setUnavailable(LibFunc_tmpfile);
    TLI.setUnavailable(LibFunc_unlink);
    TLI.setUnavailable(LibFunc_uname);
    TLI.setUnavailable(LibFunc_unsetenv);
    TLI.setUnavailable(LibFunc_utime);
    TLI.setUnavailable(LibFunc_utimes);
    TLI.setUnavailable(LibFunc_valloc);
    TLI.setUnavailable(LibFunc_write);

    // Miscellaneous other functions not provided.
    TLI.setUnavailable(LibFunc_atomic_load);
    TLI.setUnavailable(LibFunc_atomic_store);
    TLI.setUnavailable(LibFunc___kmpc_alloc_shared);
    TLI.setUnavailable(LibFunc___kmpc_free_shared);
    TLI.setUnavailable(LibFunc_dunder_strndup);
    TLI.setUnavailable(LibFunc_bcmp);
    TLI.setUnavailable(LibFunc_bcopy);
    TLI.setUnavailable(LibFunc_bzero);
    TLI.setUnavailable(LibFunc_cabs);
    TLI.setUnavailable(LibFunc_cabsf);
    TLI.setUnavailable(LibFunc_cabsl);
    TLI.setUnavailable(LibFunc_ffs);
    TLI.setUnavailable(LibFunc_flockfile);
    TLI.setUnavailable(LibFunc_fseeko);
    TLI.setUnavailable(LibFunc_ftello);
    TLI.setUnavailable(LibFunc_ftrylockfile);
    TLI.setUnavailable(LibFunc_funlockfile);
    TLI.setUnavailable(LibFunc_htonl);
    TLI.setUnavailable(LibFunc_htons);
    TLI.setUnavailable(LibFunc_isascii);
    TLI.setUnavailable(LibFunc_memccpy);
    TLI.setUnavailable(LibFunc_mempcpy);
    TLI.setUnavailable(LibFunc_memrchr);
    TLI.setUnavailable(LibFunc_ntohl);
    TLI.setUnavailable(LibFunc_ntohs);
    TLI.setUnavailable(LibFunc_reallocf);
    TLI.setUnavailable(LibFunc_roundeven);
    TLI.setUnavailable(LibFunc_roundevenf);
    TLI.setUnavailable(LibFunc_roundevenl);
    TLI.setUnavailable(LibFunc_stpcpy);
    TLI.setUnavailable(LibFunc_stpncpy);
    TLI.setUnavailable(LibFunc_strlcat);
    TLI.setUnavailable(LibFunc_strlcpy);
    TLI.setUnavailable(LibFunc_strndup);
    TLI.setUnavailable(LibFunc_strnlen);
    TLI.setUnavailable(LibFunc_toascii);
  }

  // As currently implemented in clang, NVPTX code has no standard library to
  // speak of.  Headers provide a standard-ish library implementation, but many
  // of the signatures are wrong -- for example, many libm functions are not
  // extern "C".
  //
  // libdevice, an IR library provided by nvidia, is linked in by the front-end,
  // but only used functions are provided to llvm.  Moreover, most of the
  // functions in libdevice don't map precisely to standard library functions.
  //
  // FIXME: Having no standard library prevents e.g. many fastmath
  // optimizations, so this situation should be fixed.
  if (T.isNVPTX()) {
    TLI.disableAllFunctions();
    TLI.setAvailable(LibFunc_nvvm_reflect);
    TLI.setAvailable(llvm::LibFunc_malloc);
    TLI.setAvailable(llvm::LibFunc_free);

    // TODO: We could enable the following two according to [0] but we haven't
    //       done an evaluation wrt. the performance implications.
    // [0]
    // https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#dynamic-global-memory-allocation-and-operations
    //
    //    TLI.setAvailable(llvm::LibFunc_memcpy);
    //    TLI.setAvailable(llvm::LibFunc_memset);

    TLI.setAvailable(llvm::LibFunc___kmpc_alloc_shared);
    TLI.setAvailable(llvm::LibFunc___kmpc_free_shared);
  } else {
    TLI.setUnavailable(LibFunc_nvvm_reflect);
  }

#if INTEL_CUSTOMIZATION
  // Windows specific
  if (!T.isOSWindows()) {
    TLI.setUnavailable(LibFunc_acrt_iob_func);
    TLI.setUnavailable(LibFunc_atexit);
    TLI.setUnavailable(LibFunc_tunder_mb_cur_max_func);
    TLI.setUnavailable(LibFunc_dunder_CxxFrameHandler3);
    TLI.setUnavailable(LibFunc_dunder_RTDynamicCast);
    TLI.setUnavailable(LibFunc_dunder_RTtypeid);
    TLI.setUnavailable(LibFunc_dunder_std_terminate);
    TLI.setUnavailable(LibFunc_dunder_std_type_info_compare);
    TLI.setUnavailable(LibFunc_dunder_std_type_info_name);
    TLI.setUnavailable(LibFunc_CloseHandle);
    TLI.setUnavailable(LibFunc_ConvertThreadToFiber);
    TLI.setUnavailable(LibFunc_CreateFiber);
    TLI.setUnavailable(LibFunc_CreateFileA);
    TLI.setUnavailable(LibFunc_CreateFileW);
    TLI.setUnavailable(LibFunc_DeleteFiber);
    TLI.setUnavailable(LibFunc_DeleteCriticalSection);
    TLI.setUnavailable(LibFunc_EnterCriticalSection);
    TLI.setUnavailable(LibFunc_FindClose);
    TLI.setUnavailable(LibFunc_FindFirstFileA);
    TLI.setUnavailable(LibFunc_FindFirstFileW);
    TLI.setUnavailable(LibFunc_FindNextFileA);
    TLI.setUnavailable(LibFunc_FindNextFileW);
    TLI.setUnavailable(LibFunc_FindResourceA);
    TLI.setUnavailable(LibFunc_FormatMessageA);
    TLI.setUnavailable(LibFunc_FreeResource);
    TLI.setUnavailable(LibFunc_GetCurrentDirectoryA);
    TLI.setUnavailable(LibFunc_GetCurrentDirectoryW);
    TLI.setUnavailable(LibFunc_GetCurrentProcess);
    TLI.setUnavailable(LibFunc_GetCurrentThreadId);
    TLI.setUnavailable(LibFunc_GetFullPathNameA);
    TLI.setUnavailable(LibFunc_GetFullPathNameW);
    TLI.setUnavailable(LibFunc_GetLastError);
    TLI.setUnavailable(LibFunc_GetModuleFileNameA);
    TLI.setUnavailable(LibFunc_GetModuleHandleA);
    TLI.setUnavailable(LibFunc_GetProcAddress);
    TLI.setUnavailable(LibFunc_GetProcessTimes);
    TLI.setUnavailable(LibFunc_GetShortPathNameW);
    TLI.setUnavailable(LibFunc_GetSystemTime);
    TLI.setUnavailable(LibFunc_GetVersionExA);
    TLI.setUnavailable(LibFunc_GlobalMemoryStatus);
    TLI.setUnavailable(LibFunc_InitializeCriticalSection);
    TLI.setUnavailable(LibFunc_InitializeCriticalSectionAndSpinCount);
    TLI.setUnavailable(LibFunc_LeaveCriticalSection);
    TLI.setUnavailable(LibFunc_LoadLibraryA);
    TLI.setUnavailable(LibFunc_LoadResource);
    TLI.setUnavailable(LibFunc_LocalFree);
    TLI.setUnavailable(LibFunc_LockResource);
    TLI.setUnavailable(LibFunc_MultiByteToWideChar);
    TLI.setUnavailable(LibFunc_QueryPerformanceCounter);
    TLI.setUnavailable(LibFunc_ReadFile);
    TLI.setUnavailable(LibFunc_SetFilePointer);
    TLI.setUnavailable(LibFunc_SizeofResource);
    TLI.setUnavailable(LibFunc_Sleep);
    TLI.setUnavailable(LibFunc_SwitchToFiber);
    TLI.setUnavailable(LibFunc_SystemTimeToFileTime);
    TLI.setUnavailable(LibFunc_WideCharToMultiByte);
    TLI.setUnavailable(LibFunc_WriteFile);
    TLI.setUnavailable(LibFunc_islower);
    TLI.setUnavailable(LibFunc_isprint);
    TLI.setUnavailable(LibFunc_isxdigit);
    TLI.setUnavailable(LibFunc_local_stdio_printf_options);
    TLI.setUnavailable(LibFunc_local_stdio_scanf_options);
    TLI.setUnavailable(LibFunc_msvc_std_bad_alloc_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_bad_alloc_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_bad_array_new_length_ctor); 
    TLI.setUnavailable(LibFunc_msvc_std_bad_array_new_length_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_under_Unlock);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_imbue);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_overflow);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_pbackfail);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_setbuf);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_std_fpos_seekoff);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_std_fpos_seekpos);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_sync);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_uflow);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_under_Endwrite);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_under_Lock);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_underflow);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_xsgetn);
    TLI.setUnavailable(LibFunc_msvc_std_basic_filebuf_xsputn);
    TLI.setUnavailable(LibFunc_msvc_std_basic_ios_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_istream_vector_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_ostream_vector_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_imbue);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_under_Lock);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_overflow);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_pbackfail);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_setbuf);
    TLI.setUnavailable(LibFunc_msvc_std_basic_ios_setstate);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_showmanyc);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_std_fpos_seekoff);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_std_fpos_seekpos);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_sync);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_uflow);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_underflow);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_under_Unlock);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_xsgetn);
    TLI.setUnavailable(LibFunc_msvc_std_basic_streambuf_xsputn);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_append);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_append_size_value);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_assign_const_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_assign_const_ptr_size);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_ptr64_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_insert);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_operator_equal_const_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_operator_equal_ptr64);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_operator_plus_equal_char);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_push_back);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_resize);
    TLI.setUnavailable(LibFunc_msvc_std_basic_string_under_xlen);
    TLI.setUnavailable(LibFunc_msvc_std_codecvt_do_always_noconv);
    TLI.setUnavailable(LibFunc_msvc_std_codecvt_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_codecvt_use_facet);
    TLI.setUnavailable(LibFunc_msvc_std_codecvt_do_encoding);
    TLI.setUnavailable(LibFunc_msvc_std_codecvt_do_in);
    TLI.setUnavailable(LibFunc_msvc_std_codecvt_do_length);
    TLI.setUnavailable(LibFunc_msvc_std_codecvt_do_max_length);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_do_narrow_char_char);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_do_narrow_ptr_ptr_char_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_codecvt_do_out);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_do_tolower_char);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_do_tolower_ptr_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_do_toupper_char);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_do_toupper_ptr_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_codecvt_do_unshift);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_do_widen_char);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_do_widen_ptr_ptr_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_ctype_use_facet);
    TLI.setUnavailable(LibFunc_msvc_std_CxxThrowException);
    TLI.setUnavailable(LibFunc_msvc_std_error_category_default_error);
    TLI.setUnavailable(LibFunc_msvc_std_error_category_equivalent_error_code);
    TLI.setUnavailable(LibFunc_msvc_std_error_category_equivalent_error_condition);
    TLI.setUnavailable(LibFunc_msvc_std_error_code_make_error_code);
    TLI.setUnavailable(LibFunc_msvc_std_Execute_once);
    TLI.setUnavailable(LibFunc_msvc_std_exception_const_ptr_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_exception_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_exception_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_exception_what);
    TLI.setUnavailable(LibFunc_msvc_std_facet_register);
    TLI.setUnavailable(LibFunc_msvc_std_under_Fiopen);
    TLI.setUnavailable(LibFunc_msvc_std_ios_base_under_Ios_base_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_ios_base_failure);
    TLI.setUnavailable(LibFunc_msvc_std_ios_base_failure_const_ptr_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_ios_base_failure_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_ios_base_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_istreambuf_iterator_operator_plus_plus);
    TLI.setUnavailable(LibFunc_msvc_std_locale_facet_decref);
    TLI.setUnavailable(LibFunc_msvc_std_locale_facet_incref);
    TLI.setUnavailable(LibFunc_msvc_std_locale_under_Init);
    TLI.setUnavailable(LibFunc_msvc_std_lockit);
    TLI.setUnavailable(LibFunc_msvc_std_lockit_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_locimp_Getgloballocale);
    TLI.setUnavailable(LibFunc_msvc_std_locinfo_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_locinfo_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_bool_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_double_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_float_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_long_double_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_long_long_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_long_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_unsigned_int_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_unsigned_long_long_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_unsigned_long_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_unsigned_short_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_do_get_void_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_under_Getffld);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_under_Getifld);
    TLI.setUnavailable(LibFunc_msvc_std_num_get_use_facet);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_do_put_bool);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_do_put_double);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_do_put_long);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_do_put_long_double);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_do_put_long_long);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_do_put_ulong);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_do_put_ulong_long);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_do_put_void_ptr);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_ostreambuf_iterator_Fput);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_ostreambuf_iterator_iput);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_ostreambuf_iterator_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_numpunct_do_decimal_point);
    TLI.setUnavailable(LibFunc_msvc_std_numpunct_do_falsename);
    TLI.setUnavailable(LibFunc_msvc_std_numpunct_do_grouping);
    TLI.setUnavailable(LibFunc_msvc_std_numpunct_do_thousands_sep);
    TLI.setUnavailable(LibFunc_msvc_std_numpunct_do_truename);
    TLI.setUnavailable(LibFunc_msvc_std_numpunct_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_numpunct_use_facet);
    TLI.setUnavailable(LibFunc_msvc_std_num_put_use_facet);
    TLI.setUnavailable(LibFunc_msvc_std_numpunct_Tidy);
    TLI.setUnavailable(LibFunc_msvc_std_runtime_error_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_runtime_error_char_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_runtime_error_ptr64_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_runtime_error_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_system_error_const_ptr_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_system_error_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_Syserror_map);
    TLI.setUnavailable(LibFunc_msvc_std_under_system_error_const_ptr_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_uncaught_exception);
    TLI.setUnavailable(LibFunc_msvc_std_under_generic_error_category_message);
    TLI.setUnavailable(LibFunc_msvc_std_under_immortalize_impl);
    TLI.setUnavailable(LibFunc_msvc_std_under_iostreamer_error_category_message);
    TLI.setUnavailable(LibFunc_msvc_std_under_iostreamer_error_category_name);
    TLI.setUnavailable(LibFunc_msvc_std_under_iostreamer_error_category_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_under_iostreamer_error_category2_message);
    TLI.setUnavailable(LibFunc_msvc_std_under_iostreamer_error_category2_name);
    TLI.setUnavailable(LibFunc_msvc_std_under_iostreamer_error_category2_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_under_locinfo_ctor);
    TLI.setUnavailable(LibFunc_msvc_std_under_locinfo_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_under_system_error_scalar_deleting_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_under_Throw_bad_array_new_length);
    TLI.setUnavailable(LibFunc_msvc_std_under_Xlen_string);
    TLI.setUnavailable(LibFunc_msvc_std_Xbad_alloc);
    TLI.setUnavailable(LibFunc_msvc_std_Xout_of_range);
    TLI.setUnavailable(LibFunc_msvc_std_Xlength_error);
    TLI.setUnavailable(LibFunc_msvc_std_Xran);
    TLI.setUnavailable(LibFunc_msvc_std_Xruntime_error);
    TLI.setUnavailable(LibFunc_msvc_std_yarn_dtor);
    TLI.setUnavailable(LibFunc_msvc_std_yarn_wchar_dtor);
    TLI.setUnavailable(LibFunc_sprintf_s);
    TLI.setUnavailable(LibFunc_stdio_common_vfprintf);
    TLI.setUnavailable(LibFunc_stdio_common_vfscanf);
    TLI.setUnavailable(LibFunc_stdio_common_vsprintf);
    TLI.setUnavailable(LibFunc_dunder_stdio_common_vsprintf_s);
    TLI.setUnavailable(LibFunc_stdio_common_vsscanf);
    TLI.setUnavailable(LibFunc_std_exception_copy);
    TLI.setUnavailable(LibFunc_std_exception_destroy);
    TLI.setUnavailable(LibFunc_std_reverse_trivially_swappable_8);
    TLI.setUnavailable(LibFunc_strncpy_s);
    TLI.setUnavailable(LibFunc_terminate);
    TLI.setUnavailable(LibFunc_under_Getctype);
    TLI.setUnavailable(LibFunc_under_Getcvt);
    TLI.setUnavailable(LibFunc_under_Init_thread_abort);
    TLI.setUnavailable(LibFunc_under_Stollx);
    TLI.setUnavailable(LibFunc_under_Stolx);
    TLI.setUnavailable(LibFunc_under_Stoullx);
    TLI.setUnavailable(LibFunc_under_Stoulx);
    TLI.setUnavailable(LibFunc_under_Tolower);
    TLI.setUnavailable(LibFunc_under_Toupper);
    TLI.setUnavailable(LibFunc_under_chdir);
    TLI.setUnavailable(LibFunc_under_commit);
    TLI.setUnavailable(LibFunc_under_close);
    TLI.setUnavailable(LibFunc_under_errno);
    TLI.setUnavailable(LibFunc_under_fdopen);
    TLI.setUnavailable(LibFunc_under_fileno);
    TLI.setUnavailable(LibFunc_under_findclose);
    TLI.setUnavailable(LibFunc_under_findfirst64i32);
    TLI.setUnavailable(LibFunc_under_findnext64i32);
    TLI.setUnavailable(LibFunc_under_fseeki64);
    TLI.setUnavailable(LibFunc_under_fstat64);
    TLI.setUnavailable(LibFunc_under_fstat64i32);
    TLI.setUnavailable(LibFunc_under_ftelli64);
    TLI.setUnavailable(LibFunc_under_ftime64);
    TLI.setUnavailable(LibFunc_under_invalid_parameter_noinfo_noreturn);
    TLI.setUnavailable(LibFunc_under_localtime64);
    TLI.setUnavailable(LibFunc_under_lock_file);
    TLI.setUnavailable(LibFunc_under_lseeki64);
    TLI.setUnavailable(LibFunc_under_difftime64);
    TLI.setUnavailable(LibFunc_under_get_stream_buffer_pointers);
    TLI.setUnavailable(LibFunc_under_getcwd);
    TLI.setUnavailable(LibFunc_under_getdcwd);
    TLI.setUnavailable(LibFunc_under_getdrive);
    TLI.setUnavailable(LibFunc_under_getpid);
    TLI.setUnavailable(LibFunc_under_gmtime64);
    TLI.setUnavailable(LibFunc_under_Init_thread_footer);
    TLI.setUnavailable(LibFunc_under_Init_thread_header);
    TLI.setUnavailable(LibFunc_under_mkdir);
    TLI.setUnavailable(LibFunc_under_purecall);
    TLI.setUnavailable(LibFunc_under_read);
    TLI.setUnavailable(LibFunc_under_set_errno);
    TLI.setUnavailable(LibFunc_under_setmode);
    TLI.setUnavailable(LibFunc_under_sleep);
    TLI.setUnavailable(LibFunc_under_stat64);
    TLI.setUnavailable(LibFunc_under_stat64i32);
    TLI.setUnavailable(LibFunc_under_stricmp);
    TLI.setUnavailable(LibFunc_under_strnicmp);
    TLI.setUnavailable(LibFunc_under_strtoi64);
    TLI.setUnavailable(LibFunc_under_time64);
    TLI.setUnavailable(LibFunc_under_unlink);
    TLI.setUnavailable(LibFunc_under_unlock_file);
    TLI.setUnavailable(LibFunc_under_waccess);
    TLI.setUnavailable(LibFunc_under_wassert);
    TLI.setUnavailable(LibFunc_under_wfopen);
    TLI.setUnavailable(LibFunc_under_wopen);
    TLI.setUnavailable(LibFunc_under_wremove);
    TLI.setUnavailable(LibFunc_under_write);
    TLI.setUnavailable(LibFunc_under_wstat64);
    TLI.setUnavailable(LibFunc_wcscpy);
    TLI.setUnavailable(LibFunc_wcsncat);
  }
#endif // INTEL_CUSTOMIZATION

  // These vec_malloc/free routines are only available on AIX.
  if (!T.isOSAIX()) {
    TLI.setUnavailable(LibFunc_vec_calloc);
    TLI.setUnavailable(LibFunc_vec_malloc);
    TLI.setUnavailable(LibFunc_vec_realloc);
    TLI.setUnavailable(LibFunc_vec_free);
  }

  TLI.addVectorizableFunctionsFromVecLib(ClVectorLibrary);
}

TargetLibraryInfoImpl::TargetLibraryInfoImpl() {
  // Default to everything being available.
  memset(AvailableArray, -1, sizeof(AvailableArray));

  initialize(*this, Triple(), StandardNames);
}

TargetLibraryInfoImpl::TargetLibraryInfoImpl(const Triple &T) {
  // Default to everything being available.
  memset(AvailableArray, -1, sizeof(AvailableArray));

  initialize(*this, T, StandardNames);
}

TargetLibraryInfoImpl::TargetLibraryInfoImpl(const TargetLibraryInfoImpl &TLI)
    : CustomNames(TLI.CustomNames), ShouldExtI32Param(TLI.ShouldExtI32Param),
      ShouldExtI32Return(TLI.ShouldExtI32Return),
      ShouldSignExtI32Param(TLI.ShouldSignExtI32Param),
#if INTEL_CUSTOMIZATION
      SizeOfInt(TLI.SizeOfInt),
      CurVectorLibrary(TLI.CurVectorLibrary) {
#else // INTEL_CUSTOMIZATION
      SizeOfInt(TLI.SizeOfInt) {
#endif // INTEL_CUSTOMIZATION
  memcpy(AvailableArray, TLI.AvailableArray, sizeof(AvailableArray));
  VectorDescs = TLI.VectorDescs;
  ScalarDescs = TLI.ScalarDescs;
}

TargetLibraryInfoImpl::TargetLibraryInfoImpl(TargetLibraryInfoImpl &&TLI)
    : CustomNames(std::move(TLI.CustomNames)),
      ShouldExtI32Param(TLI.ShouldExtI32Param),
      ShouldExtI32Return(TLI.ShouldExtI32Return),
      ShouldSignExtI32Param(TLI.ShouldSignExtI32Param),
#if INTEL_CUSTOMIZATION
      SizeOfInt(TLI.SizeOfInt),
      CurVectorLibrary(TLI.CurVectorLibrary) {
#else // INTEL_CUSTOMIZATION
      SizeOfInt(TLI.SizeOfInt) {
#endif // INTEL_CUSTOMIZATION
  std::move(std::begin(TLI.AvailableArray), std::end(TLI.AvailableArray),
            AvailableArray);
  VectorDescs = TLI.VectorDescs;
  ScalarDescs = TLI.ScalarDescs;
}

TargetLibraryInfoImpl &TargetLibraryInfoImpl::operator=(const TargetLibraryInfoImpl &TLI) {
  CustomNames = TLI.CustomNames;
  ShouldExtI32Param = TLI.ShouldExtI32Param;
  ShouldExtI32Return = TLI.ShouldExtI32Return;
  ShouldSignExtI32Param = TLI.ShouldSignExtI32Param;
#if INTEL_CUSTOMIZATION
  CurVectorLibrary = TLI.CurVectorLibrary;
  VectorDescs = TLI.VectorDescs;
  ScalarDescs = TLI.ScalarDescs;
#endif // INTEL_CUSTOMIZATION
  SizeOfInt = TLI.SizeOfInt;
  memcpy(AvailableArray, TLI.AvailableArray, sizeof(AvailableArray));
  return *this;
}

TargetLibraryInfoImpl &TargetLibraryInfoImpl::operator=(TargetLibraryInfoImpl &&TLI) {
  CustomNames = std::move(TLI.CustomNames);
  ShouldExtI32Param = TLI.ShouldExtI32Param;
  ShouldExtI32Return = TLI.ShouldExtI32Return;
  ShouldSignExtI32Param = TLI.ShouldSignExtI32Param;
#if INTEL_CUSTOMIZATION
  CurVectorLibrary = TLI.CurVectorLibrary;
  VectorDescs = std::move(TLI.VectorDescs);
  ScalarDescs = std::move(TLI.ScalarDescs);
#endif // INTEL_CUSTOMIZATION
  SizeOfInt = TLI.SizeOfInt;
  std::move(std::begin(TLI.AvailableArray), std::end(TLI.AvailableArray),
            AvailableArray);
  return *this;
}

static StringRef sanitizeFunctionName(StringRef funcName) {
  // Filter out empty names and names containing null bytes, those can't be in
  // our table.
  if (funcName.empty() || funcName.contains('\0'))
    return StringRef();

  // Check for \01 prefix that is used to mangle __asm declarations and
  // strip it if present.
  return GlobalValue::dropLLVMManglingEscape(funcName);
}

bool TargetLibraryInfoImpl::getLibFunc(StringRef funcName, LibFunc &F) const {
  funcName = sanitizeFunctionName(funcName);
  if (funcName.empty())
    return false;

  const auto *Start = std::begin(StandardNames);
  const auto *End = std::end(StandardNames);
  const auto *I = std::lower_bound(Start, End, funcName);
  if (I != End && *I == funcName) {
    F = (LibFunc)(I - Start);
    return true;
  }
  return false;
}

bool TargetLibraryInfoImpl::isValidProtoForLibFunc(const FunctionType &FTy,
                                                   LibFunc F,
                                                   const Module &M) const {
  // FIXME: There is really no guarantee that sizeof(size_t) is equal to
  // sizeof(int*) for every target. So the assumption used here to derive the
  // SizeTBits based on the size of an integer pointer in address space zero
  // isn't always valid.
  unsigned SizeTBits = M.getDataLayout().getPointerSizeInBits(/*AddrSpace=*/0);
  unsigned NumParams = FTy.getNumParams();

  switch (F) {
  case LibFunc_execl:
  case LibFunc_execlp:
  case LibFunc_execle:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getReturnType()->isIntegerTy(32));
  case LibFunc_execv:
  case LibFunc_execvp:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getReturnType()->isIntegerTy(32));
  case LibFunc_execvP:
  case LibFunc_execvpe:
  case LibFunc_execve:
    return (NumParams == 3 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getReturnType()->isIntegerTy(32));
  case LibFunc_strlen_chk:
    --NumParams;
    if (!FTy.getParamType(NumParams)->isIntegerTy(SizeTBits))
      return false;
    LLVM_FALLTHROUGH;
  case LibFunc_strlen:
    return NumParams == 1 && FTy.getParamType(0)->isPointerTy() &&
           FTy.getReturnType()->isIntegerTy(SizeTBits);

  case LibFunc_strchr:
  case LibFunc_strrchr:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0) == FTy.getReturnType() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_strtol:
  case LibFunc_strtod:
  case LibFunc_strtof:
  case LibFunc_strtoul:
  case LibFunc_strtoll:
  case LibFunc_strtold:
  case LibFunc_strtoull:
    return ((NumParams == 2 || NumParams == 3) &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_strcat_chk:
    --NumParams;
    if (!FTy.getParamType(NumParams)->isIntegerTy(SizeTBits))
      return false;
    LLVM_FALLTHROUGH;
  case LibFunc_strcat:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0) == FTy.getReturnType() &&
            FTy.getParamType(1) == FTy.getReturnType());

  case LibFunc_strncat_chk:
    --NumParams;
    if (!FTy.getParamType(NumParams)->isIntegerTy(SizeTBits))
      return false;
    LLVM_FALLTHROUGH;
  case LibFunc_strncat:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0) == FTy.getReturnType() &&
            FTy.getParamType(1) == FTy.getReturnType() &&
            FTy.getParamType(2)->isIntegerTy(SizeTBits));

  case LibFunc_strcpy_chk:
  case LibFunc_stpcpy_chk:
    --NumParams;
    if (!FTy.getParamType(NumParams)->isIntegerTy(SizeTBits))
      return false;
    LLVM_FALLTHROUGH;
  case LibFunc_strcpy:
  case LibFunc_stpcpy:
    return (NumParams == 2 && FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getParamType(0) == FTy.getParamType(1) &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_strlcat_chk:
  case LibFunc_strlcpy_chk:
    --NumParams;
    if (!FTy.getParamType(NumParams)->isIntegerTy(SizeTBits))
      return false;
    LLVM_FALLTHROUGH;
  case LibFunc_strlcat:
  case LibFunc_strlcpy:
    return NumParams == 3 && FTy.getReturnType()->isIntegerTy(SizeTBits) &&
           FTy.getParamType(0)->isPointerTy() &&
           FTy.getParamType(1)->isPointerTy() &&
           FTy.getParamType(2)->isIntegerTy(SizeTBits);

  case LibFunc_strncpy_chk:
  case LibFunc_stpncpy_chk:
    --NumParams;
    if (!FTy.getParamType(NumParams)->isIntegerTy(SizeTBits))
      return false;
    LLVM_FALLTHROUGH;
  case LibFunc_strncpy:
  case LibFunc_stpncpy:
    return (NumParams == 3 && FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getParamType(0) == FTy.getParamType(1) &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy(SizeTBits));

  case LibFunc_strxfrm:
    return (NumParams == 3 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_strcmp:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy(32) &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(0) == FTy.getParamType(1));

  case LibFunc_strncmp:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy(32) &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(0) == FTy.getParamType(1) &&
            FTy.getParamType(2)->isIntegerTy(SizeTBits));

  case LibFunc_strspn:
  case LibFunc_strcspn:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(0) == FTy.getParamType(1) &&
            FTy.getReturnType()->isIntegerTy());

  case LibFunc_strcoll:
  case LibFunc_strcasecmp:
  case LibFunc_strncasecmp:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_strstr:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_strpbrk:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getParamType(0) == FTy.getParamType(1));

  case LibFunc_strtok:
  case LibFunc_strtok_r:
    return (NumParams >= 2 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_scanf:
  case LibFunc_setbuf:
  case LibFunc_setvbuf:
    return (NumParams >= 1 && FTy.getParamType(0)->isPointerTy());
  case LibFunc_strdup:
  case LibFunc_strndup:
    return (NumParams >= 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());
  case LibFunc_sscanf:
  case LibFunc_stat:
  case LibFunc_statvfs:
  case LibFunc_siprintf:
  case LibFunc_small_sprintf:
  case LibFunc_sprintf:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getReturnType()->isIntegerTy(32));

#if INTEL_CUSTOMIZATION
  case LibFunc_sprintf_s:
    return (NumParams >= 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());
#endif // INTEL_CUSTOMIZATION

  case LibFunc_sprintf_chk:
    return NumParams == 4 && FTy.getParamType(0)->isPointerTy() &&
           FTy.getParamType(1)->isIntegerTy(32) &&
           FTy.getParamType(2)->isIntegerTy(SizeTBits) &&
           FTy.getParamType(3)->isPointerTy() &&
           FTy.getReturnType()->isIntegerTy(32);

  case LibFunc_snprintf:
    return NumParams == 3 && FTy.getParamType(0)->isPointerTy() &&
           FTy.getParamType(1)->isIntegerTy(SizeTBits) &&
           FTy.getParamType(2)->isPointerTy() &&
           FTy.getReturnType()->isIntegerTy(32);

  case LibFunc_snprintf_chk:
    return NumParams == 5 && FTy.getParamType(0)->isPointerTy() &&
           FTy.getParamType(1)->isIntegerTy(SizeTBits) &&
           FTy.getParamType(2)->isIntegerTy(32) &&
           FTy.getParamType(3)->isIntegerTy(SizeTBits) &&
           FTy.getParamType(4)->isPointerTy() &&
           FTy.getReturnType()->isIntegerTy(32);

  case LibFunc_setitimer:
    return (NumParams == 3 && FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());
  case LibFunc_system:
    return (NumParams == 1 && FTy.getParamType(0)->isPointerTy());
  case LibFunc___kmpc_alloc_shared:
    return NumParams == 1 && FTy.getReturnType()->isPointerTy();
  case LibFunc_malloc:
  case LibFunc_vec_malloc:
    return NumParams == 1 && FTy.getParamType(0)->isIntegerTy(SizeTBits) &&
           FTy.getReturnType()->isPointerTy();
  case LibFunc_memcmp:
    return NumParams == 3 && FTy.getReturnType()->isIntegerTy(32) &&
           FTy.getParamType(0)->isPointerTy() &&
           FTy.getParamType(1)->isPointerTy() &&
           FTy.getParamType(2)->isIntegerTy(SizeTBits);

  case LibFunc_memchr:
  case LibFunc_memrchr:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getParamType(1)->isIntegerTy(32) &&
            FTy.getParamType(2)->isIntegerTy(SizeTBits));
  case LibFunc_modf:
  case LibFunc_modff:
  case LibFunc_modfl:
    return (NumParams >= 2 && FTy.getParamType(1)->isPointerTy());

  case LibFunc_memcpy_chk:
  case LibFunc_mempcpy_chk:
  case LibFunc_memmove_chk:
    --NumParams;
    if (!FTy.getParamType(NumParams)->isIntegerTy(SizeTBits))
      return false;
    LLVM_FALLTHROUGH;
  case LibFunc_memcpy:
  case LibFunc_mempcpy:
  case LibFunc_memmove:
    return (NumParams == 3 && FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy(SizeTBits));

  case LibFunc_memset_chk:
    --NumParams;
    if (!FTy.getParamType(NumParams)->isIntegerTy(SizeTBits))
      return false;
    LLVM_FALLTHROUGH;
  case LibFunc_memset:
    return (NumParams == 3 && FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy(SizeTBits));

  case LibFunc_memccpy_chk:
      --NumParams;
    if (!FTy.getParamType(NumParams)->isIntegerTy(SizeTBits))
      return false;
    LLVM_FALLTHROUGH;
  case LibFunc_memccpy:
    return (NumParams >= 2 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_memalign:
    return (FTy.getReturnType()->isPointerTy());
  case LibFunc_realloc:
  case LibFunc_reallocf:
  case LibFunc_vec_realloc:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0) == FTy.getReturnType() &&
            FTy.getParamType(1)->isIntegerTy(SizeTBits));
#if INTEL_CUSTOMIZATION
  case LibFunc_re_compile_fastmap:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());
  case LibFunc_re_search_2:
    return (NumParams = 9 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy() &&
            FTy.getParamType(6)->isIntegerTy() &&
            FTy.getParamType(7)->isPointerTy() &&
            FTy.getParamType(8)->isIntegerTy());
#ifndef _WIN32
  case LibFunc_read:
    return (NumParams == 3 && FTy.getParamType(1)->isPointerTy());
#else
  // NOTE: The libfunc read is an alias to _read in Windows (LibFunc_under_read)
  case LibFunc_read:
#endif // _WIN32
  case LibFunc_under_read:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());
#endif // INTEL_CUSTOMIZATION
  case LibFunc_rewind:
  case LibFunc_rmdir:
  case LibFunc_remove:
  case LibFunc_realpath:
    return (NumParams >= 1 && FTy.getParamType(0)->isPointerTy());
  case LibFunc_rename:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_readlink:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_write:
    return (NumParams == 3 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_aligned_alloc:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy());
  case LibFunc_bcopy:
  case LibFunc_bcmp:
    return (NumParams == 3 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_bzero:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy());
  case LibFunc_calloc:
  case LibFunc_vec_calloc:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0) == FTy.getParamType(1));

#if INTEL_CUSTOMIZATION
  case LibFunc_atexit:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());
#endif // INTEL_CUSTOMIZATION

  case LibFunc_atof:
  case LibFunc_atoi:
  case LibFunc_atol:
  case LibFunc_atoll:
  case LibFunc_ferror:
  case LibFunc_getenv:
  case LibFunc_getpwnam:
  case LibFunc_iprintf:
  case LibFunc_small_printf:
  case LibFunc_pclose:
  case LibFunc_perror:
  case LibFunc_printf:
  case LibFunc_puts:
  case LibFunc_uname:
  case LibFunc_under_IO_getc:
  case LibFunc_unlink:
  case LibFunc_unsetenv:
    return (NumParams == 1 && FTy.getParamType(0)->isPointerTy());

  case LibFunc_access:
  case LibFunc_chmod:
  case LibFunc_chown:
  case LibFunc_clearerr:
  case LibFunc_closedir:
  case LibFunc_ctermid:
  case LibFunc_fclose:
  case LibFunc_feof:
  case LibFunc_fflush:
  case LibFunc_fgetc:
  case LibFunc_fgetc_unlocked:
  case LibFunc_fileno:
  case LibFunc_flockfile:
  case LibFunc_free:
  case LibFunc_fseek:
  case LibFunc_fseeko64:
  case LibFunc_fseeko:
  case LibFunc_fsetpos:
  case LibFunc_ftell:
  case LibFunc_ftello64:
  case LibFunc_ftello:
  case LibFunc_ftrylockfile:
  case LibFunc_funlockfile:
  case LibFunc_getc:
  case LibFunc_getc_unlocked:
  case LibFunc_getlogin_r:
  case LibFunc_mkdir:
  case LibFunc_mktime:
  case LibFunc_times:
  case LibFunc_vec_free:
    return (NumParams != 0 && FTy.getParamType(0)->isPointerTy());
  case LibFunc___kmpc_free_shared:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy(SizeTBits));

#if INTEL_CUSTOMIZATION
  case LibFunc_cpow:
    return (NumParams == 4 && FTy.getParamType(0)->isDoubleTy() &&
            FTy.getParamType(1)->isDoubleTy() &&
            FTy.getParamType(2)->isDoubleTy() &&
            FTy.getParamType(3)->isDoubleTy() &&
            FTy.getReturnType()->isStructTy() &&
            FTy.getReturnType()->getStructNumElements() == 2 &&
            FTy.getReturnType()->getStructElementType(0)->isDoubleTy() &&
            FTy.getReturnType()->getStructElementType(1)->isDoubleTy());

  case LibFunc_cpowf:
    return (NumParams == 2 && FTy.getParamType(0)->isVectorTy() &&
            FTy.getParamType(0)->getScalarType()->isFloatTy() &&
            FTy.getParamType(1)->isVectorTy() &&
            FTy.getParamType(1)->getScalarType()->isFloatTy() &&
            FTy.getReturnType()->isVectorTy() &&
            FTy.getReturnType()->getScalarType()->isFloatTy());

  case LibFunc_csqrt:
    return (NumParams == 2 && FTy.getParamType(0)->isDoubleTy() &&
            FTy.getParamType(1)->isDoubleTy() &&
            FTy.getReturnType()->isStructTy() &&
            FTy.getReturnType()->getStructNumElements() == 2 &&
            FTy.getReturnType()->getStructElementType(0)->isDoubleTy() &&
            FTy.getReturnType()->getStructElementType(1)->isDoubleTy());

  case LibFunc_csqrtf:
    return (NumParams == 1 && FTy.getParamType(0)->isVectorTy() &&
            FTy.getParamType(0)->getScalarType()->isFloatTy() &&
            FTy.getReturnType()->isVectorTy() &&
            FTy.getReturnType()->getScalarType()->isFloatTy());
#endif // INTEL_CUSTOMIZATION

  case LibFunc_fopen:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_fork:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy(32));
  case LibFunc_fdopen:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_fputc:
  case LibFunc_fputc_unlocked:
  case LibFunc_fstat:
  case LibFunc_frexp:
  case LibFunc_frexpf:
  case LibFunc_frexpl:
  case LibFunc_fstatvfs:
    return (NumParams == 2 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_fgets:
  case LibFunc_fgets_unlocked:
    return (NumParams == 3 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());
  case LibFunc_fread:
  case LibFunc_fread_unlocked:
    return (NumParams == 4 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());
  case LibFunc_fwrite:
  case LibFunc_fwrite_unlocked:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy());
  case LibFunc_fputs:
  case LibFunc_fputs_unlocked:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_fscanf:
  case LibFunc_fiprintf:
  case LibFunc_small_fprintf:
  case LibFunc_fprintf:
    return (NumParams >= 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_fgetpos:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_getchar:
  case LibFunc_getchar_unlocked:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());
  case LibFunc_gets:
    return (NumParams == 1 && FTy.getParamType(0)->isPointerTy());
  case LibFunc_getitimer:
    return (NumParams == 2 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_ungetc:
    return (NumParams == 2 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_utime:
  case LibFunc_utimes:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_putc:
  case LibFunc_putc_unlocked:
    return (NumParams == 2 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_pread:
  case LibFunc_pwrite:
    return (NumParams == 4 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_popen:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_vscanf:
    return (NumParams == 2 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_vsscanf:
    return (NumParams == 3 && FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());
  case LibFunc_vfscanf:
    return (NumParams == 3 && FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());
  case LibFunc_valloc:
    return (FTy.getReturnType()->isPointerTy());
  case LibFunc_vprintf:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy());
  case LibFunc_vfprintf:
  case LibFunc_vsprintf:
    return (NumParams == 3 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_vsprintf_chk:
    return NumParams == 5 && FTy.getParamType(0)->isPointerTy() &&
           FTy.getParamType(1)->isIntegerTy(32) &&
           FTy.getParamType(2)->isIntegerTy(SizeTBits) && FTy.getParamType(3)->isPointerTy();
  case LibFunc_vsnprintf:
    return (NumParams == 4 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());
  case LibFunc_vsnprintf_chk:
    return NumParams == 6 && FTy.getParamType(0)->isPointerTy() &&
           FTy.getParamType(2)->isIntegerTy(32) &&
           FTy.getParamType(3)->isIntegerTy(SizeTBits) && FTy.getParamType(4)->isPointerTy();
  case LibFunc_open:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy());
  case LibFunc_opendir:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());
  case LibFunc_tmpfile:
    return (FTy.getReturnType()->isPointerTy());
  case LibFunc_htonl:
  case LibFunc_ntohl:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy(32) &&
            FTy.getReturnType() == FTy.getParamType(0));
  case LibFunc_htons:
  case LibFunc_ntohs:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy(16) &&
            FTy.getReturnType() == FTy.getParamType(0));
  case LibFunc_lstat:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_lchown:
    return (NumParams == 3 && FTy.getParamType(0)->isPointerTy());
  case LibFunc_qsort:
    return (NumParams == 4 && FTy.getParamType(3)->isPointerTy());
#if INTEL_CUSTOMIZATION
  case LibFunc_QueryPerformanceCounter:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());
#endif // INTEL_CUSTOMIZATION
  case LibFunc_dunder_strdup:
  case LibFunc_dunder_strndup:
    return (NumParams >= 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());
  case LibFunc_dunder_strtok_r:
    return (NumParams == 3 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_under_IO_putc:
    return (NumParams == 2 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_dunder_isoc99_scanf:
    return (NumParams >= 1 && FTy.getParamType(0)->isPointerTy());
  case LibFunc_stat64:
  case LibFunc_lstat64:
  case LibFunc_statvfs64:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_dunder_isoc99_sscanf:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_fopen64:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_tmpfile64:
    return (FTy.getReturnType()->isPointerTy());
  case LibFunc_fstat64:
  case LibFunc_fstatvfs64:
    return (NumParams == 2 && FTy.getParamType(1)->isPointerTy());
  case LibFunc_open64:
    return (NumParams >= 2 && FTy.getParamType(0)->isPointerTy());
  case LibFunc_gettimeofday:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  // new(unsigned int);
  case LibFunc_Znwj:
  // new(unsigned long);
  case LibFunc_Znwm:
  // new[](unsigned int);
  case LibFunc_Znaj:
  // new[](unsigned long);
  case LibFunc_Znam:
  // new(unsigned int);
  case LibFunc_msvc_new_int:
  // new(unsigned long long);
  case LibFunc_msvc_new_longlong:
  // new[](unsigned int);
  case LibFunc_msvc_new_array_int:
  // new[](unsigned long long);
  case LibFunc_msvc_new_array_longlong:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy());

  // new(unsigned int, nothrow);
  case LibFunc_ZnwjRKSt9nothrow_t:
  // new(unsigned long, nothrow);
  case LibFunc_ZnwmRKSt9nothrow_t:
  // new[](unsigned int, nothrow);
  case LibFunc_ZnajRKSt9nothrow_t:
  // new[](unsigned long, nothrow);
  case LibFunc_ZnamRKSt9nothrow_t:
  // new(unsigned int, nothrow);
  case LibFunc_msvc_new_int_nothrow:
  // new(unsigned long long, nothrow);
  case LibFunc_msvc_new_longlong_nothrow:
  // new[](unsigned int, nothrow);
  case LibFunc_msvc_new_array_int_nothrow:
  // new[](unsigned long long, nothrow);
  case LibFunc_msvc_new_array_longlong_nothrow:
  // new(unsigned int, align_val_t)
  case LibFunc_ZnwjSt11align_val_t:
  // new(unsigned long, align_val_t)
  case LibFunc_ZnwmSt11align_val_t:
  // new[](unsigned int, align_val_t)
  case LibFunc_ZnajSt11align_val_t:
  // new[](unsigned long, align_val_t)
  case LibFunc_ZnamSt11align_val_t:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy());

  // new(unsigned int, align_val_t, nothrow)
  case LibFunc_ZnwjSt11align_val_tRKSt9nothrow_t:
  // new(unsigned long, align_val_t, nothrow)
  case LibFunc_ZnwmSt11align_val_tRKSt9nothrow_t:
  // new[](unsigned int, align_val_t, nothrow)
  case LibFunc_ZnajSt11align_val_tRKSt9nothrow_t:
  // new[](unsigned long, align_val_t, nothrow)
  case LibFunc_ZnamSt11align_val_tRKSt9nothrow_t:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy());

  // void operator delete[](void*);
  case LibFunc_ZdaPv:
  // void operator delete(void*);
  case LibFunc_ZdlPv:
  // void operator delete[](void*);
  case LibFunc_msvc_delete_array_ptr32:
  // void operator delete[](void*);
  case LibFunc_msvc_delete_array_ptr64:
  // void operator delete(void*);
  case LibFunc_msvc_delete_ptr32:
  // void operator delete(void*);
  case LibFunc_msvc_delete_ptr64:
    return (NumParams == 1 && FTy.getParamType(0)->isPointerTy());

  // void operator delete[](void*, nothrow);
  case LibFunc_ZdaPvRKSt9nothrow_t:
  // void operator delete[](void*, unsigned int);
  case LibFunc_ZdaPvj:
  // void operator delete[](void*, unsigned long);
  case LibFunc_ZdaPvm:
  // void operator delete(void*, nothrow);
  case LibFunc_ZdlPvRKSt9nothrow_t:
  // void operator delete(void*, unsigned int);
  case LibFunc_ZdlPvj:
  // void operator delete(void*, unsigned long);
  case LibFunc_ZdlPvm:
  // void operator delete(void*, align_val_t)
  case LibFunc_ZdlPvSt11align_val_t:
  // void operator delete[](void*, align_val_t)
  case LibFunc_ZdaPvSt11align_val_t:
  // void operator delete[](void*, unsigned int);
  case LibFunc_msvc_delete_array_ptr32_int:
  // void operator delete[](void*, nothrow);
  case LibFunc_msvc_delete_array_ptr32_nothrow:
  // void operator delete[](void*, unsigned long long);
  case LibFunc_msvc_delete_array_ptr64_longlong:
  // void operator delete[](void*, nothrow);
  case LibFunc_msvc_delete_array_ptr64_nothrow:
  // void operator delete(void*, unsigned int);
  case LibFunc_msvc_delete_ptr32_int:
  // void operator delete(void*, nothrow);
  case LibFunc_msvc_delete_ptr32_nothrow:
  // void operator delete(void*, unsigned long long);
  case LibFunc_msvc_delete_ptr64_longlong:
  // void operator delete(void*, nothrow);
  case LibFunc_msvc_delete_ptr64_nothrow:
    return (NumParams == 2 && FTy.getParamType(0)->isPointerTy());

#if INTEL_CUSTOMIZATION
  case LibFunc_msvc_std_bad_array_new_length_ctor:
      return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
          FTy.getParamType(0)->isPointerTy() &&         // this pointer
          FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_bad_alloc_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_basic_istream_vector_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_ostream_vector_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_bad_alloc_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_filebuf_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_ios_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_filebuf_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());        // this pointer

  case LibFunc_msvc_std_basic_filebuf_imbue:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_basic_filebuf_overflow:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_filebuf_pbackfail:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_filebuf_setbuf:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_filebuf_std_fpos_seekoff:
    return (NumParams == 5 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_msvc_std_basic_filebuf_std_fpos_seekpos:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_msvc_std_basic_filebuf_uflow:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());        // this pointer

  case LibFunc_msvc_std_basic_filebuf_under_Endwrite:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());        // this pointer

  case LibFunc_msvc_std_basic_filebuf_underflow:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());        // this pointer

  case LibFunc_msvc_std_basic_filebuf_under_Lock:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());        // this pointer

  case LibFunc_msvc_std_basic_filebuf_xsgetn:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_filebuf_xsputn:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_streambuf_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());        // this pointer

  case LibFunc_msvc_std_basic_streambuf_imbue:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_basic_streambuf_under_Lock:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());        // this pointer

  case LibFunc_msvc_std_basic_streambuf_overflow:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_streambuf_pbackfail:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_streambuf_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_streambuf_setbuf:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_ios_setstate:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_streambuf_showmanyc:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());       // this pointer

  case LibFunc_msvc_std_basic_filebuf_sync:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());       // this pointer

  case LibFunc_msvc_std_basic_streambuf_sync:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());       // this pointer

  case LibFunc_msvc_std_basic_streambuf_uflow:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());       // this pointer

  case LibFunc_msvc_std_basic_streambuf_underflow:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());       // this pointer

  case LibFunc_msvc_std_basic_filebuf_under_Unlock:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());        // this pointer

  case LibFunc_msvc_std_basic_streambuf_under_Unlock:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());        // this pointer

  case LibFunc_msvc_std_basic_streambuf_xsgetn:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_streambuf_xsputn:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_streambuf_std_fpos_seekoff:
    return (NumParams == 5 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_msvc_std_basic_streambuf_std_fpos_seekpos:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_msvc_std_basic_string_append:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_string_append_size_value:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_string_assign_const_ptr:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_basic_string_assign_const_ptr_size:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_string_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&      // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_basic_string_ptr64_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
          FTy.getParamType(0)->isPointerTy() &&      // this pointer
          FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_basic_string_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());         // this pointer

  case LibFunc_msvc_std_basic_string_insert:
    return (NumParams == 4 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_msvc_std_basic_string_operator_equal_const_ptr:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_basic_string_operator_equal_ptr64:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_basic_string_operator_plus_equal_char:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_string_push_back:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_basic_string_resize:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_basic_string_under_xlen:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_msvc_std_codecvt_do_always_noconv:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_codecvt_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_codecvt_use_facet:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_codecvt_do_encoding:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_codecvt_do_in:
    return (NumParams == 8 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy() &&
            FTy.getParamType(7)->isPointerTy());

  case LibFunc_msvc_std_codecvt_do_length:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_msvc_std_codecvt_do_max_length:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_ctype_do_narrow_char_char:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_ctype_do_narrow_ptr_ptr_char_ptr:
    return (NumParams == 5 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_msvc_std_codecvt_do_out:
    return (NumParams == 8 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy() &&
            FTy.getParamType(7)->isPointerTy());

  case LibFunc_msvc_std_ctype_do_tolower_char:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_ctype_do_tolower_ptr_ptr:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_msvc_std_ctype_do_toupper_char:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_ctype_do_toupper_ptr_ptr:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_msvc_std_ctype_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_codecvt_do_unshift:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_msvc_std_ctype_do_widen_char:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_ctype_do_widen_ptr_ptr_ptr:
    return (NumParams == 4 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&       // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_msvc_std_ctype_use_facet:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_error_category_default_error:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_error_category_equivalent_error_code:
     return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());
  case LibFunc_msvc_std_error_category_equivalent_error_condition:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_msvc_std_error_code_make_error_code:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_Execute_once:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_msvc_std_exception_const_ptr_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_exception_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_exception_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_exception_what:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_facet_register:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_under_Fiopen:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_ios_base_under_Ios_base_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_locale_under_Init:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_msvc_std_ios_base_failure:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_msvc_std_ios_base_failure_const_ptr_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_ios_base_failure_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_ios_base_scalar_deleting_dtor:
     return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_istreambuf_iterator_operator_plus_plus:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_locale_facet_decref:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_locale_facet_incref:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_locimp_Getgloballocale:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_msvc_std_locinfo_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_locinfo_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_lockit:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_lockit_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_bool_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_double_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_float_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_long_long_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_long_double_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_long_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_unsigned_int_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_unsigned_long_long_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_unsigned_long_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_unsigned_short_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_do_get_void_ptr:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_msvc_std_num_get_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_num_get_under_Getffld:
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_msvc_std_num_get_under_Getifld:
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_msvc_std_num_get_use_facet:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_num_put_do_put_bool:
    return (NumParams == 6 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy());

  case LibFunc_msvc_std_num_put_do_put_double:
    return (NumParams == 6 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isDoubleTy());

  case LibFunc_msvc_std_num_put_do_put_long:
    return (NumParams == 6 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy());


  case LibFunc_msvc_std_num_put_do_put_long_double:
    return (NumParams == 6 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isDoubleTy());

  case LibFunc_msvc_std_num_put_do_put_long_long:
    return (NumParams == 6 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy());

case LibFunc_msvc_std_num_put_do_put_ulong:
    return (NumParams == 6 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy());

  case LibFunc_msvc_std_num_put_do_put_ulong_long:
    return (NumParams == 6 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy());

  case LibFunc_msvc_std_num_put_do_put_void_ptr:
    return (NumParams == 6 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_msvc_std_num_put_ostreambuf_iterator_Fput:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isIntegerTy());

  case LibFunc_msvc_std_num_put_ostreambuf_iterator_iput:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&    // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isIntegerTy());

  case LibFunc_msvc_std_num_put_ostreambuf_iterator_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_numpunct_do_decimal_point:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_numpunct_do_falsename:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_numpunct_do_truename:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_numpunct_do_grouping:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_numpunct_do_thousands_sep:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_numpunct_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_numpunct_use_facet:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_num_put_use_facet:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_numpunct_Tidy:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_runtime_error_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_runtime_error_ptr64_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
        FTy.getParamType(0)->isPointerTy() &&
        FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_runtime_error_char_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_runtime_error_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_Syserror_map:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_msvc_std_system_error_const_ptr_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_system_error_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_bad_array_new_length_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_under_iostreamer_error_category2_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_under_Throw_bad_array_new_length:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_msvc_std_Xbad_alloc:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_msvc_std_under_Xlen_string:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_msvc_std_Xlength_error:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_under_generic_error_category_message:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_under_iostreamer_error_category_message:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_under_iostreamer_error_category2_message:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_msvc_std_under_iostreamer_error_category_name:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_under_iostreamer_error_category2_name:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_under_iostreamer_error_category_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_under_system_error_scalar_deleting_dtor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_msvc_std_uncaught_exception:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_msvc_std_under_locinfo_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_under_locinfo_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_under_immortalize_impl:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_msvc_std_under_system_error_const_ptr_ctor:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_msvc_std_Xout_of_range:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_Xran:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_msvc_std_Xruntime_error:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_yarn_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_yarn_wchar_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_msvc_std_CxxThrowException:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
#endif // INTEL_CUSTOMIZATION
  // void operator delete(void*, align_val_t, nothrow)
  case LibFunc_ZdlPvSt11align_val_tRKSt9nothrow_t:
  // void operator delete[](void*, align_val_t, nothrow)
  case LibFunc_ZdaPvSt11align_val_tRKSt9nothrow_t:
  // void operator delete(void*, unsigned int, align_val_t)
  case LibFunc_ZdlPvjSt11align_val_t:
  // void operator delete(void*, unsigned long, align_val_t)
  case LibFunc_ZdlPvmSt11align_val_t:
  // void operator delete[](void*, unsigned int, align_val_t);
  case LibFunc_ZdaPvjSt11align_val_t:
  // void operator delete[](void*, unsigned long, align_val_t);
  case LibFunc_ZdaPvmSt11align_val_t:
    return (NumParams == 3 && FTy.getParamType(0)->isPointerTy());

  // void __atomic_load(size_t, void *, void *, int)
  case LibFunc_atomic_load:
  // void __atomic_store(size_t, void *, void *, int)
  case LibFunc_atomic_store:
    return (NumParams == 4 && FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_memset_pattern4:
  case LibFunc_memset_pattern8:
  case LibFunc_memset_pattern16:
    return (!FTy.isVarArg() && NumParams == 3 &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_cxa_guard_abort:
  case LibFunc_cxa_guard_acquire:
  case LibFunc_cxa_guard_release:
  case LibFunc_nvvm_reflect:
    return (NumParams == 1 && FTy.getParamType(0)->isPointerTy());

  case LibFunc_sincospi_stret:
  case LibFunc_sincospif_stret:
    return (NumParams == 1 && FTy.getParamType(0)->isFloatingPointTy());

  case LibFunc_acos:
  case LibFunc_acos_finite:
  case LibFunc_acosf:
  case LibFunc_acosf_finite:
  case LibFunc_acosh:
  case LibFunc_acosh_finite:
  case LibFunc_acoshf:
  case LibFunc_acoshf_finite:
  case LibFunc_acoshl:
  case LibFunc_acoshl_finite:
  case LibFunc_acosl:
  case LibFunc_acosl_finite:
  case LibFunc_asin:
  case LibFunc_asin_finite:
  case LibFunc_asinf:
  case LibFunc_asinf_finite:
  case LibFunc_asinh:
  case LibFunc_asinhf:
  case LibFunc_asinhl:
  case LibFunc_asinl:
  case LibFunc_asinl_finite:
  case LibFunc_atan:
  case LibFunc_atanf:
  case LibFunc_atanh:
  case LibFunc_atanh_finite:
  case LibFunc_atanhf:
  case LibFunc_atanhf_finite:
  case LibFunc_atanhl:
  case LibFunc_atanhl_finite:
  case LibFunc_atanl:
  case LibFunc_cbrt:
  case LibFunc_cbrtf:
  case LibFunc_cbrtl:
  case LibFunc_ceil:
  case LibFunc_ceilf:
  case LibFunc_ceill:
  case LibFunc_cos:
  case LibFunc_cosd:  // INTEL
  case LibFunc_cosdf: // INTEL
  case LibFunc_cosf:
  case LibFunc_cosh:
  case LibFunc_cosh_finite:
  case LibFunc_coshf:
  case LibFunc_coshf_finite:
  case LibFunc_coshl:
  case LibFunc_coshl_finite:
  case LibFunc_cosl:
  case LibFunc_exp10:
  case LibFunc_exp10_finite:
  case LibFunc_exp10f:
  case LibFunc_exp10f_finite:
  case LibFunc_exp10l:
  case LibFunc_exp10l_finite:
  case LibFunc_exp2:
  case LibFunc_exp2_finite:
  case LibFunc_exp2f:
  case LibFunc_exp2f_finite:
  case LibFunc_exp2l:
  case LibFunc_exp2l_finite:
  case LibFunc_exp:
  case LibFunc_exp_finite:
  case LibFunc_expf:
  case LibFunc_expf_finite:
  case LibFunc_expl:
  case LibFunc_expl_finite:
  case LibFunc_expm1:
  case LibFunc_expm1f:
  case LibFunc_expm1l:
  case LibFunc_fabs:
  case LibFunc_fabsf:
  case LibFunc_fabsl:
  case LibFunc_floor:
  case LibFunc_floorf:
  case LibFunc_floorl:
  case LibFunc_log10:
  case LibFunc_log10_finite:
  case LibFunc_log10f:
  case LibFunc_log10f_finite:
  case LibFunc_log10l:
  case LibFunc_log10l_finite:
  case LibFunc_log1p:
  case LibFunc_log1pf:
  case LibFunc_log1pl:
  case LibFunc_log2:
  case LibFunc_log2_finite:
  case LibFunc_log2f:
  case LibFunc_log2f_finite:
  case LibFunc_log2l:
  case LibFunc_log2l_finite:
  case LibFunc_log:
  case LibFunc_log_finite:
  case LibFunc_logb:
  case LibFunc_logbf:
  case LibFunc_logbl:
  case LibFunc_logf:
  case LibFunc_logf_finite:
  case LibFunc_logl:
  case LibFunc_logl_finite:
  case LibFunc_nearbyint:
  case LibFunc_nearbyintf:
  case LibFunc_nearbyintl:
  case LibFunc_rint:
  case LibFunc_rintf:
  case LibFunc_rintl:
  case LibFunc_round:
  case LibFunc_roundf:
  case LibFunc_roundl:
  case LibFunc_roundeven:
  case LibFunc_roundevenf:
  case LibFunc_roundevenl:
  case LibFunc_sin:
  case LibFunc_sind:  // INTEL
  case LibFunc_sindf: // INTEL
  case LibFunc_sinf:
  case LibFunc_sinh:
  case LibFunc_sinh_finite:
  case LibFunc_sinhf:
  case LibFunc_sinhf_finite:
  case LibFunc_sinhl:
  case LibFunc_sinhl_finite:
  case LibFunc_sinl:
  case LibFunc_sqrt:
  case LibFunc_sqrt_finite:
  case LibFunc_sqrtf:
  case LibFunc_sqrtf_finite:
  case LibFunc_sqrtl:
  case LibFunc_sqrtl_finite:
  case LibFunc_tan:
  case LibFunc_tand:  // INTEL
  case LibFunc_tandf: // INTEL
  case LibFunc_tanf:
  case LibFunc_tanh:
  case LibFunc_tanhf:
  case LibFunc_tanhl:
  case LibFunc_tanl:
  case LibFunc_trunc:
  case LibFunc_truncf:
  case LibFunc_truncl:
    return (NumParams == 1 && FTy.getReturnType()->isFloatingPointTy() &&
            FTy.getReturnType() == FTy.getParamType(0));

  case LibFunc_atan2:
  case LibFunc_atan2_finite:
  case LibFunc_atan2f:
  case LibFunc_atan2f_finite:
  case LibFunc_atan2l:
  case LibFunc_atan2l_finite:
  case LibFunc_fmin:
  case LibFunc_fminf:
  case LibFunc_fminl:
  case LibFunc_fmax:
  case LibFunc_fmaxf:
  case LibFunc_fmaxl:
  case LibFunc_fmod:
  case LibFunc_fmodf:
  case LibFunc_fmodl:
  case LibFunc_remainder:
  case LibFunc_remainderf:
  case LibFunc_remainderl:
  case LibFunc_copysign:
  case LibFunc_copysignf:
  case LibFunc_copysignl:
  case LibFunc_pow:
  case LibFunc_pow_finite:
  case LibFunc_powf:
  case LibFunc_powf_finite:
  case LibFunc_powl:
  case LibFunc_powl_finite:
    return (NumParams == 2 && FTy.getReturnType()->isFloatingPointTy() &&
            FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getReturnType() == FTy.getParamType(1));

  case LibFunc_ldexp:
  case LibFunc_ldexpf:
  case LibFunc_ldexpl:
    return (NumParams == 2 && FTy.getReturnType()->isFloatingPointTy() &&
            FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getParamType(1)->isIntegerTy(getIntSize()));

  case LibFunc_ffs:
  case LibFunc_ffsl:
  case LibFunc_ffsll:
  case LibFunc_fls:
  case LibFunc_flsl:
  case LibFunc_flsll:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy(32) &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_isdigit:
  case LibFunc_isascii:
  case LibFunc_toascii:
  case LibFunc_putchar:
  case LibFunc_putchar_unlocked:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy(32) &&
            FTy.getReturnType() == FTy.getParamType(0));

  case LibFunc_abs:
  case LibFunc_labs:
  case LibFunc_llabs:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getReturnType() == FTy.getParamType(0));

  case LibFunc_cxa_atexit:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

#if INTEL_CUSTOMIZATION
  case LibFunc_intel_sinpi:
  case LibFunc_intel_cospi:
  case LibFunc_asinpi:
  case LibFunc_acospi:
  case LibFunc_atanpi:
  case LibFunc_pow2o3:
  case LibFunc_pow3o2:
  case LibFunc_tanpi:
#endif // INTEL_CUSTOMIZATION
  case LibFunc_sinpi:
  case LibFunc_cospi:
    return (NumParams == 1 && FTy.getReturnType()->isDoubleTy() &&
            FTy.getReturnType() == FTy.getParamType(0));

#if INTEL_CUSTOMIZATION
  case LibFunc_intel_sinpif:
  case LibFunc_intel_cospif:
  case LibFunc_asinpif:
  case LibFunc_acospif:
  case LibFunc_atanpif:
  case LibFunc_pow2o3f:
  case LibFunc_pow3o2f:
  case LibFunc_tanpif:
#endif // INTEL_CUSTOMIZATION
  case LibFunc_sinpif:
  case LibFunc_cospif:
    return (NumParams == 1 && FTy.getReturnType()->isFloatTy() &&
            FTy.getReturnType() == FTy.getParamType(0));

#if INTEL_CUSTOMIZATION
  case LibFunc_atan2pi:
  case LibFunc_fdim:
  case LibFunc_maxmag:
  case LibFunc_minmag:
  case LibFunc_powr:
  case LibFunc_nextafter:
    return (NumParams == 2 && FTy.getReturnType()->isDoubleTy() &&
            FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getReturnType() == FTy.getParamType(1));

  case LibFunc_fdimf:
  case LibFunc_atan2pif:
  case LibFunc_maxmagf:
  case LibFunc_minmagf:
  case LibFunc_powrf:
  case LibFunc_nextafterf:
    return (NumParams == 2 && FTy.getReturnType()->isFloatTy() &&
            FTy.getReturnType() == FTy.getParamType(0) &&
            FTy.getReturnType() == FTy.getParamType(1));
#endif // INTEL_CUSTOMIZATION

  case LibFunc_strnlen:
    return (NumParams == 2 && FTy.getReturnType() == FTy.getParamType(1) &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy(SizeTBits));

  case LibFunc_posix_memalign:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy(32) &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy(SizeTBits) &&
            FTy.getParamType(2)->isIntegerTy(SizeTBits));

  case LibFunc_wcslen:
    return (NumParams == 1 && FTy.getParamType(0)->isPointerTy() &&
            FTy.getReturnType()->isIntegerTy());

  case LibFunc_cabs:
  case LibFunc_cabsf:
  case LibFunc_cabsl: {
    Type* RetTy = FTy.getReturnType();
    if (!RetTy->isFloatingPointTy())
      return false;

    // NOTE: These prototypes are target specific and currently support
    // "complex" passed as an array or discrete real & imaginary parameters.
    // Add other calling conventions to enable libcall optimizations.
    if (NumParams == 1)
      return (FTy.getParamType(0)->isArrayTy() &&
              FTy.getParamType(0)->getArrayNumElements() == 2 &&
              FTy.getParamType(0)->getArrayElementType() == RetTy);
    else if (NumParams == 2)
      return (FTy.getParamType(0) == RetTy && FTy.getParamType(1) == RetTy);
    else
      return false;
  }

#if INTEL_CUSTOMIZATION
  case LibFunc_cexp:
    return (NumParams == 2 && FTy.getParamType(0)->isDoubleTy() &&
            FTy.getParamType(1)->isDoubleTy() &&
            FTy.getReturnType()->isArrayTy() &&
            FTy.getReturnType()->getArrayNumElements() == 2 &&
            FTy.getReturnType()->getArrayElementType()->isDoubleTy());
#endif // INTEL_CUSTOMIZATION

  case LibFunc::NumLibFuncs:
  case LibFunc::NotLibFunc:
    break;

#if INTEL_CUSTOMIZATION
  // Note: These are being placed after the case for LibFunc::NumLibFuncs to
  // avoid conflicts during community pulldowns, which otherwise occur
  // every time a new entry is added to the enumeration.

  case LibFunc_dunder_isoc99_fscanf:
    // int __isoc99_fscanf(FILE *stream, const char *format, ... );
    return (NumParams >= 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());
  case LibFunc_dunder_xstat64:
  case LibFunc_dunder_xstat:
    // int __xstat64 (int vers, const char *file, struct stat64 *buf);
    // int __xstat(int vers, const char *file, struct stat *buf);
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());
  case LibFunc_exit:
    // void exit(int status);
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy());
  case LibFunc_sincos:
    // void sincos(double x, double *y, double *z);
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isDoubleTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());
  case LibFunc_sincosf:
    // void sincosf(float x, float *y, float *z);
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isFloatTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_assert_fail:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_clang_call_terminate:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_ctype_b_loc:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_ctype_get_mb_cur_max:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_ctype_tolower_loc:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_ctype_toupper_loc:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_cxa_allocate_exception:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_cxa_bad_cast:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_cxa_bad_typeid:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_cxa_begin_catch:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_cxa_call_unexpected:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_cxa_end_catch:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_cxa_free_exception:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_cxa_get_exception_ptr:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_cxa_pure_virtual:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_cxa_rethrow:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_cxa_throw:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_CloseHandle:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_ConvertThreadToFiber:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_CreateFiber:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_CreateFileA:
    return (NumParams == 7 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_CreateFileW:
    return (NumParams == 7 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_DeleteCriticalSection:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_DeleteFiber:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_dynamic_cast:
    return (NumParams == 4 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_EnterCriticalSection:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_errno_location:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_fxstat:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_fxstat64:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_FindClose:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_FindFirstFileA:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_FindFirstFileW:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_FindNextFileA:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_FindNextFileW:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_FindResourceA:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_FormatMessageA:
    return (NumParams == 7 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isIntegerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_FreeResource:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_GetCurrentDirectoryA:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_GetCurrentDirectoryW:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_GetCurrentProcess:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_GetCurrentThreadId:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_GetFullPathNameA:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_GetFullPathNameW:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_GetLastError:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_GetModuleFileNameA:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_GetModuleHandleA:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_GetProcAddress:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_GetProcessTimes:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_GetShortPathNameW:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_GetSystemTime:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_GetVersionExA:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_GlobalMemoryStatus:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_gxx_personality_v0:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_isinf:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isFloatingPointTy());

  case LibFunc_isnan:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isFloatingPointTy());

  case LibFunc_isnanf:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isFloatingPointTy());
#endif  // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  case LibFunc_kmpc_atomic_compare_exchange:
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy());

  case LibFunc_kmpc_atomic_fixed4_add:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_kmpc_atomic_float8_add:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isDoubleTy());

  case LibFunc_kmpc_atomic_load:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_kmpc_atomic_store:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_kmpc_barrier:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_critical:
    if (NumParams == 1)
      return (FTy.getReturnType()->isVoidTy() &&
              FTy.getParamType(0)->isPointerTy());
    else if (NumParams == 3)
      return (FTy.getReturnType()->isVoidTy() &&
              FTy.getParamType(0)->isPointerTy() &&
              FTy.getParamType(1)->isIntegerTy() &&
              FTy.getParamType(2)->isPointerTy());
    else
      return false;

  case LibFunc_kmpc_critical_with_hint:
    if (NumParams == 2)
      return (FTy.getReturnType()->isVoidTy() &&
              FTy.getParamType(0)->isPointerTy() &&
              FTy.getParamType(1)->isIntegerTy());
    else if (NumParams == 4)
      return (FTy.getReturnType()->isVoidTy() &&
              FTy.getParamType(0)->isPointerTy() &&
              FTy.getParamType(1)->isIntegerTy() &&
              FTy.getParamType(2)->isPointerTy() &&
              FTy.getParamType(3)->isIntegerTy());
    else
      return false;

  case LibFunc_kmpc_dispatch_init_4:
  case LibFunc_kmpc_dispatch_init_4u:
  case LibFunc_kmpc_dispatch_init_8:
  case LibFunc_kmpc_dispatch_init_8u:
    return (NumParams == 7 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy() &&
            FTy.getParamType(6)->isIntegerTy());

  case LibFunc_kmpc_dispatch_next_4:
  case LibFunc_kmpc_dispatch_next_4u:
  case LibFunc_kmpc_dispatch_next_8:
  case LibFunc_kmpc_dispatch_next_8u:
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_kmpc_end_critical:
    if (NumParams == 1)
      return (NumParams == FTy.getReturnType()->isVoidTy() &&
              FTy.getParamType(0)->isPointerTy());
    else if (NumParams == 3)
      return (FTy.getReturnType()->isVoidTy() &&
              FTy.getParamType(0)->isPointerTy() &&
              FTy.getParamType(1)->isIntegerTy() &&
              FTy.getParamType(2)->isPointerTy());
    else
      return false;

  case LibFunc_kmpc_end_reduce:
  case LibFunc_kmpc_end_reduce_nowait:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_kmpc_end_serialized_parallel:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_flush:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_kmpc_for_static_fini:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_for_static_init_4:
  case LibFunc_kmpc_for_static_init_4u:
    return (NumParams == 9 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy() &&
            FTy.getParamType(7)->isIntegerTy() &&
            FTy.getParamType(8)->isIntegerTy());

  case LibFunc_kmpc_for_static_init_8:
  case LibFunc_kmpc_for_static_init_8u:
    return (NumParams == 9 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy() &&
            FTy.getParamType(7)->isIntegerTy() &&
            FTy.getParamType(8)->isIntegerTy());

  case LibFunc_kmpc_fork_call:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_kmpc_global_thread_num:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_kmpc_ok_to_fork:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_kmpc_omp_task:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_kmpc_omp_task_alloc:
    return (NumParams == 6 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_kmpc_omp_task_begin_if0:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_kmpc_omp_task_complete_if0:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_kmpc_omp_taskwait:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_push_num_threads:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_kmpc_reduce:
  case LibFunc_kmpc_reduce_nowait:
    return (NumParams == 7 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy() &&
            FTy.getParamType(6)->isPointerTy());

  case LibFunc_kmpc_serialized_parallel:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_single:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_end_single:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_masked:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_kmpc_end_masked:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_master:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_end_master:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmpc_threadprivate_cached:
    return (NumParams == 5 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isPointerTy());
#endif  // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
  case LibFunc_gnu_std_basic_filebuf_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_gnu_std_cxx11_basic_stringstream_ctor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_ZNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcESaIcEED1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());  // this pointer

  case LibFunc_LeaveCriticalSection:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_LoadLibraryA:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_LoadResource:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_LocalFree:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_LockResource:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_lxstat:
  case LibFunc_lxstat64:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_regcomp:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_regerror:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_regexec:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_regfree:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_ReadFile:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_SetFilePointer:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_sigsetjmp:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_SizeofResource:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_std_exception_copy:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_std_exception_destroy:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_std_reverse_trivially_swappable_8:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_strncpy_s:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_SystemTimeToFileTime:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_tunder_mb_cur_max_func:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_dunder_CxxFrameHandler3:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_dunder_RTDynamicCast:
    return (NumParams == 5 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_dunder_RTtypeid:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_dunder_powi4i4:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_dunder_powr8i8:
    return (NumParams == 2 && FTy.getReturnType()->isDoubleTy() &&
            FTy.getParamType(0)->isDoubleTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_dunder_std_terminate:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_dunder_std_type_info_compare:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_dunder_std_type_info_name:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_sysv_signal:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_Getctype:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_Getcvt:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_Init_thread_abort:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_Stollx:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_under_Stolx:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_under_Stoullx:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_under_Stoulx:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_under_Tolower:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_Toupper:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_chdir:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

case LibFunc_under_commit:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_under_close:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_under_errno:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_under_fdopen:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_fstat64:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_fstat64i32:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_fseeki64:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_under_ftelli64:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_ftime64:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_difftime64:
    return (NumParams == 2 && FTy.getReturnType()->isDoubleTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_under_get_stream_buffer_pointers:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_under_getcwd:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_under_getdcwd:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_under_getdrive:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_under_exit:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_under_fileno:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_findclose:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_under_findfirst64i32:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_findnext64i32:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_invalid_parameter_noinfo_noreturn:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_under_lock_file:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_localtime64:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_lseeki64:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_under_Init_thread_footer:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_Init_thread_header:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_mkdir:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_purecall:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_under_set_errno:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_under_setmode:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  // _sleep was deprecated since VS2015, it has been replaced with
  // Sleep (LibFunc_Sleep)
  case LibFunc_Sleep:
  case LibFunc_under_sleep:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_SwitchToFiber:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_stat64:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_stat64i32:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_stricmp:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_under_strnicmp:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_under_strtoi64:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_under_time64:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_unlink:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_unlock_file:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_waccess:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_under_wassert:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_under_wfopen:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  // The third parameter for _wopen is a vararg
  case LibFunc_under_wopen:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_under_wremove:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_write:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

    case LibFunc_under_wstat64:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_obstack_begin:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_obstack_memory_used:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_obstack_newchunk:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_setjmp:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_ZNKSs17find_first_not_ofEPKcmm:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNKSs4findEcm:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNKSs4findEPKcmm:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNKSs5rfindEcm:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNKSs5rfindEPKcmm:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNKSs7compareEPKc:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNKSt13runtime_error4whatEv:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNKSt5ctypeIcE13_M_widen_initEv:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_std_cxx11_basic_string_find_first_not_of:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE4findEPKcmm:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_std_cxx11_basic_string_find_char_unsigned_long:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5rfindEPKcmm:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5rfindEcm:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7compareEPKc:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNKSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE3strEv:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNKSt9bad_alloc4whatEv:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNKSt9basic_iosIcSt11char_traitsIcEE5widenEc:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNKSt9exception4whatEv:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSi10_M_extractIdEERSiRT_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSi10_M_extractIfEERSiRT_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSi10_M_extractIlEERSiRT_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSi10_M_extractImEERSiRT_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSi4readEPci:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
      FTy.getParamType(0)->isPointerTy() && // this pointer
      FTy.getParamType(1)->isPointerTy() &&
      FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSi4readEPcl:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSi5tellgEv:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSi5ungetEv:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSirsERi:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSo3putEc:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSo5flushEv:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSo5writeEPKci:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
      FTy.getParamType(0)->isPointerTy() && // this pointer
      FTy.getParamType(1)->isPointerTy() &&
      FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSo5writeEPKcl:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSo9_M_insertIbEERSoT_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSo9_M_insertIdEERSoT_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isFloatingPointTy());

  case LibFunc_ZNSo9_M_insertIlEERSoT_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSo9_M_insertImEERSoT_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSo9_M_insertIPKvEERSoT_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSolsEi:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSs12_M_leak_hardEv:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSs4_Rep10_M_destroyERKSaIcE:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZNSs4_Rep9_S_createEmmRKSaIcE:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_ZNSs6appendEmc:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSs6appendEPKcm:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSs6appendERKSs:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSs6assignEPKcm:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSs6assignERKSs:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSs6insertEmPKcm:
    return (NumParams == 4 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNSs6resizeEmc:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSs7replaceEmmPKcm:
    return (NumParams == 5 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_ZNSs7reserveEm:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSs9_M_mutateEmmm:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNSsC1EPKcmRKSaIcE:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_ZNSsC1EPKcRKSaIcE:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_ZNSsC1ERKSs:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSsC1ERKSsmm:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNSt12__basic_fileIcED1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt13basic_filebufIcSt11char_traitsIcEE4openEPKcSt13_Ios_Openmode:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt13basic_filebufIcSt11char_traitsIcEE5closeEv:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt13basic_filebufIcSt11char_traitsIcEEC1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt13runtime_errorC1EPKc:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt13runtime_errorC1ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt13runtime_errorC1ERKSs:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt13runtime_errorC1ERKS_:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt13runtime_errorC2EPKc:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt13runtime_errorC2ERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_std_runtime_error_std_runtime_error_const:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt13runtime_errorC2ERKSs:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt13runtime_errorD0Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt13runtime_errorD1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt13runtime_errorD2Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt14basic_ifstreamIcSt11char_traitsIcEEC1EPKcSt13_Ios_Openmode:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_std_basic_ifstream_ctor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_ZNSt14basic_ifstreamIcSt11char_traitsIcEED1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_ZNSt14basic_ifstreamIcSt11char_traitsIcEED2Ev:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());  // virtual table

  case LibFunc_ZNSt14basic_ofstreamIcSt11char_traitsIcEEC1EPKcSt13_Ios_Openmode:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_std_basic_ofstream_dtor:
    return (NumParams = 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE5imbueERKSt6locale:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE5uflowEv:
    return (NumParams = 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsputnEPKcl:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt15basic_streambufIcSt11char_traitsIcEED2Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());  // this pointer

  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE7_M_syncEPcmm:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE7seekoffElSt12_Ios_SeekdirSt13_Ios_Openmode:
    return (NumParams == 4 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE7seekposESt4fposI11__mbstate_tESt13_Ios_Openmode:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE8overflowEi:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE9pbackfailEi:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE9underflowEv:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());  // this pointer

  case LibFunc_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEEC2ERKSsSt13_Ios_Openmode:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt6localeC1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt6localeD1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6resizeEmc:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7reserveEm:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE8_M_eraseEmm:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcm:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_assignERKS4_:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERmm:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_mutateEmmPKcm:
    return (NumParams == 5 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_replaceEmmPKcm:
    return (NumParams == 5 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE14_M_replace_auxEmmmc:
    return (NumParams == 5 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5eraseEmm:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2EPKcRKS3_:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2ERKS4_:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2ERKS4_mm:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED2Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE6setbufEPcl:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7_M_syncEPcmm:
    return (NumParams = 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7seekoffElSt12_Ios_SeekdirSt13_Ios_Openmode:
    return (NumParams == 4 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE7seekposESt4fposI11__mbstate_tESt13_Ios_Openmode:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE8overflowEi:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE9pbackfailEi:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE9showmanycEv:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE9underflowEv:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());  // this pointer

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEEC2ERKNS_12basic_stringIcS2_S3_EESt13_Ios_Openmode:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEED2Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcESaIcEEC1ERKNS_12basic_stringIcS2_S3_EESt13_Ios_Openmode:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcESaIcEEC1ESt13_Ios_Openmode:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1119basic_istringstreamIcSt11char_traitsIcESaIcEEC1ERKNS_12basic_stringIcS2_S3_EESt13_Ios_Openmode:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_ZNSt7__cxx1119basic_ostringstreamIcSt11char_traitsIcESaIcEEC1ESt13_Ios_Openmode:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_std_cxx11_basic_ostringstream_ctor:
  case LibFunc_std_cxx11_basic_ostringstream_dtor:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6appendEPKc:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6appendERKS4:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6assignEPKc:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6insertEmRKS4_:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEOS4_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEPKc:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2EOS4:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt8__detail15_List_node_base11_M_transferEPS0_S1_:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_ZNSt8__detail15_List_node_base7_M_hookEPS0_:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt8__detail15_List_node_base9_M_unhookEv:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt8ios_base4InitC1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt8ios_base4InitD1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt8ios_baseC2Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt8ios_baseD2Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt9bad_allocD0Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt9bad_allocD1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE4initEPSt15basic_streambufIcS1_E:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ZNSt9basic_iosIcSt11char_traitsIcEE5rdbufEPSt15basic_streambufIcS1_E:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_ZNSt9exceptionD0Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt9exceptionD1Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

  case LibFunc_ZNSt9exceptionD2Ev:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy()); // this pointer

      // static call (not a member function)
  case LibFunc_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_i:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
      FTy.getParamType(0)->isPointerTy() &&
      FTy.getParamType(1)->isPointerTy() &&
      FTy.getParamType(2)->isIntegerTy());

  // static call (not a member function)
  case LibFunc_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  // static call (not a member function)
  case LibFunc_ZSt16__throw_bad_castv:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  // static call (not a member function)
  case LibFunc_ZSt17__throw_bad_allocv:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  // static call (not a member function)
  case LibFunc_ZSt18_Rb_tree_decrementPKSt18_Rb_tree_node_base:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZSt18_Rb_tree_decrementPSt18_Rb_tree_node_base:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZSt18_Rb_tree_incrementPSt18_Rb_tree_node_base:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZSt19__throw_logic_errorPKc:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZSt20__throw_length_errorPKc:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZSt20__throw_out_of_rangePKc:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_ZSt24__throw_out_of_range_fmtPKcz:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZSt28_Rb_tree_rebalance_for_erasePSt18_Rb_tree_node_baseRS_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZSt28__throw_bad_array_new_lengthv:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  // static call (not a member function)
  case LibFunc_ZSt29_Rb_tree_insert_and_rebalancebPSt18_Rb_tree_node_baseS0_RS_:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_std_basic_ostream_std_endl:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_std_basic_ostream_std_flush:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_ZSt7getlineIcSt11char_traitsIcESaIcEERSt13basic_istreamIT_T0_ES7_RSbIS4_S5_T1_ES4_:
    return (NumParams == 4 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_std_basic_istream_getline_cxx11_char:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  // static call (not a member function)
  case LibFunc_ZSt9terminatev:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() && // this pointer
            FTy.getParamType(1)->isPointerTy());

  // static call (not a member function)
  case LibFunc_ZStrsIcSt11char_traitsIcEERSt13basic_istreamIT_T0_ES6_RS3_:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_abort:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_acrt_iob_func:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_alphasort:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_asctime:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_asprintf:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_backtrace:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_backtrace_symbols:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_bsearch:
    return (NumParams == 5 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_chdir:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_clock:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_close:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_ctime:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_difftime:
    return (NumParams == 2 && FTy.getReturnType()->isFloatingPointTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_div:
    return (NumParams == 2 && FTy.getReturnType()->isStructTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_dunder_rawmemchr:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_dup:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_dup2:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_erfc:
    return (NumParams == 1 && FTy.getReturnType()->isFloatingPointTy() &&
            FTy.getParamType(0)->isFloatingPointTy());

  case LibFunc_error:
    return (NumParams == 3 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_fcntl:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_fcntl64:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_fnmatch:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_for_adjustl:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_for_alloc_allocatable:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_for_alloc_allocatable_handle:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_for_allocate:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_for_allocate_handle:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_for_array_copy_in:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_for_array_copy_out:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_for_backspace: // Varargs function
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_for_check_mult_overflow64: // Varargs function
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_for_close: // Varargs function
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_for_concat:
    return (NumParams == 4 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_for_contig_array:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_for_cpstr:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_for_cpystr:
    return (NumParams == 5 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_for_date_and_time:
    return (NumParams == 8 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isIntegerTy() &&
            FTy.getParamType(6)->isPointerTy() &&
            FTy.getParamType(7)->isIntegerTy());

  case LibFunc_for_dealloc_allocatable:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_for_dealloc_allocatable_handle:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_for_deallocate:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_for_endfile: // Varargs function
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_for_exponent8_v:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isDoubleTy());

  case LibFunc_for_f90_index:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_for_f90_scan:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_for_f90_verify:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_for_fraction8_v:
    return (NumParams == 1 && FTy.getReturnType()->isDoubleTy() &&
            FTy.getParamType(0)->isDoubleTy());

  case LibFunc_for_getcmd_arg_err:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isIntegerTy());

  case LibFunc_for_iargc:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_for_inquire: //  Varargs function
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_for_len_trim:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_for_open: // Varargs function
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_for_random_number:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_for_random_seed_bit_size:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_for_random_seed_put:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_for_read_int_fmt: // Varargs function
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_for_read_int_lis: // Varargs function
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_for_read_int_lis_xmit:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_for_read_seq_fmt: // Varargs function
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_for_read_seq_lis: // Varargs function
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_for_read_seq_lis_xmit:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_for_read_seq_nml: // Varargs function
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_for_realloc_lhs:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_for_rewind:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_for_scale8_v:
    return (NumParams == 2 && FTy.getReturnType()->isDoubleTy() &&
            FTy.getParamType(0)->isDoubleTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_for_set_reentrancy:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_for_setexp8_v:
    return (NumParams == 2 && FTy.getReturnType()->isDoubleTy() &&
            FTy.getParamType(0)->isDoubleTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_for_stop_core_quiet: // Varargs function
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy());

  case LibFunc_for_system_clock_count:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_for_trim:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy());

  case LibFunc_for_write_int_fmt: // Varargs function
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_for_write_int_lis: // Varargs function
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_for_write_int_lis_xmit:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_for_write_int_fmt_xmit:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_for_write_seq: // Varargs function
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_for_write_seq_fmt: // Varargs function
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_for_write_seq_fmt_xmit:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_for_write_seq_lis: // Varargs function
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_for_write_seq_lis_xmit:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_for_write_seq_xmit:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_freopen:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_freopen64:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy());

  case LibFunc_fsync:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_ftruncate64:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_getcwd:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_getegid:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_geteuid:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_getgid:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_getopt_long:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

    case LibFunc_getopt_long_only:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_under_getpid:
  case LibFunc_getpid:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_getpwuid:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_getrlimit:
  case LibFunc_getrlimit64:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_getrusage:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_getuid:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_glob:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_globfree:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_under_gmtime64:
  case LibFunc_gmtime:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_gmtime_r:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_hypot:
    return (NumParams == 2 && FTy.getReturnType()->isFloatingPointTy() &&
            FTy.getParamType(0)->isFloatingPointTy() &&
            FTy.getParamType(1)->isFloatingPointTy());

  case LibFunc_hypotf:
    return (NumParams == 2 && FTy.getReturnType()->isFloatingPointTy() &&
            FTy.getParamType(0)->isFloatingPointTy() &&
            FTy.getParamType(1)->isFloatingPointTy());

  case LibFunc_InitializeCriticalSection:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_InitializeCriticalSectionAndSpinCount:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_ioctl:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_isalnum:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_isalpha:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_isatty:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_iscntrl:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_islower:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_isprint:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_isspace:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_isupper:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_iswspace:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_isxdigit:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_j0:
    return (NumParams == 1 && FTy.getReturnType()->isFloatingPointTy() &&
            FTy.getParamType(0)->isFloatingPointTy());

  case LibFunc_j1:
    return (NumParams == 1 && FTy.getReturnType()->isFloatingPointTy() &&
            FTy.getParamType(0)->isFloatingPointTy());

  case LibFunc_kill:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_kmp_set_blocktime:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_link:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_local_stdio_printf_options:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_local_stdio_scanf_options:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_localeconv:
    return (NumParams == 0 && FTy.getReturnType()->isPointerTy());

  case LibFunc_localtime:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_localtime_r:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_longjmp:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_lseek:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_lseek64:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy(64) &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_mallopt:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
      FTy.getParamType(0)->isIntegerTy() &&
      FTy.getParamType(1)->isIntegerTy());

  case LibFunc_mblen:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_mbstowcs:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_mkdtemp:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_mkstemps:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_mmap:
    return (NumParams == 6 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isIntegerTy() &&
            FTy.getParamType(5)->isIntegerTy());

  case LibFunc_MultiByteToWideChar:
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isIntegerTy());

  case LibFunc_munmap:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_obstack_free:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_omp_destroy_lock:
  case LibFunc_omp_init_lock:
  case LibFunc_omp_set_lock:
  case LibFunc_omp_unset_lock:
  case LibFunc_omp_destroy_nest_lock:
  case LibFunc_omp_init_nest_lock:
  case LibFunc_omp_set_nest_lock:
  case LibFunc_omp_unset_nest_lock:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_omp_init_lock_with_hint:
  case LibFunc_omp_init_nest_lock_with_hint:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_omp_test_lock:
  case LibFunc_omp_test_nest_lock:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_omp_get_active_level:
  case LibFunc_omp_get_cancellation:
  case LibFunc_omp_get_default_device:
  case LibFunc_omp_get_dynamic:
  case LibFunc_omp_get_initial_device:
  case LibFunc_omp_get_level:
  case LibFunc_omp_get_max_active_levels:
  case LibFunc_omp_get_max_task_priority:
  case LibFunc_omp_get_max_threads:
  case LibFunc_omp_get_nested:
  case LibFunc_omp_get_num_devices:
  case LibFunc_omp_get_num_procs:
  case LibFunc_omp_get_num_teams:
  case LibFunc_omp_get_num_threads:
  case LibFunc_omp_get_proc_bind:
  case LibFunc_omp_get_team_num:
  case LibFunc_omp_get_thread_limit:
  case LibFunc_omp_get_thread_num:
  case LibFunc_omp_in_final:
  case LibFunc_omp_in_parallel:
  case LibFunc_omp_is_initial_device:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_omp_get_ancestor_thread_num:
  case LibFunc_omp_get_team_size:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_omp_get_wtick:
  case LibFunc_omp_get_wtime:
    return (NumParams == 0 && FTy.getReturnType()->isDoubleTy());

  case LibFunc_omp_set_default_device:
  case LibFunc_omp_set_dynamic:
  case LibFunc_omp_set_max_active_levels:
  case LibFunc_omp_set_num_threads:
  case LibFunc_omp_set_nested:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_omp_get_schedule:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_omp_set_schedule:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_pipe:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_pthread_key_create:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_pthread_self:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_putenv:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_qsort_r:
    return (NumParams == 5 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_raise:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_rand:
    return (NumParams == 0 && FTy.getReturnType()->isIntegerTy());

  case LibFunc_readdir:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_readdir64:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_scandir:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_select:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_setgid:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_setlocale:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_setrlimit:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_setuid:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_siglongjmp:
    return (NumParams == 2 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_signal:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_signbit:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isDoubleTy());

  case LibFunc_sleep:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_srand:
    return (NumParams == 1 && FTy.getReturnType()->isVoidTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_stdio_common_vfprintf:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_stdio_common_vfscanf:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());

  case LibFunc_stdio_common_vsprintf:
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_dunder_stdio_common_vsprintf_s:
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_stdio_common_vsscanf:
    return (NumParams == 6 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isPointerTy());

  case LibFunc_strerror:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_strftime:
    return (NumParams == 4 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isPointerTy());

  case LibFunc_strsignal:
    return (NumParams == 1 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_symlink:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_sysconf:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_terminate:
    return (NumParams == 0 && FTy.getReturnType()->isVoidTy());

  case LibFunc_time:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy());

  case LibFunc_tolower:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_toupper:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_towlower:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_towupper:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_truncate64:
    return (NumParams == 2 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isIntegerTy());

  case LibFunc_usleep:
    return (NumParams == 1 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy());

  case LibFunc_vasprintf:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_waitpid:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_wcscpy:
    return (NumParams == 2 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy());

  case LibFunc_wcsncat:
    return (NumParams == 3 && FTy.getReturnType()->isPointerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_wcstombs:
    return (NumParams == 3 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy());

  case LibFunc_WideCharToMultiByte:
    return (NumParams == 8 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isIntegerTy() &&
            FTy.getParamType(1)->isIntegerTy() &&
            FTy.getParamType(2)->isPointerTy() &&
            FTy.getParamType(3)->isIntegerTy() &&
            FTy.getParamType(4)->isPointerTy() &&
            FTy.getParamType(5)->isIntegerTy() &&
            FTy.getParamType(6)->isPointerTy() &&
            FTy.getParamType(7)->isPointerTy());

  case LibFunc_WriteFile:
    return (NumParams == 5 && FTy.getReturnType()->isIntegerTy() &&
            FTy.getParamType(0)->isPointerTy() &&
            FTy.getParamType(1)->isPointerTy() &&
            FTy.getParamType(2)->isIntegerTy() &&
            FTy.getParamType(3)->isPointerTy() &&
            FTy.getParamType(4)->isPointerTy());
#endif // INTEL_CUSTOMIZATION
  }

  llvm_unreachable("Invalid libfunc");
}

bool TargetLibraryInfoImpl::getLibFunc(const Function &FDecl,
                                       LibFunc &F) const {
  // Intrinsics don't overlap w/libcalls; if our module has a large number of
  // intrinsics, this ends up being an interesting compile time win since we
  // avoid string normalization and comparison. 
  if (FDecl.isIntrinsic()) return false;

  const Module *M = FDecl.getParent();
  assert(M && "Expecting FDecl to be connected to a Module.");

  return getLibFunc(FDecl.getName(), F) &&
         isValidProtoForLibFunc(*FDecl.getFunctionType(), F, *M);
}

void TargetLibraryInfoImpl::disableAllFunctions() {
  memset(AvailableArray, 0, sizeof(AvailableArray));
}

static bool compareByScalarFnName(const VecDesc &LHS, const VecDesc &RHS) {
  return LHS.ScalarFnName < RHS.ScalarFnName;
}

static bool compareByVectorFnName(const VecDesc &LHS, const VecDesc &RHS) {
  return LHS.VectorFnName < RHS.VectorFnName;
}

static bool compareWithScalarFnName(const VecDesc &LHS, StringRef S) {
  return LHS.ScalarFnName < S;
}

void TargetLibraryInfoImpl::addVectorizableFunctions(ArrayRef<VecDesc> Fns) {
  llvm::append_range(VectorDescs, Fns);
  llvm::sort(VectorDescs, compareByScalarFnName);

  llvm::append_range(ScalarDescs, Fns);
  llvm::sort(ScalarDescs, compareByVectorFnName);
}

void TargetLibraryInfoImpl::addVectorizableFunctionsFromVecLib(
    enum VectorLibrary VecLib) {
#if INTEL_CUSTOMIZATION
  assert(
      CurVectorLibrary == NoLibrary &&
      "Using multiple vector math libraries simultaneously is not supported");
  CurVectorLibrary = VecLib;
#endif // INTEL_CUSTOMIZATION
  switch (VecLib) {
  case Accelerate: {
    const VecDesc VecFuncs[] = {
    #define TLI_DEFINE_ACCELERATE_VECFUNCS
    #include "llvm/Analysis/VecFuncs.def"
    };
    addVectorizableFunctions(VecFuncs);
    break;
  }
  case DarwinLibSystemM: {
    const VecDesc VecFuncs[] = {
    #define TLI_DEFINE_DARWIN_LIBSYSTEM_M_VECFUNCS
    #include "llvm/Analysis/VecFuncs.def"
    };
    addVectorizableFunctions(VecFuncs);
    break;
  }
  case LIBMVEC_X86: {
    const VecDesc VecFuncs[] = {
    #define TLI_DEFINE_LIBMVEC_X86_VECFUNCS
    #include "llvm/Analysis/VecFuncs.def"
    };
    addVectorizableFunctions(VecFuncs);
    break;
  }
  case MASSV: {
    const VecDesc VecFuncs[] = {
    #define TLI_DEFINE_MASSV_VECFUNCS
    #include "llvm/Analysis/VecFuncs.def"
    };
    addVectorizableFunctions(VecFuncs);
    break;
  }
  case SVML: {
    const VecDesc VecFuncs[] = {
#if INTEL_CUSTOMIZATION
#define GET_SVML_VARIANTS
#include "llvm/IR/Intel_SVML.gen"
#undef GET_SVML_VARIANTS
#else
    #define TLI_DEFINE_SVML_VECFUNCS
    #include "llvm/Analysis/VecFuncs.def"
#endif // INTEL_CUSTOMIZATION
    };
    addVectorizableFunctions(VecFuncs);
    break;
  }
#if INTEL_CUSTOMIZATION
  case Libmvec: {
    const VecDesc VecFuncs[] = {
#define GET_LIBMVEC_VARIANTS
#include "llvm/IR/Intel_Libmvec.gen"
#undef GET_LIBMVEC_VARIANTS
    };
    addVectorizableFunctions(VecFuncs);
    break;
  }
#endif // INTEL_CUSTOMIZATION
  case NoLibrary:
    break;
  }
}

#if INTEL_CUSTOMIZATION
bool TargetLibraryInfoImpl::isSVMLEnabled() const {
  return CurVectorLibrary == SVML;
}
#endif // INTEL_CUSTOMIZATION

bool TargetLibraryInfoImpl::isFunctionVectorizable(StringRef funcName,
                                       /* INTEL */ bool IsMasked) const {
  funcName = sanitizeFunctionName(funcName);
  if (funcName.empty())
    return false;

  std::vector<VecDesc>::const_iterator I =
      llvm::lower_bound(VectorDescs, funcName, compareWithScalarFnName);
#if INTEL_CUSTOMIZATION
  // A masked version of a function can be used in unmasked calling context.
  // Hence we return 'true' in this case.
  while (I != VectorDescs.end() && StringRef(I->ScalarFnName) == funcName) {
    if (I->Masked || !IsMasked)
      return true;
    ++I;
  }
  return false;
#endif
}

StringRef
TargetLibraryInfoImpl::getVectorizedFunction(StringRef F,
                                             const ElementCount &VF,
                                             bool Masked) const { // INTEL
  F = sanitizeFunctionName(F);
  if (F.empty())
    return F;
  std::vector<VecDesc>::const_iterator I =
      llvm::lower_bound(VectorDescs, F, compareWithScalarFnName);
  while (I != VectorDescs.end() && StringRef(I->ScalarFnName) == F) {
    if (I->VectorizationFactor == VF && I->Masked == Masked) // INTEL
      return I->VectorFnName;
    ++I;
  }
  return StringRef();
}

TargetLibraryInfo TargetLibraryAnalysis::run(const Function &F,
                                             FunctionAnalysisManager &) {
  if (!BaselineInfoImpl)
    BaselineInfoImpl =
        TargetLibraryInfoImpl(Triple(F.getParent()->getTargetTriple()));
  return TargetLibraryInfo(*BaselineInfoImpl, &F);
}

unsigned TargetLibraryInfoImpl::getWCharSize(const Module &M) const {
  if (auto *ShortWChar = cast_or_null<ConstantAsMetadata>(
      M.getModuleFlag("wchar_size")))
    return cast<ConstantInt>(ShortWChar->getValue())->getZExtValue();
  return 0;
}

TargetLibraryInfoWrapperPass::TargetLibraryInfoWrapperPass()
    : ImmutablePass(ID), TLA(TargetLibraryInfoImpl()) {
  initializeTargetLibraryInfoWrapperPassPass(*PassRegistry::getPassRegistry());
}

TargetLibraryInfoWrapperPass::TargetLibraryInfoWrapperPass(const Triple &T)
    : ImmutablePass(ID), TLA(TargetLibraryInfoImpl(T)) {
  initializeTargetLibraryInfoWrapperPassPass(*PassRegistry::getPassRegistry());
}

TargetLibraryInfoWrapperPass::TargetLibraryInfoWrapperPass(
    const TargetLibraryInfoImpl &TLIImpl)
    : ImmutablePass(ID), TLA(TLIImpl) {
  initializeTargetLibraryInfoWrapperPassPass(*PassRegistry::getPassRegistry());
}

AnalysisKey TargetLibraryAnalysis::Key;

// Register the basic pass.
INITIALIZE_PASS(TargetLibraryInfoWrapperPass, "targetlibinfo",
                "Target Library Information", false, true)
char TargetLibraryInfoWrapperPass::ID = 0;

void TargetLibraryInfoWrapperPass::anchor() {}

void TargetLibraryInfoImpl::getWidestVF(StringRef ScalarF,
                                        ElementCount &FixedVF,
                                        ElementCount &ScalableVF) const {
  ScalarF = sanitizeFunctionName(ScalarF);
  // Use '0' here because a type of the form <vscale x 1 x ElTy> is not the
  // same as a scalar.
  ScalableVF = ElementCount::getScalable(0);
  FixedVF = ElementCount::getFixed(1);
  if (ScalarF.empty())
    return;

  std::vector<VecDesc>::const_iterator I =
      llvm::lower_bound(VectorDescs, ScalarF, compareWithScalarFnName);
  while (I != VectorDescs.end() && StringRef(I->ScalarFnName) == ScalarF) {
    ElementCount *VF =
        I->VectorizationFactor.isScalable() ? &ScalableVF : &FixedVF;
    if (ElementCount::isKnownGT(I->VectorizationFactor, *VF))
      *VF = I->VectorizationFactor;
    ++I;
  }
}
