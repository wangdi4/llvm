; This test verifies that DeadArrayOpsElimination optimization is not triggered
; for s_qsort. "-dead-array-ops-functions" is used to indicate s_qsort is Qsort
; function and used high index for %perm is 491. That means, all elements of
; the array may be used after s_qsort. So, DeadArrayOpsElimination is not
; triggered for s_qsort.
; This test is same as  deadarrayopselimination_01.ll.
; Note that the functions don't have any valid IR or meaning.

; REQUIRES: asserts
; RUN: opt < %s  -deadarrayopselimination -dead-array-ops-functions="s_qsort,491" -debug-only=deadarrayopselimination  -mtriple=i686-- -mattr=+avx2 -whole-program-assume  -enable-intel-advanced-opts -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s  -passes='module(deadarrayopselimination)' -dead-array-ops-functions="s_qsort,491" -debug-only=deadarrayopselimination  -mtriple=i686-- -mattr=+avx2 -whole-program-assume  -enable-intel-advanced-opts -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:   User Candidate: s_qsort  491
; CHECK:  DeadArrayOpsElimi: Considering qsort function: s_qsort
; CHECK:    First Call:   call void @s_qsort(i8* nonnull %bc1, i64 490)
; CHECK:    Recursion Call:   tail call fastcc void @s_qsort(i8* %t6, i64 %t3)
; CHECK:    ArraySize: 490
; CHECK:    failed: no partial use of array
; CHECK: DeadArrayOpsElimi Failed: No candidates found.

%struct.b = type { i32, i32 }

define internal void @s_qsort(i8* %arg, i64 %arg1) {
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
  call void @s_qsort(i8* nonnull %bc1, i64 490)
  br label %BB2

BB2:
  ret void
}

define dso_local i32 @main() {
entry:
  call void @foo()
  ret i32 0
}

