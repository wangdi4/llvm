; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -debug-only=irmover-dtrans-types -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-types-08a.ll %S/Inputs/intel-merge-types-08b.ll 2>&1 | FileCheck %s

; This test case checks that the structures without mangled names were merged
; correctly. It is the same test case as intel-merge-types-08.ll, but it
; checks the debug output. This test case was created from the following test
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

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-types-08a.ll
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct = type { ptr }
; CHECK:     Destination type: None
; CHECK:   Source type: %struct.ident_t = type { i32, i32, i32, i32, ptr }
; CHECK:     Destination type: None
; CHECK: Destination module passed verification

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-types-08b.ll
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct.0 = type { ptr }
; CHECK:     Destination type: None
; CHECK:   Source type: %struct.ident_t.1 = type { i32, i32, i32, i32, ptr }
; CHECK:     Destination type: %struct.ident_t = type { i32, i32, i32, i32, ptr }
; CHECK: Destination module passed verification

; CHECK: %struct.ident_t = type { i32, i32, i32, i32, ptr }
; CHECK-NOT: %struct.ident_t. = type { i32, i32, i32, i32, ptr }

; CHECK: !intel.dtrans.types = !{!1, !3, !6, !3}

; CHECK: !3 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !4, !4, !4, !4, !5}
; CHECK: !4 = !{i32 0, i32 0}
; CHECK: !5 = !{i8 0, i32 1}

; end INTEL_FEATURE_SW_DTRANS
