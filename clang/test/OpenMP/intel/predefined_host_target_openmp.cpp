// INTEL_COLLAB
// RUN: %clang_cc1 -fopenmp -verify -DNO_OPENMP -o - %s
//
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline -verify -DHOST_OPENMP \
// RUN: -o - %s
//
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm-bc -disable-llvm-passes \
// RUN:  -fopenmp -fopenmp-targets=spir64,spir \
// RUN:  -fopenmp-late-outline -fintel-compatibility \
// RUN:  -o %t_host.bc %s
//
// RUN: %clang_cc1 -triple spir64 \
// RUN:  -fopenmp -fopenmp-targets=spir64,spir \
// RUN:  -fopenmp-late-outline -fintel-compatibility \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
// RUN:  -verify -DTARGET_OPENMP -o - %s
// expected-no-diagnostics

// Verify predefined macros for -fiopenmp and -fiopenmp -fopenmp-targets=

#ifdef HOST_OPENMP
// -fiopenmp/-fopenmp-late-outline specified
#ifndef __INTEL_HOST_OPENMP
#error "__INTEL_HOST_OPENMP macro undefined with -fiopenmp option"
#endif
#elif defined(TARGET_OPENMP)
// -fiopenmp/-fopenmp-late-outline and -fopenmp-targets= specified
#ifndef __INTEL_TARGET_OPENMP
#error "__INTEL_TARGET_OPENMP macro undefined with -fiopenmp -fopenmp-targets="
#endif
#elif defined(NO_OPENMP)
#if defined(__INTEL_TARGET_OPENMP) || defined(__INTEL_HOST_OPENMP)
#error "__INTEL_[HOST|TARGET]_OPENMP macro defined without -fiopenmp flag"
#endif
#endif
// end INTEL_COLLAB
