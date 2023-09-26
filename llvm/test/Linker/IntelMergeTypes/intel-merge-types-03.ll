; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-types-03a.ll %S/Inputs/intel-merge-types-03b.ll | FileCheck %s -check-prefix=CHECK-AB

; This test case checks that the types were merged when there are function
; pointers. In the case of non-opaque pointers, the CFE will generate a
; pointer to an empty structure in one module, and a function pointer
; in another module (check test case
; Linker/IntelMergeTypes/intel-merge-mangled07.ll for more details.). In
; the case of opaque pointers, they are just pointers and the DTrans metadata
; should match in both modules.

; simple.cpp
;   #include "simple.h"
;
;   int func1(TestStructA* T) {
;     return T->F(T);
;   }

; simple.h
;   #ifndef __SIMPLE_H__
;   #define __SIMPLE_H__
;
;   struct TestStructA;
;   using FPType = int(*)(TestStructA*);
;
;   struct TestStructA{
;     FPType F;
;   };
;
;   #endif // __SIMPLE_H__

; simple_2.cpp
;   #include "simple.h"
;
;   int func2(TestStructA* T, int i) {
;     return T->F(T);
;   }


; Check that there is only one type

; CHECK-AB: %struct._ZTS11TestStructA.TestStructA = type { ptr }

; Check that the DTrans metadata was merged

; CHECK-AB: !intel.dtrans.types = !{!0, !0}

; Metadata for
; struct TestStructA{
;   FPType F;
; };

; CHECK-AB: !0 = !{!"S", %struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1, !1}
; CHECK-AB: !1 = !{!2, i32 1}
; CHECK-AB: !2 = !{!"F", i1 false, i32 1, !3, !4}
; CHECK-AB: !3 = !{i32 0, i32 0}


; This test case checks that the types where merged correctly when the order of
; linking changes.

; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-types-03b.ll %S/Inputs/intel-merge-types-03a.ll | FileCheck %s -check-prefix=CHECK-BA

; Check that there is only one type

; CHECK-BA: %struct._ZTS11TestStructA.TestStructA = type { ptr }

; Check that the DTrans metadata was merged

; CHECK-BA: !intel.dtrans.types = !{!0, !0}

; Metadata for
; struct TestStructA{
;   FPType F;
; };

; CHECK-BA: !0 = !{!"S", %struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1, !1}
; CHECK-BA: !1 = !{!2, i32 1}
; CHECK-BA: !2 = !{!"F", i1 false, i32 1, !3, !4}
; CHECK-BA: !3 = !{i32 0, i32 0}

; end INTEL_FEATURE_SW_DTRANS
