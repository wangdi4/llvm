// INTEL_FEATURE_CPU_DMR
// REQUIRES: intel_feature_cpu_dmr

// RUN: not %clang_cc1 -triple i386--- -target-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix X86
// X86: error: unknown target CPU 'not-a-cpu'
// X86: note: {{.*}}diamondrapids{{.*}}

// RUN: not %clang_cc1 -triple x86_64--- -target-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix X86_64
// X86_64: error: unknown target CPU 'not-a-cpu'
// X86_64: note: {{.*}}diamondrapids{{.*}}

// end INTEL_FEATURE_CPU_DMR
