#if INTEL_FEATURE_ISA_AMX_BF16_EVEX
// REQUIRES: intel_feature_isa_amx_bf16_evex

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-bf16-evex %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-BF16-EVEX %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-bf16-evex \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-BF16-EVEX %s
// AMX-BF16-EVEX: "-target-feature" "+amx-bf16-evex"
// NO-AMX-BF16-EVEX: "-target-feature" "-amx-bf16-evex"

#endif // INTEL_FEATURE_ISA_AMX_BF16_EVEX
