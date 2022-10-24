; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; This test verifies that transformations for DeadArrayOpsElimination
; optimization are done correctly. "s_qsort" is treated as Qsort function
; since it is marked with "is-qsort". Array Analysis helps to detect that
; only first 20 elements of the array are really used after "s_qsort"
; call in "foo".
; Note that the functions don't have any valid IR or meaning.

; RUN: opt < %s -S -passes='module(deadarrayopselimination)' -whole-program-assume 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Checks that s_qsort is cloned and 3rd argument is added to s_qsort call
; correctly.
;
; CHECK: define dso_local void @foo()
; CHECK: [[BC0:%[0-9]+]] = bitcast [491 x %struct.b*]* %perm to %struct.b**
; CHECK: %add.ptr = getelementptr inbounds %struct.b*, %struct.b** [[BC0]], i64 1
; CHECK: %bc1 = bitcast %struct.b** %add.ptr to i8*
; CHECK: [[BC1:%[0-9]+]] = bitcast i8* %bc1 to %struct.b**
; CHECK: [[GEP0:%[0-9]+]] = getelementptr %struct.b*, %struct.b** [[BC1]], i64 20
; CHECK: [[BC2:%[0-9]+]] = bitcast %struct.b** [[GEP0]] to i8*
; CHECK:  call void @s_qsort{{.*}}(i8* %bc1, i64 490, i8* [[BC2]])

; Checks that recursion call in cloned s_qsort is controlled under
; new condition without changing CFG.
;
; CHECK: define internal void @s_qsort{{.*}}(
; CHECK:  %t6 = getelementptr inbounds i8, i8* %t5, i64 4
; CHECK:  %t2 = icmp ugt i64 %t1, 8
; CHECK: [[CMP1:%[0-9]+]] = icmp ult i8* %t6, %2
; CHECK: [[AND1:%[0-9]+]] = and i1 [[CMP1]], %t2
; CHECK: br i1 [[AND1]], label %bb1, label %bb2


%struct.b = type { i32, i32 }

define internal void @s_qsort(i8* %arg, i64 %arg1) #0 {
entry:
  %t1 = add i64 %arg1, 1
  %t5 = getelementptr inbounds i8, i8* %arg, i64 1
  br label %bb0

bb0:
  %t6 = getelementptr inbounds i8, i8* %t5, i64 4
  %t2 = icmp ugt i64 %t1, 8
  br i1 %t2, label %bb1, label %bb2

bb1:
  %t3 = lshr i64 %t1, 3
  tail call fastcc void @s_qsort(i8* %t6, i64 %t3)
  br label %bb2

bb2:
  %t4 = icmp ugt i64 %t1, 4
  br i1 %t4, label %bb0, label %bb3

bb3:
  ret void
}

define dso_local void @foo() {
entry:
  %perm = alloca [491 x %struct.b*], align 16
  br label %BB1

BB1:
  %0 = bitcast [491 x %struct.b*]* %perm to %struct.b**
  %add.ptr = getelementptr inbounds %struct.b*, %struct.b** %0, i64 1
  %bc1 = bitcast %struct.b** %add.ptr to i8*
  call void @s_qsort(i8* nonnull %bc1, i64 490) #0
  br label %BB2

BB2:
  br label %loop

loop:
  %iv = phi i64 [ 0, %BB2 ], [ %iv.next, %loop ]
  %iv.next = add i64 %iv, 1
  %ptr = getelementptr inbounds %struct.b*, %struct.b** %0, i64 %iv
  %pbc = bitcast %struct.b** %ptr to i8**
  %p1 = load i8*, i8** %pbc
  %cmp = icmp eq i64 %iv, 20
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

define dso_local i32 @main() {
entry:
  call void @foo()
  ret i32 0
}

attributes #0 = { "is-qsort" }

; end INTEL_FEATURE_SW_ADVANCED
