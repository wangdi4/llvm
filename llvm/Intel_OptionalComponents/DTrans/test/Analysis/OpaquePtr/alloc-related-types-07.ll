; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s

; This test case checks that "Bad casting (related types) -- Allocation
; includes related types" is not set since the size used in the allocation
; is different that the size used in the division.

; CHECK-NOT: dtrans-safety: Bad casting (related types) -- Allocation includes related types
; CHECK: dtrans-safety: Bad casting -- Allocation of ambiguous type
; CHECK:   [foo]   %17 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %16)
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

%class.TestClass.Outer = type { %class.TestClass.Inner, %class.TestClass.Inner_vect}
%class.TestClass.Inner = type { %class.MainClass.base, [4 x i8] }

%class.TestClass.Inner_vect = type { %class.TestClass.Inner_vect_imp, %class.TestClass.Inner_vect_imp, %class.TestClass.Inner_vect_imp}
%class.TestClass.Inner_vect_imp = type { ptr, ptr, ptr }

define dso_local void @foo(ptr noundef "intel_dtrans_func_index"="1" %ptr, ptr noundef "intel_dtrans_func_index"="2" %ptr2) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !17 {
entry:
  %0 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr, i64 0, i32 1
  %1 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr2, i64 0, i32 1
  %2 = getelementptr inbounds %class.TestClass.Inner_vect, ptr %1, i64 0, i32 1
  %3 = load ptr, ptr %2
  %4 = load ptr, ptr %1
  %5 = ptrtoint ptr %3 to i64
  %6 = ptrtoint ptr %4 to i64
  %7 = sub i64 %5, %6
  %8 = sdiv exact i64 %7, 32
  %9 = tail call i64 @llvm.umax.i64(i64 %8, i64 1)
  %10 = add nsw i64 %9, %8
  %11 = icmp ult i64 %10, %8
  %12 = icmp ugt i64 %10, 1092816592044404
  %13 = or i1 %11, %12
  %14 = select i1 %13, i64 1092816592044404, i64 %10
  %15 = icmp eq i64 %14, 0
  br i1 %15, label %call.to.new, label %continue.br

call.to.new:
  %16 = mul nuw nsw i64 %14, 24
  %17 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %16)
  br label %continue.br

continue.br:
  %18 = phi ptr [ null, %entry ], [ %17, %call.to.new ]
  %19 = getelementptr inbounds %class.TestClass.Inner, ptr %18, i64 %8
  %20 = getelementptr inbounds %class.MainClass, ptr %18, i64 0, i32 0
  br label %exit

exit:
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

!17 = distinct !{!16, !16}
!18 = distinct !{!2}
