; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -irmover-trace-dtrans-metadata-loss -irmover-enable-merge-with-dtrans -irmover-type-merging=false -opaque-pointers -S %S/Inputs/intel-merge-types-opq-09a.ll %S/Inputs/intel-merge-types-opq-09b.ll 2>&1 | FileCheck %s

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

; Check that there is no metadata loss in the source module
; intel-merge-types-opq-09a.ll.

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-types-opq-09a.ll

; CHECK: Checking for metadata loss in source module:
; CHECK-NEXT: Checking for metadata loss in destination module:
; CHECK-EMPTY:

; Check that the metadata for %struct._ZTS11TestStructB.TestStructB
; in the source module intel-merge-types-opq-09b.ll is missing.

; CHECK: Merging types from source module:
; CHECK-SAME: intel-merge-types-opq-09b.ll

; CHECK:     llvm::Type: %struct._ZTS11TestStructB.TestStructB = type { ptr, ptr }
; CHECK:     DTransType: None

;end INTEL_FEATURE_SW_DTRANS