// INTEL_FEATURE_CPU_ADL
// REQUIRES: intel_feature_cpu_adl

// RUN: not %clang_cc1 -triple i386--- -target-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix X86
// X86: error: unknown target CPU 'not-a-cpu'
// X86: note: {{.*}}alderlake{{.*}}

// RUN: not %clang_cc1 -triple x86_64--- -target-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix X86_64
// X86_64: error: unknown target CPU 'not-a-cpu'
// X86_64: note: {{.*}}alderlake{{.*}}

// end INTEL_FEATURE_CPU_ADL
