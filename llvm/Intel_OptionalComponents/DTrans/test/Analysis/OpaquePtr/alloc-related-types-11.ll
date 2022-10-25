; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s

; This test case checks that "Bad casting (related types) -- Allocation
; includes related types" is set NOT for the call to new since the allocated
; space is NOT the same size as the parent structure %class.TestClass.Inner,
; but there is no dominant aggregate type.

; CHECK-NOT: dtrans-safety: Bad casting (related types) -- Allocation includes related types
; CHECK: dtrans-safety: Bad casting -- Allocation of ambiguous type
; CHECK:   [foo]   %1 = tail call noalias noundef nonnull ptr @_Znwm(i64 32)
; CHECK:     LocalPointerInfo: CompletelyAnalyzed
; CHECK:     Declared Types:
; CHECK:       Aliased types:
; CHECK:         %class.MainClass*
; CHECK:         %class.TestClass.Inner*
; CHECK:         i8*
; CHECK:       No element pointees.
; CHECK:     Usage Types:
; CHECK:       Aliased types:
; CHECK:         %class.MainClass*
; CHECK:         %class.TestClass.Inner*
; CHECK:         i8*
; CHECK:       No element pointees.


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.MainClass = type { [4 x i32], i32, [4 x i8]}
%class.MainClass.base = type { [4 x i32], i32}

%class.TestClass.Outer = type { %class.TestClass.Inner }
%class.TestClass.Inner = type { %class.MainClass.base, [4 x i8] }

define dso_local void @foo(ptr noundef "intel_dtrans_func_index"="1" %ptr) !intel.dtrans.func.type !13 {
entry:
  %0 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr, i64 0, i32 0
  %1 = tail call noalias noundef nonnull ptr @_Znwm(i64 32)
  %2 = getelementptr inbounds %class.TestClass.Inner, ptr %1, i64 0, i32 0
  %3 = getelementptr inbounds %class.MainClass, ptr %1, i64 0, i32 0
  br label %exit

exit:
  ret void
}

declare !intel.dtrans.func.type !14 "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

!intel.dtrans.types = !{!5, !6, !8, !11}

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

!11 = !{!"S", %class.TestClass.Outer zeroinitializer, i32 1, !9} ; type { %class.TestClass.Inner }
!12 = !{%class.TestClass.Outer zeroinitializer, i32 1} ; %class.TestClass.Outer*

!13 = distinct !{!12}
!14 = distinct !{!2}
