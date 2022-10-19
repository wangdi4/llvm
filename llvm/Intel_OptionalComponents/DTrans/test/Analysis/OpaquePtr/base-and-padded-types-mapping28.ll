; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s

; This test case checks that the dtrans safety analyzer indentifies the call
; to @foo in @test as "Bad casting (related types)". The reason is that
; it has multiple declaration types, but all of them are related through the
; zero element access.

; CHECK: dtrans-safety: Bad casting (related types) -- Formal parameter is a related type of the actual parameter, or vice-versa
; CHECK:   [test]   %ret = call i32 @foo(ptr %phi)
; CHECK:   Arg#0:   %phi = phi ptr [ %1, %alloc.c.br ], [ %3, %alloc.b.br ], [ %5, %alloc.arr.br ], [ %6, %alloc.new.br ]
; CHECK:     LocalPointerInfo: CompletelyAnalyzed
; CHECK:     Declared Types:
; CHECK:       Aliased types:
; CHECK:         %struct.test.a*
; CHECK:         %struct.test.b*
; CHECK:         %struct.test.c*
; CHECK:         [4 x i32]*
; CHECK:         i8*
; CHECK:       No element pointees.
; CHECK:     Usage Types:
; CHECK:       Aliased types:
; CHECK:         %struct.test.a*
; CHECK:         %struct.test.b*
; CHECK:         %struct.test.c*
; CHECK:         [4 x i32]*
; CHECK:         i8*
; CHECK:       No element pointees.

; ModuleID = 'simple2.cpp'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%boost.array = type { [4 x i32] }

%struct.test.a = type { %boost.array, i32, [4 x i32], [4 x i8] }
%struct.test.a.base = type { %boost.array, i32, [4 x i32] }

%struct.test.b = type { %struct.test.a.base, [4 x i8] }
%struct.test.b.base = type { %struct.test.a.base }

%struct.test.c = type { %struct.test.b.base, [4 x i8] }

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @foo(ptr noundef "intel_dtrans_func_index"="1" %ptr) local_unnamed_addr #0 !intel.dtrans.func.type !19 {
entry:
  %0 = getelementptr inbounds [4 x i32], ptr %ptr, i64 0, i64 0
  %1 = load i32, ptr %0
  ret i32 %1
}

define i32 @test(i32 %var) {
entry:
  %0 = icmp sgt i32 %var, 0
  br i1 %0, label %alloc.c.br, label %then.first.br

alloc.c.br:
  %1 = alloca %struct.test.c
  br label %then.br

then.first.br:
  %2 = icmp sgt i32 %var, 100
  br i1 %2, label %alloc.b.br, label %alloc.arr.br

alloc.b.br:
  %3 = alloca %struct.test.b
  br label %then.br

then.second.br:
  %4 = icmp sgt i32 %var, 100
  br i1 %4, label %alloc.arr.br, label %alloc.new.br

alloc.arr.br:
  %5 = alloca [4 x i32]
  br label %then.br

alloc.new.br:
  %6 = call ptr @_Znwm(i64 40)
  br label %then.br

then.br:
  %phi = phi ptr [%1, %alloc.c.br], [%3, %alloc.b.br], [%5, %alloc.arr.br], [%6, %alloc.new.br]
  %ret = call i32 @foo(ptr %phi)
  ret i32 %ret
}

declare !intel.dtrans.func.type !18 "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!intel.dtrans.types = !{!5, !7, !9, !11, !13, !15}

!1 = !{i32 0, i32 0}      ; i32
!2 = !{!"A", i32 4, !1}   ; [4 x i32]
!3 = !{i8 0, i32 0}       ; i8
!4 = !{!"A", i32 4, !3}   ; [4 x i8]

!5 = !{!"S", %boost.array zeroinitializer, i32 1, !2} ; %boost.array = type { [4 x i32] }
!6 = !{%boost.array zeroinitializer, i32 0}           ; %boost.array

!7 = !{!"S", %struct.test.a.base zeroinitializer, i32 3, !6, !1, !2} ; %struct.test.a.base = type { %boost.array, i32, [4 x i32] }
!8 = !{%struct.test.a.base zeroinitializer, i32 0}    ; %struct.test.a.base

!9 = !{!"S", %struct.test.a zeroinitializer, i32 4, !6, !1, !2, !4} ; %struct.test.a = type { %boost.array, i32, [4 x i32], [4 x i8] }
!10 = !{%struct.test.a zeroinitializer, i32 1}        ; %struct.test.a*

!11 = !{!"S", %struct.test.b.base zeroinitializer, i32 1, !8 } ; %struct.test.b.base = type { %struct.test.a.base }
!12 = !{%struct.test.b.base zeroinitializer, i32 0}    ; %struct.test.b.base

!13 = !{!"S", %struct.test.b zeroinitializer, i32 2, !8, !4 } ; %struct.test.b = type { %struct.test.a.base, [4 x i8] }
!14 = !{%struct.test.b zeroinitializer, i32 1}         ; %struct.test.b*

!15 = !{!"S", %struct.test.c zeroinitializer, i32 2, !12, !4} ; %struct.test.c = type { %struct.test.b.base, [4 x i8] }
!16 = !{%struct.test.c zeroinitializer, i32 1}         ; %struct.test.c*

!17 = !{i8 0, i32 1}  ; i8*
!18 = distinct !{!17}
!19 = distinct !{!10}
