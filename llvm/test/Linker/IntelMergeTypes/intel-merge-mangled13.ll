; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-mangled13-a.ll %S/Inputs/intel-merge-mangled13-b.ll | FileCheck %s

; This test case checks that the structures without mangled names were merged
; correctly. It is the same test case as intel-merge-mangled11.ll with the
; structure %struct.ident_t added. It was created from the following test
; by adding %struct.ident_t, which will be added by PAROPT:

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


; file: simple2.cpp
;   struct TestStruct {
;     int arr[5];
;   };
;
;   TestStruct *glob;
;
;   int bar(int i) {
;     return glob->arr[i];
;   }

; CHECK: %struct.ident_t = type { i32, i32, i32, i32, ptr }
; CHECK-NOT: %struct.ident_t.

; CHECK: !intel.dtrans.types = !{!1, !3, !6, !3}
; CHECK: !3 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !4, !4, !4, !4, !5}

;end INTEL_FEATURE_SW_DTRANS
