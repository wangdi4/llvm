; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -debug-only=irmover-dtrans-types  -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-mangled09-a.ll %S/Inputs/intel-merge-mangled09-b.ll 2>&1 | FileCheck %s

; This test case checks that the anonymous won't get merged
; if there are multiple structures in one parent structure.
; The test case is the same as intel-merge-mangled09.ll, but it checks
; the debug print. It was created from the following test case:

; file: simple.cpp
;   struct TestStruct {
;     union {
;       int i;
;       char d;
;     };
;
;     union {
;       int j;
;       char c;
;     };
;   };
;
;   int bar(TestStruct *T);
;
;   int foo(TestStruct *T) {
;     return T->i + T->j + bar(T);
;   }

; file: simple2.cpp
;   struct TestStruct {
;     union {
;       int i;
;       char d;
;     };
;
;     union {
;       int j;
;       char c;
;     };
;   };
;
;   int bar(TestStruct *T);
;
;   int foo(TestStruct *T) {
;     return T->i + T->j + bar(T);
;   }

; CHECK: Merging types from source module:
; CHECK-SAME: Inputs/intel-merge-mangled09-a.ll
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct = type { %union._ZTSN10TestStructUt_E.anon, %union._ZTSN10TestStructUt_E.anon.0 }
; CHECK:     Destination type: None
; CHECK: Destination module passed verification

; CHECK: Merging types from source module:
; CHECK-SAME: Inputs/intel-merge-mangled09-b.ll
; CHECK:   Source type: %struct._ZTS10TestStruct.TestStruct.0 = type { %union._ZTSN10TestStructUt_E.anon.1, %union._ZTSN10TestStructUt_E.anon.0.2 }
; CHECK:     Destination type: %struct._ZTS10TestStruct.TestStruct = type { %union._ZTSN10TestStructUt_E.anon, %union._ZTSN10TestStructUt_E.anon.0 }
; CHECK: Destination module passed verification

; Check that %union._ZTSN10TestStructUt_E.anon.0 wasn't replaced with
; %union._ZTSN10TestStructUt_E.anon.

; CHECK: %struct._ZTS10TestStruct.TestStruct = type { %union._ZTSN10TestStructUt_E.anon, %union._ZTSN10TestStructUt_E.anon.0 }
; CHECK: %union._ZTSN10TestStructUt_E.anon = type { i32 }
; CHECK: %union._ZTSN10TestStructUt_E.anon.0 = type { i32 }

; Check the DTrans metadata.

; CHECK: !intel.dtrans.types = !{!0, !3, !5, !0, !3, !5}

; CHECK: !0 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !1, !2}
; CHECK: !1 = !{%union._ZTSN10TestStructUt_E.anon zeroinitializer, i32 0}
; CHECK: !2 = !{%union._ZTSN10TestStructUt_E.anon.0 zeroinitializer, i32 0}
; CHECK: !3 = !{!"S", %union._ZTSN10TestStructUt_E.anon zeroinitializer, i32 1, !4}
; CHECK: !4 = !{i32 0, i32 0}
; CHECK: !5 = !{!"S", %union._ZTSN10TestStructUt_E.anon.0 zeroinitializer, i32 1, !4}

;end INTEL_FEATURE_SW_DTRANS
