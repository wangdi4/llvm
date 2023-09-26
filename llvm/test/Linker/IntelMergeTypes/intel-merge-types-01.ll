; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-types-01a.ll %S/Inputs/intel-merge-types-01b.ll | FileCheck %s

; This test case checks that the types aren't merged during the IR mover since
; the pointer types don't match. It represents the following C/C++ source code:

; file: simple.cpp
;   struct TestStruct {
;     int *ptr;
;   };
;
;   double bar(int i);
;
;   int foo(TestStruct *T, int i) {
;     return T->ptr[i] + (int)bar(i);
;   }

; file: simple2.cpp
;   struct TestStruct {
;     double *ptr;
;   };
;
;   TestStruct *glob;
;
;   double bar(int i) {
;     return glob->ptr[i];
;   }


; Check that both structures are in the IR

; CHECK: %struct._ZTS10TestStruct.TestStruct = type { ptr }
; CHECK: %struct._ZTS10TestStruct.TestStruct.0 = type { ptr }

; Check that the DTrans metadata wasn't merged

; CHECK: !intel.dtrans.types = !{!1, !3}

; struct TestStruct {
;    int *ptr;
; }

; CHECK: !1 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1, !2}
; CHECK: !2 = !{i32 0, i32 1}

; struct TestStruct {
;    double *ptr;
; }

; CHECK: !3 = !{!"S", %struct._ZTS10TestStruct.TestStruct.0 zeroinitializer, i32 1, !4}
; CHECK: !4 = !{double 0.000000e+00, i32 1}

; end INTEL_FEATURE_SW_DTRANS
