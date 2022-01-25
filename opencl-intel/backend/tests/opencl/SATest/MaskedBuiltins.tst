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

; Explanation for the 'magic' number:
; (i64)4616414798036126925 -> 0x4010CCCCCCCCCCCD -> (double)4.20000000000000017763568394003

; CHECK:        [[AND1:%.*]] = and <8 x i64> [[CONDITION1:%.*]], <i64 4616414798036126925,
; CHECK-NEXT:   [[XOR1:%.*]] = xor <8 x i64> [[AND1]], <i64 4616414798036126925,

; CHECK:        [[AND2:%.*]] = and <8 x i64> [[CONDITION1]], [[GATHER1:%.*]]
; CHECK-NEXT:   [[OR1:%.*]] = or <8 x i64> [[XOR1]], [[AND2]]
; CHECK-NEXT:   [[ORDOUBLE1:%.*]] = bitcast <8 x i64> [[OR1]] to <8 x double>

; CHECK:        [[AND3:%.*]] = and <8 x i64> [[CONDITION2:%.*]], <i64 4616414798036126925,
; CHECK-NEXT:   [[XOR2:%.*]] = xor <8 x i64> [[AND3]], <i64 4616414798036126925,

; CHECK:        [[AND4:%.*]] = and <8 x i64> [[CONDITION2]], [[GATHER2:%.*]]
; CHECK-NEXT:   [[OR2:%.*]] = or <8 x i64> [[XOR2]], [[AND4]]
; CHECK-NEXT:   [[ORDOUBLE2:%.*]] = bitcast <8 x i64> [[OR2]] to <8 x double>
; CHECK-NEXT:   call contract intel_ocl_bicc_avx512 <8 x double> @__ocl_svml_z0_acosh8(<8 x double> noundef [[ORDOUBLE1]])
; CHECK-NEXT:   call contract intel_ocl_bicc_avx512 <8 x double> @__ocl_svml_z0_acosh8(<8 x double> noundef [[ORDOUBLE2]])
