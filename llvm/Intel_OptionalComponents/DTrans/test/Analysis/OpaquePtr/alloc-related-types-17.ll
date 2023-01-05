; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer-verbose -dtrans-print-types 2>&1 | FileCheck %s

; This test case checks that "Bad pointer manipulation -- Pointer subtract
; result has non-divide by size use" was set for the call to "new" in @foo, and
; the safety data "Bad pointer manipulation" was set for %class.MainClass.base.
; The reason is that the result from the subtraction in %7 is used for
; something else than the call to new (call in %14).

; CHECK: dtrans-safety: Bad pointer manipulation -- Pointer subtract result has non-divide by size use

; CHECK: LLVMType: %class.MainClass.base = type { [4 x i32], i32 }
; CHECK: Safety data: Bad alloc size | Bad pointer manipulation | Nested structure | Has C++ handling | Structure could be base for ABI padding{{ *$}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.MainClass = type { [4 x i32], i32, [4 x i8]}
%class.MainClass.base = type { [4 x i32], i32}

%class.TestClass.Outer = type { %class.TestClass.Inner, %class.TestClass.Inner_vect}
%class.TestClass.Inner = type { %class.MainClass.base, [4 x i8] }

%class.TestClass.Inner_vect = type { %class.TestClass.Inner_vect_imp, %class.TestClass.Inner_vect_imp, %class.TestClass.Inner_vect_imp}
%class.TestClass.Inner_vect_imp = type { ptr, ptr, ptr }

define dso_local void @foo(ptr noundef "intel_dtrans_func_index"="1" %ptr) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !17 {
entry:
  %0 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr, i64 0, i32 1
  %1 = getelementptr inbounds %class.TestClass.Inner_vect_imp, ptr %0, i64 0, i32 0
  %2 = getelementptr inbounds %class.TestClass.Inner_vect_imp, ptr %0, i64 0, i32 1
  %3 = load ptr, ptr %2
  %4 = load ptr, ptr %1
  %5 = ptrtoint ptr %3 to i64
  %6 = ptrtoint ptr %4 to i64
  %7 = sub i64 %5, %6
  %8 = sdiv exact i64 %7, 24
  %9 = icmp eq i64 %8, 0
  br i1 %9, label %invoke.good, label %is.not.zero.br

is.not.zero.br:
  %10 = icmp ugt i64 %8, 1092816592044404
  br i1 %10, label %is.out.of.bounds.br, label %call.to.new.br

is.out.of.bounds.br:
  br label %exit

call.to.new.br:
  %11 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %7)
          to label %invoke.good unwind label %unwind.br

invoke.good:
  %12 = phi ptr [ null, %entry ], [ %11, %call.to.new.br ]
  %13 = getelementptr inbounds %class.TestClass.Inner, ptr %12, i64 0, i32 0
  %14 = tail call noalias noundef nonnull ptr @test(i64 %7)
  br label %exit

unwind.br:
  %15 = landingpad { ptr, i32 }
        cleanup
  br label %exit

exit:
  ret void
}

declare !intel.dtrans.func.type !18 "intel_dtrans_func_index"="1" ptr @test(i64)

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

!17 = distinct !{!16}
!18 = distinct !{!2}
