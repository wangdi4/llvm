; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-dtrans-incomplete-metadata -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-types-07a.ll %S/Inputs/intel-merge-types-07b.ll %S/Inputs/intel-merge-types-07c.ll | FileCheck %s

; This test case checks that the types from the module
; intel-merge-types-07a.ll and intel-merge-types-07c.ll were merged
; correctly, while the type from module intel-merge-types-07b.ll wasn't
; merged since it is a different structure. It represents the following C/C++
; source code:

; file: simple.cpp
;   struct TestStruct {
;     int *ptr;
;   };
;
;   double bar(int i);
;   int bas(int i);
;
;   int foo(TestStruct *T, int i) {
;     return T->ptr[i] + (double)bar(i) + bas(i);
;   }

; file: simple2.cpp
;   struct TestStruct {
;     double *ptr;
;   };
;
;   TestStruct *globA;
;
;   double bar(int i) {
;     return globA->ptr[i][i];
;   }

; file: simple3.cpp
;   struct TestStruct {
;     int *ptr;
;   };
;
;   TestStruct *globB;
;
;   int bar(int i) {
;     return globB->ptr[i][i];
;   }

; Check that both structures are in the IR

; CHECK: %struct._ZTS10TestStruct.TestStruct = type { ptr }
; CHECK: %struct._ZTS10TestStruct.TestStruct.0 = type { ptr }

; Check that the globals have the correct type

; CHECK: @globA = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0
; CHECK: @globB = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !1

; Check that the DTrans metadata wasn't merged

; CHECK: !intel.dtrans.types = !{!2, !4, !2}

; Check that the types of the globals are correct

; CHECK: !0 = !{%struct._ZTS10TestStruct.TestStruct.0 zeroinitializer, i32 1}
; CHECK: !1 = !{%struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1}

; Check the structures metadata

; struct TestStruct {
;    int *ptr;
; }

; CHECK: !2 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1, !3}
; CHECK: !3 = !{i32 0, i32 1}

; struct TestStruct {
;    double *ptr;
; }

; CHECK: !4 = !{!"S", %struct._ZTS10TestStruct.TestStruct.0 zeroinitializer, i32 1, !5}
; CHECK: !5 = !{double 0.000000e+00, i32 1}

; end INTEL_FEATURE_SW_DTRANS
