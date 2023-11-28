; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-types-08a.ll %S/Inputs/intel-merge-types-08b.ll | FileCheck %s

; This test case checks that the structures without mangled names were merged
; correctly. It was created from the following test by adding %struct.ident_t,
; which will be added by PAROPT:

; file: simple.cpp
;   struct TestStruct {
;     int *arrptr[5];
;   };
;
;   int bar(int i);
;
;   int foo(TestStruct *T, int i) {
;     return T->arrptr[i][i] + bar(i);
;   }

; CHECK: %struct.ident_t = type { i32, i32, i32, i32, ptr }
; CHECK-NOT: %struct.ident_t. = type { i32, i32, i32, i32, ptr }

; CHECK: !intel.dtrans.types = !{!1, !3, !6, !3}

; CHECK: !3 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !4, !4, !4, !4, !5}
; CHECK: !4 = !{i32 0, i32 0}
; CHECK: !5 = !{i8 0, i32 1}

; end INTEL_FEATURE_SW_DTRANS
