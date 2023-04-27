#if INTEL_FEATURE_ISA_UMSR
// REQUIRES: intel_feature_isa_umsr

// RUN: %clang -target i686-unknown-linux-gnu -mumsr %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=UMSR %s
// RUN: %clang -target i686-unknown-linux-gnu -mno-umsr %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-UMSR %s
// UMSR: "-target-feature" "+umsr"
// NO-UMSR: "-target-feature" "-umsr"
#endif // INTEL_FEATURE_ISA_UMSR
