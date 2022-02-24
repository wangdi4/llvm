// INTEL_FEATURE_ISA_PREFETCHST2
// REQUIRES: intel_feature_isa_prefetchst2

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mprefetchst2 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=PREFETCHST2 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-prefetchst2 %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-PREFETCHST2 %s
// PREFETCHST2: "-target-feature" "+prefetchst2"
// NO-PREFETCHST2: "-target-feature" "-prefetchst2"
// end INTEL_FEATURE_ISA_PREFETCHST2
