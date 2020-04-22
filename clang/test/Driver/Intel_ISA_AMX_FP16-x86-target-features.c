#if INTEL_FEATURE_ISA_AMX_FP16
// REQUIRES: intel_feature_isa_amx_fp16

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-fp16 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-FP16 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-fp16 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-FP16 %s
// AMX-FP16: "-target-feature" "+amx-fp16"
// NO-AMX-FP16: "-target-feature" "-amx-fp16"

#endif // INTEL_FEATURE_ISA_AMX_FP16
