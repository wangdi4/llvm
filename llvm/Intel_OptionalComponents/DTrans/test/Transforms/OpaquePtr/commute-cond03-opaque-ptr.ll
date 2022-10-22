; This test verifies that CommuteCond transformation is triggered
; using DTrans heuristics, which check that %ident is a field of a struct
; and the number of possible constant values for the field is less than 4.
; Verify that operands of %and1 are swapped by the transformation.
;
; This test is the same as commute-cond03.ll, but uses opaque pointers,
; and does not contain any pointer bitcasts.

;  RUN: opt < %s -S -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-commutecondop 2>&1 | FileCheck %s

; CHECK: define i32 @proc2
; CHECK: %and1 = and i1 %cmp2, %cmp1
; CHECK-NOT: %and1 = and i1 %cmp1, %cmp2

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64, i32 }

define i32 @main() {
entry:
  call void @proc1();
  ret i32 0
}

define i32 @proc2(ptr "intel_dtrans_func_index"="1" %arc, i64 %red_cost) !intel.dtrans.func.type !6 {
entry:
  %F4 = getelementptr %struct.test.01, ptr %arc, i32 0, i32 4
  %ident = load i16, ptr %F4
  %cmp = icmp slt i64 %red_cost, 0
  br i1 %cmp, label %land.lhs.true, label %lor.rhs

lor.rhs:                                          ; preds = %entry
  %cmp1 = icmp ne i64 %red_cost, 0
  %cmp2 = icmp eq i16 %ident, 2
  %and1 = and i1 %cmp1, %cmp2
  br i1 %and1, label %l.end.true, label %l.end.false

land.lhs.true:                                    ; preds = %entry
  %cmp3 = icmp eq i16 %ident, 1
  br i1 %cmp3, label %l.end.true, label %l.end.false

l.end.true:                              ; preds = %lor.rhs, %land.lhs.true
  ret i32 1

l.end.false:                             ; preds = %lor.rhs, %land.lhs.true
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call noalias ptr @calloc(i64 10, i64 56)
  %F4 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 4
  store i16 0, ptr %F4
  %S1 = select i1 undef, i16 1, i16 2
  store i16 %S1, ptr %F4
  %call2 = call i32 @proc2(ptr %call1, i64 20);
  ret void
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !1} ; { i32, i64, i32, i32, i16, i64*, i64, i32 }

!intel.dtrans.types = !{!9}

