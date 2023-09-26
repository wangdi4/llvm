; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-mangled11-a.ll %S/Inputs/intel-merge-mangled11-b.ll | FileCheck %s

; This test case checks that the metadata based type merging is not affected
; when one field of a structure is an array of pointers and the other is not.

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


; file: simple2.cpp
;   struct TestStruct {
;     int arr[5];
;   };
;
;   TestStruct *glob;
;
;   int bar(int i) {
;     return glob->arr[i];
;   }

; CHECK: %struct._ZTS10TestStruct.TestStruct = type { [5 x ptr] }
; CHECK: %struct._ZTS10TestStruct.TestStruct.0 = type { [5 x i32] }

; CHECK: !intel.dtrans.types = !{!1, !4}

; CHECK: !1 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1, !2}
; CHECK: !2 = !{!"A", i32 5, !3}
; CHECK: !3 = !{i32 0, i32 1}

; CHECK: !4 = !{!"S", %struct._ZTS10TestStruct.TestStruct.0 zeroinitializer, i32 1, !5}
; CHECK: !5 = !{!"A", i32 5, !6}
; CHECK: !6 = !{i32 0, i32 0}

;end INTEL_FEATURE_SW_DTRANS
