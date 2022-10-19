; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-safetyanalyzer -disable-output -debug-only=dtrans-safetyanalyzer-verbose -dtrans-print-types 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer-verbose -dtrans-print-types 2>&1 | FileCheck %s

; This test case checks that "Bad pointer manipulation (related types) -- Byte
; flattened GEP used for accessing next field" was printed for the GEP %5 in
; function @test. The reason is because the store instruction is moving data
; within the same structure. Also, this test case checks that the structure
; %class.TestClass.Inner was set to "Bad pointer manipulation (related types)".

; CHECK: dtrans-safety: Bad pointer manipulation (related types) -- Byte flattened GEP used for accessing next field
; CHECK:   [foo]   %5 = getelementptr i8, ptr %3, i64 %4
; CHECK:    LocalPointerInfo: CompletelyAnalyzed
; CHECK:    Declared Types:
; CHECK:      Aliased types:
; CHECK:        %class.TestClass.Inner*
; CHECK:      No element pointees.
; CHECK:    Usage Types:
; CHECK:      Aliased types:
; CHECK:        %class.TestClass.Inner*
; CHECK:      No element pointees.

; CHECK:  Name: class.TestClass.Inner
; CHECK:   Safety data: Nested structure | Contains nested structure | Bad casting (related types) | Bad pointer manipulation (related types) | Mismatched element access (related types) | Unsafe pointer store (related types){{.*}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.MainClass = type { [4 x i32], i32, [4 x i8]}
%class.MainClass.base = type { [4 x i32], i32}

%class.TestClass.Outer = type { %class.TestClass.Inner, %class.TestClass.Inner_vect}
%class.TestClass.Inner = type { %class.MainClass.base, [4 x i8] }

%class.TestClass.Inner_vect = type { %class.TestClass.Inner_vect_imp, %class.TestClass.Inner_vect_imp, %class.TestClass.Inner_vect_imp}
%class.TestClass.Inner_vect_imp = type { ptr, ptr, ptr }

define dso_local void @foo(ptr noundef "intel_dtrans_func_index"="1" %ptr, i64 %var) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !17 {
entry:
  %0 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr, i64 0, i32 1
  %1 = getelementptr inbounds %class.TestClass.Inner_vect, ptr %0, i64 0, i32 0
  %2 = getelementptr inbounds %class.TestClass.Inner_vect_imp, ptr %1, i64 0, i32 1
  %3 = load ptr, ptr %2
  %4 = mul i64 %var, 24
  %5 = getelementptr i8, ptr %3, i64 %4
  store ptr %5, ptr %2

  ret void
}


declare !intel.dtrans.func.type !18 "intel_dtrans_func_index"="1" ptr @_Znwm(i64)
declare i32 @__gxx_personality_v0(...)
declare i64 @llvm.umax.i64(i64, i64)

!intel.dtrans.types = !{!5, !6, !8, !11, !13, !15}

!0 = !{i32 0, i32 0} ; i32
!1 = !{i8 0, i32 0}  ; i8
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{!"A", i32 4, !0} ; [4 x i32]
!4 = !{!"A", i32 4, !1} ; [4 x i8]

!5 = !{!"S", %class.MainClass.base zeroinitializer, i32 2, !3, !0} ; %class.MainClass.base = type { [4 x i32], i32}
!6 = !{!"S", %class.MainClass zeroinitializer, i32 3, !3, !0, !4} ; %class.MainClass = type { [4 x i32], i32, [4 x i8]}
!7 = !{%class.MainClass.base zeroinitializer, i32 0} ; %class.MainClass.base

!8 = !{!"S", %class.TestClass.Inner zeroinitializer, i32 2, !7, !4} ; %class.TestClass.Inner = type { %class.MainClass.base, [4 x i8] }
!9 = !{%class.TestClass.Inner zeroinitializer, i32 0} ; %class.TestClass.Inner
!10 = !{%class.TestClass.Inner zeroinitializer, i32 1} ; %class.TestClass.Inner*

!11 = !{!"S", %class.TestClass.Inner_vect_imp zeroinitializer, i32 3, !10, !10, !10} ; %class.TestClass.Inner_vect_imp = type { %class.TestClass.Inner*, %class.TestClass.Inner*, %class.TestClass.Inner* }
!12 = !{%class.TestClass.Inner_vect_imp zeroinitializer, i32 0} ; %class.TestClass.Inner_vect_imp

!13 = !{!"S", %class.TestClass.Inner_vect zeroinitializer, i32 3, !12, !12, !12} ; %class.TestClass.Inner_vect = type { %class.TestClass.Inner_vect_imp, %class.TestClass.Inner_vect_imp, %class.TestClass.Inner_vect_imp}
!14 = !{%class.TestClass.Inner_vect zeroinitializer, i32 0} ; %class.TestClass.Inner_vect

!15 = !{!"S", %class.TestClass.Outer zeroinitializer, i32 2, !9, !14} ; type { %class.TestClass.Inner, %class.TestClass.Inner_vect}
!16 = !{%class.TestClass.Outer zeroinitializer, i32 1} ; %class.TestClass.Outer*

!17 = distinct !{!16}
!18 = distinct !{!2}
