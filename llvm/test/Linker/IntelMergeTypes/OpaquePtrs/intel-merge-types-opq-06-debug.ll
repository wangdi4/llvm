; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -debug-only=irmover-mangled-names -irmover-enable-merge-by-mangled-names -irmover-enable-dtrans-incomplete-metadata -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-types-opq-06a.ll %S/Inputs/intel-merge-types-opq-06b.ll 2>&1 | FileCheck %s

; This test case checks that the types aren't merged during the IR mover since
; the arrays type won't match. This is the same test case as
; intel-merge-types-opq-05.ll but it checks the debug information. It
; represents the following C/C++ source code:

; file: simple.cpp
;   struct TestStruct {
;     int* ptr[8];
;   };
;
;   double bar(int i, int j);
;
;   int foo(TestStruct *T, int i, int j) {
;     return T->ptr[i][j] + (int)bar(i,j);
;   }

; file: simple2.cpp
;   struct TestStruct {
;     double* ptr[8];
;   };
;
;   TestStruct *glob;
;
;   double bar(int i, int j) {
;     return glob->ptr[i][j];
;   }

; Check the debug output

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-types-opq-06a.ll
; CHECK:   WARNING: DTrans metadata collected incorrectly from destination
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct = type { [8 x ptr] }
; CHECK:     Destination type: None
; CHECK:     Fields that will be repaired:
; CHECK: Destination module passed verification

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-types-opq-06b.ll
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct.0 = type { [8 x ptr] }
; CHECK:     Destination type: None
; CHECK:     Fields that will be repaired:
; CHECK: Destination module passed verification

; Check that both structures are in the IR

; CHECK: %struct._ZTS10TestStruct.TestStruct = type { [8 x ptr] }
; CHECK: %struct._ZTS10TestStruct.TestStruct.0 = type { [8 x ptr] }

; Check that the DTrans metadata wasn't merged

; CHECK: !intel.dtrans.types = !{!1, !4}

; struct TestStruct {
;    int* ptr[8];
; }

; CHECK: !1 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1, !2}
; CHECK: !2 = !{!"A", i32 8, !3}
; CHECK: !3 = !{i32 0, i32 1}

; struct TestStruct {
;    double* ptr[8];
; }

; CHECK: !4 = !{!"S", %struct._ZTS10TestStruct.TestStruct.0 zeroinitializer, i32 1, !5}
; CHECK: !5 = !{!"A", i32 8, !6}
; CHECK: !6 = !{double 0.000000e+00, i32 1}

; end INTEL_FEATURE_SW_DTRANS