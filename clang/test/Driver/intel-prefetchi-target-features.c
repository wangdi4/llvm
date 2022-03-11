// INTEL_FEATURE_ISA_PREFETCHI
// REQUIRES: intel_feature_isa_prefetchi

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mprefetchi %s -### -o %t.o 2>&1 | FileCheck -check-prefix=PREFETCHI %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-prefetchi %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-PREFETCHI %s
// PREFETCHI: "-target-feature" "+prefetchi"
// NO-PREFETCHI: "-target-feature" "-prefetchi"
// end INTEL_FEATURE_ISA_PREFETCHI
