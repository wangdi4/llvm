; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -irmover-trace-dtrans-metadata-loss -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -irmover-enable-full-dtrans-types-check -S %S/Inputs/intel-merge-mangled12-a.ll %S/Inputs/intel-merge-mangled12-b.ll 2>&1 | FileCheck %s

; This test case checks that the information is printed correctly when the
; metadata is missing in the input modules. It was created from the following
; C++ example, but the metadata for TestStructB was removed in order to
; expose the issue of missing metadata.

; file: simple.cpp
;   struct TestStructA {
;     int *i;
;     int *j;
;   };
;
;   struct TestStructB {
;     int *i;
;     int *j;
;   };
;
;   int bar(TestStructA *T, TestStructB *S, int in);
;
;   int foo(TestStructA *T, TestStructB *S, int in) {
;     return T->i[in] + bar(T, S, in);
;   }

; file: simple2.cpp
;   struct TestStructA {
;     int *i;
;     int *j;
;   };
;
;   struct TestStructB {
;     int *i;
;     int *j;
;   };
;
;   int bar(TestStructA *T, TestStructB *S, int in) {
;     return T->i[in] + S->i[in];
;   }


; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-mangled12-a.ll
; CHECK:   Checking for metadata loss in source module:
; CHECK:     llvm::Type: %struct._ZTS11TestStructB.TestStructB = type { i32*, i32* }
; CHECK:     DTransType: None
; CHECK:   Checking for metadata loss in destination module verification:
; CHECK:     llvm::Type: %struct._ZTS11TestStructB.TestStructB = type { i32*, i32* }
; CHECK:     DTransType: None
; CHECK: Warning: Missing DTrans type in metadata for: %struct._ZTS11TestStructB.TestStructB = type { i32*, i32* }

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-mangled12-b.ll
; CHECK:   Checking for metadata loss in source module:
; CHECK:     llvm::Type: %struct._ZTS11TestStructB.TestStructB.1 = type { i32*, i32* }
; CHECK:     DTransType: None
; CHECK:   Checking for metadata loss in destination module verification:
; CHECK:     llvm::Type: %struct._ZTS11TestStructB.TestStructB = type { i32*, i32* }
; CHECK:     DTransType: None
; CHECK: Warning: Missing DTrans type in metadata for: %struct._ZTS11TestStructB.TestStructB = type { i32*, i32* }


;end INTEL_FEATURE_SW_DTRANS