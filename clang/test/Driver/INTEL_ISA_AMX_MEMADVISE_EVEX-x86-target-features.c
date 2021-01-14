#if INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
// REQUIRES: intel_feature_isa_amx_memadvise_evex

// RUN: %clang -target x86_64-unknown-linux-gnu -mamx-memadvise-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-MEMADVISE-EVEX %s
// RUN: %clang -target x86_64-unknown-linux-gnu -mno-amx-memadvise-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-MEMADVISE-EVEX %s
// AMX-MEMADVISE-EVEX: "-target-feature" "+amx-memadvise-evex"
// NO-AMX-MEMADVISE-EVEX: "-target-feature" "-amx-memadvise-evex"
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX