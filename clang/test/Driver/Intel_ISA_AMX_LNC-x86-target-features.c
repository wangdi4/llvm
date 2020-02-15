#if INTEL_FEATURE_ISA_AMX_LNC
// REQUIRES: intel_feature_isa_amx_lnc

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-transpose %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-TRANSPOSE %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-transpose \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-TRANSPOSE %s
// AMX-TRANSPOSE: "-target-feature" "+amx-transpose"
// NO-AMX-TRANSPOSE: "-target-feature" "-amx-transpose"

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mamx-avx512 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AMX-AVX512 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-amx-avx512 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AMX-AVX512 %s
// AMX-AVX512: "-target-feature" "+amx-avx512"
// NO-AMX-AVX512: "-target-feature" "-amx-avx512"

#endif // INTEL_FEATURE_ISA_AMX_LNC
