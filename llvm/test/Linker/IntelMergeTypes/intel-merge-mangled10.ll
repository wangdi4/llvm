; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -irmover-enable-full-dtrans-types-check -S %S/Inputs/intel-merge-mangled10-a.ll %S/Inputs/intel-merge-mangled10-b.ll | FileCheck %s

; This test case checks that the type merging is not affected when the
; metadata is missing and there are typed pointers.

; file: simple.cpp
;   struct TestStruct {
;     int *i;
;     int *j;
;   };
;
;   int bar(TestStruct *T, int in);
;
;   int foo(TestStruct *T, int in) {
;     return T->i[in] + bar(T, in);
;   }

; file: simple2.cpp
;   struct TestStruct {
;     int *i;
;     int *j;
;   };
;
;   int bar(TestStruct *T, int in) {
;     return T->i[in];
;   }

; CHECK: %struct._ZTS10TestStruct.TestStruct = type { i32*, i32* }
; CHECK-NOT: .TestStruct.{{[0-9]+}} =

; CHECK: !intel.dtrans.types = !{!0, !0}

; CHECK: !0 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !1}
; CHECK: !1 = !{i32 0, i32 1}

;end INTEL_FEATURE_SW_DTRANS