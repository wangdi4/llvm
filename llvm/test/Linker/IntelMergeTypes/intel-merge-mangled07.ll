; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -irmover-enable-full-dtrans-types-check -S %S/Inputs/intel-merge-mangled07-a.ll %S/Inputs/intel-merge-mangled07-b.ll 2>&1 | FileCheck %s -check-prefix=CHECK-AB

; NOTE: These test cases can be removed once we move to opaque pointers.

; This test case checks that the types were correctly merged when the
; structures contain function pointers but the CFE created them incomplete.
; This case checks when the CFE generates an incomplete structure and there
; is no information to map it to the destination module, therefore the
; type needs to be repaired before inserting it. The test case was created
; using the following example:

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

; Compiling simple.cpp will generate the structure and the field is a pointer
; to an empty structure (Inputs/intel-merge-mangled07-a.ll). Then, compiling
; simple_2.cpp will generate the structure where the field is a pointer to a
; function (Inputs/intel-merge-mangled07-b.ll). The IR mover will use the
; DTrans metadata to create a type in the destination module when linking
; simple.cpp (Inputs/intel-merge-mangled07-a.ll) and there is no information
; about the type. Then, when linking simple_2.cpp
; (Inputs/intel-merge-mangled07-b.ll), the type is already in the destination
; module, therefore it will merge it.

; CHECK-AB: %struct._ZTS11TestStructA.TestStructA = type { i32 (%struct._ZTS11TestStructA.TestStructA*)* }

; CHECK-AB-NOT:  .TestClass.{{[0-9]+}} =
; CHECK-AB-NOT: %"__Intel$Empty$Struct" =

; CHECK-AB: !intel.dtrans.types = !{!0, !0}

; CHECK-AB: !0 = !{!"S", %struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1, !1}
; CHECK-AB: !1 = !{!2, i32 1}
; CHECK-AB: !2 = !{!"F", i1 false, i32 1, !3, !4}
; CHECK-AB: !3 = !{i32 0, i32 0}
; CHECK-AB: !4 = !{%struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1}


; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-enable-module-verify -irmover-type-merging=false -S %S/Inputs/intel-merge-mangled07-b.ll %S/Inputs/intel-merge-mangled07-a.ll 2>&1 | FileCheck %s -check-prefix=CHECK-BA

; This test checks that the incomplete type is linked correctly when the linking
; order is changed. In this case, there is no type to repair because the
; destination module already has the correct type.


; CHECK-BA: %struct._ZTS11TestStructA.TestStructA = type { i32 (%struct._ZTS11TestStructA.TestStructA*)* }

; CHECK-BA-NOT:  .TestClass.{{[0-9]+}} =
; CHECK-BA-NOT: %"__Intel$Empty$Struct" =

; CHECK-BA: !intel.dtrans.types = !{!0, !0}

; CHECK-BA: !0 = !{!"S", %struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1, !1}
; CHECK-BA: !1 = !{!2, i32 1}
; CHECK-BA: !2 = !{!"F", i1 false, i32 1, !3, !4}
; CHECK-BA: !3 = !{i32 0, i32 0}
; CHECK-BA: !4 = !{%struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1}


;end INTEL_FEATURE_SW_DTRANS