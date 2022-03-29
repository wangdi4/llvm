;;; Test masked builtin implementation

; RUN: SATest -BUILD --config=%s.cfg --cpuarch=skx -tsize=16 --vectorizer-type=vpo --dump-llvm-file - | FileCheck %s

; Test kernel
; __kernel void test_masked_acos(__global double *a, __global double *c) {
;     int i = get_global_id(0);
;     if (a[i] <= 100 && a[i] >= -100) {
;         c[i] = acosh(a[i]);
;     }
; }

; Expected masked implementation
; double4 __attribute__((const)) __attribute__((overloadable)) acosh(double4 x, double4 mask)
; {
;   double4 mx = bitselect((double4)(4.2), x, mask);
;   return acosh(mx);
; }

; This test will check whether the desired default values are fed correctly.

; CHECK:      [[SELECT:%.*]] = select <16 x i1> {{.*}}, <16 x double> {{.*}}, <16 x double> <double 4.200000e+00
; CHECK-NEXT: [[DOUBLE1:%.*]] = shufflevector <16 x double> [[SELECT]], <16 x double> poison, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT: call contract intel_ocl_bicc_avx512 <8 x double> @__ocl_svml_z0_acosh8(<8 x double> noundef [[DOUBLE1]])
; CHECK-NEXT: [[DOUBLE2:%.*]] = shufflevector <16 x double> [[SELECT]], <16 x double> poison, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT: call contract intel_ocl_bicc_avx512 <8 x double> @__ocl_svml_z0_acosh8(<8 x double> noundef [[DOUBLE2]])
