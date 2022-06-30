; Check that relaxed math builtin implementations are resolved correctly.

; RUN: SATest -BUILD --config=%s.cfg -tsize=8 -cpuarch="core-avx2" --dump-llvm-file - | FileCheck %s --check-prefix=CHECK-AVX2
; RUN: SATest -BUILD --config=%s.cfg -tsize=16 -cpuarch="skx" --dump-llvm-file - | FileCheck %s --check-prefix=CHECK-SKX

CHECK-AVX2: call {{.*}} <8 x float> @__ocl_svml_{{[sl]}}9_sinf8_rm(

CHECK-SKX: call {{.*}} <16 x float> @__ocl_svml_{{[xz]}}0_sinf16_rm(
