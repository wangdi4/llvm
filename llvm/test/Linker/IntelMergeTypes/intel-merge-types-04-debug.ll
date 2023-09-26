; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -debug-only=irmover-dtrans-types -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-types-04a.ll %S/Inputs/intel-merge-types-04b.ll 2>&1 | FileCheck %s

; This test case checks that the types aren't merged during the IR mover since
; the pointers in the anonymous structures won't match. This is the same test
; case as intel-merge-types-04.ll but it checks the debug output.

; file: simple.cpp
;   struct TestStruct {
;     int *ptr;
;     struct {
;       int *inner_ptr;
;     };
;   };
;
;   double bar(int i);
;
;   int foo(TestStruct *T, int i) {
;     return T->ptr[i] + T->inner_ptr[i] + (int)bar(i);
;   }


; file: simple2.cpp
;   struct TestStruct {
;     int *ptr;
;     struct {
;       double *inner_ptr;
;     };
;   };
;
;   TestStruct *glob;
;
;   double bar(int i) {
;     return glob->inner_ptr[i] + (double)glob->ptr[i];
;   }


; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-types-04a.ll
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct = type { ptr, %struct._ZTSN10TestStructUt_E.anon }
; CHECK:     Destination type: None
; CHECK: Destination module passed verification

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-types-04b.ll
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct.0 = type { ptr, %struct._ZTSN10TestStructUt_E.anon.1 }
; CHECK:     Destination type: None
; CHECK: Destination module passed verification

; Check that the structures weren't merged

; CHECK: %struct._ZTS10TestStruct.TestStruct = type { ptr, %struct._ZTSN10TestStructUt_E.anon }
; CHECK: %struct._ZTSN10TestStructUt_E.anon = type { ptr }
; CHECK: %struct._ZTS10TestStruct.TestStruct.0 = type { ptr, %struct._ZTSN10TestStructUt_E.anon.1 }
; CHECK: %struct._ZTSN10TestStructUt_E.anon.1 = type { ptr }

; Check that the DTrans metadata wasn't merged

; CHECK: !intel.dtrans.types = !{!1, !4, !5, !7}

; struct TestStruct {
;   int *ptr;
;   struct {
;     int *inner_ptr;
;   };
; };

; CHECK: !1 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !2, !3}
; CHECK: !2 = !{i32 0, i32 1}
; CHECK: !3 = !{%struct._ZTSN10TestStructUt_E.anon zeroinitializer, i32 0}
; CHECK: !4 = !{!"S", %struct._ZTSN10TestStructUt_E.anon zeroinitializer, i32 1, !2}

; struct TestStruct {
;   int *ptr;
;   struct {
;     double *inner_ptr;
;   };
; };

; CHECK: !5 = !{!"S", %struct._ZTS10TestStruct.TestStruct.0 zeroinitializer, i32 2, !2, !6}
; CHECK: !6 = !{%struct._ZTSN10TestStructUt_E.anon.1 zeroinitializer, i32 0}
; CHECK: !7 = !{!"S", %struct._ZTSN10TestStructUt_E.anon.1 zeroinitializer, i32 1, !8}
; CHECK: !8 = !{double 0.000000e+00, i32 1}

; end INTEL_FEATURE_SW_DTRANS
