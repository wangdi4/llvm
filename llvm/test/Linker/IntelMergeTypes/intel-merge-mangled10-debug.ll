; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -debug-only=irmover-mangled-names -irmover-enable-merge-by-mangled-names -irmover-enable-module-verify -irmover-type-merging=false -irmover-enable-full-dtrans-types-check -S %S/Inputs/intel-merge-mangled10-a.ll %S/Inputs/intel-merge-mangled10-b.ll 2>&1 | FileCheck %s

; This test case checks that the type merging is not affected when the
; metadata is missing and there are typed pointers. This is the same
; test case intel-merge-mangles10.ll, but it checks the debug information.

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
;     return T->i[in] + bar(T, in);
;   }

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-mangled10-a.ll
; CHECK:   WARNING: DTrans metadata collected incorrectly from source
; CHECK:   WARNING: DTrans metadata collected incorrectly from destination
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct = type { i32*, i32* }
; CHECK:     Destination type: None
; CHECK:     Fields that will be repaired:
; CHECK: Destination module passed verification


; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-mangled10-b.ll
; CHECK:   WARNING: DTrans metadata collected incorrectly from source
; CHECK:   WARNING: DTrans metadata collected incorrectly from destination
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct.0 = type { i32*, i32* }
; CHECK:     Destination type: %struct._ZTS10TestStruct.TestStruct = type { i32*, i32* }
; CHECK: Destination module passed verification


; CHECK: %struct._ZTS10TestStruct.TestStruct = type { i32*, i32* }
; CHECK-NOT: .TestStruct.{{[0-9]+}} =

; CHECK: !intel.dtrans.types = !{!0, !0}

; CHECK: !0 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !1}
; CHECK: !1 = !{i32 0, i32 1}

;end INTEL_FEATURE_SW_DTRANS