; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s

; This test case checks that "Bad casting -- Allocation of ambiguous type" was
; printed for the call to "new" in function @test. The reason is because the
; subtraction that sets the alloc size is not guarded with the check for 0.

; CHECK-NOT: dtrans-safety: Bad casting (related types) -- Allocation includes related types
; CHECK: dtrans-safety: Bad casting -- Allocation of ambiguous type
; CHECK:   [foo]   %10 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %7)
; CHECK:           to label %invoke.good unwind label %unwind.br
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
  %8 = sdiv exact i64 %7, 24
  %9 = icmp ugt i64 %8, 1092816592044404
  br i1 %9, label %invoke.good, label %call.to.new.br

call.to.new.br:
  %10 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %7)
          to label %invoke.good unwind label %unwind.br

invoke.good:
  %11 = phi ptr [ null, %entry ], [ %10, %call.to.new.br ]
  store ptr %11, ptr %0
  %12 = getelementptr inbounds %class.TestClass.Inner_vect, ptr %0, i64 0, i32 1
  store ptr %11, ptr %12
  %13 = getelementptr inbounds %class.MainClass, ptr %11, i64 %8
  br label %exit

unwind.br:
  %14 = landingpad { ptr, i32 }
        cleanup
  br label %exit

exit:
  ret void
}


declare !intel.dtrans.func.type !18 "intel_dtrans_func_index"="1" ptr @_Znwm(i64)
declare i32 @__gxx_personality_v0(...)

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
