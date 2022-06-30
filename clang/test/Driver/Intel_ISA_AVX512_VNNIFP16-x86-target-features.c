#if INTEL_FEATURE_ISA_AVX512_VNNI_FP16
// REQUIRES: intel_feature_isa_avx512_vnni_fp16

// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mavx512vnnifp16 %s \
// RUN: -### -o %t.o 2>&1 | FileCheck -check-prefix=AVX512-VNNIFP16 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512vnnifp16 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-VNNIFP16 %s
// RUN: %clang -target i386-unknown-linux-gnu -march=i386 -mno-avx512vnnifp16 \
// RUN: %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-AVX512-VNNIFP16 %s
// AVX512-VNNIFP16: "-target-feature" "+avx512vnnifp16"
// NO-AVX512-VNNIFP16: "-target-feature" "-avx512vnnifp16"

#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP16
