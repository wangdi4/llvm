#if INTEL_FEATURE_CPU_GRT
// REQUIRES: intel_feature_cpu_grt

// RUN: not %clang_cc1 -triple i386--- -target-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix X86
// X86: gracemont

// RUN: not %clang_cc1 -triple x86_64--- -target-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix X86_64
// X86_64: gracemont

// RUN: not %clang_cc1 -triple i386--- -tune-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix TUNE_X86
// TUNE_X86: gracemont

// RUN: not %clang_cc1 -triple x86_64--- -tune-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix TUNE_X86_64
// TUNE_X86_64: gracemont
#endif // INTEL_FEATURE_CPU_GRT
