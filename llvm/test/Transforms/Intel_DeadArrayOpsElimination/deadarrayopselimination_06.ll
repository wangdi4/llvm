; This test verifies that transformations for DeadArrayOpsElimination
; optimization is not triggered. "s_qsort" is treated as Qsort function
; since it is marked with "is-qsort". This test is same as
; darrayopselimination_05.ll except "baz" call in "foo". Array Analysis
; unable to detect "array use range info" because the array is escaped
; through "baz" call.
; Note that the functions don't have any valid IR or meaning.

; REQUIRES: asserts
; RUN: opt < %s -S -deadarrayopselimination -debug-only=deadarrayopselimination -disable-output -mtriple=i686-- -mattr=+avx2 -whole-program-assume  -enable-intel-advanced-opts 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes='module(deadarrayopselimination)' -debug-only=deadarrayopselimination -disable-output -mtriple=i686-- -mattr=+avx2 -whole-program-assume  -enable-intel-advanced-opts 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: DeadArrayOpsElimi: Considering qsort function: s_qsort
; CHECK:  First Call:   call void @s_qsort(i8* nonnull %bc1, i64 490) #2
; CHECK:  Recursion Call:   tail call fastcc void @s_qsort(i8* %t6, i64 %t3)
; CHECK:  ArraySize: 490
; CHECK:  Array Use Info: Full or Empty
; CHECK:  failed: no partial use of array
; CHECK: DeadArrayOpsElimi Failed: No candidates found.


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
  call void @baz(i8* %bc1)
  ret void
}

define dso_local i32 @main() {
entry:
  call void @foo()
  ret i32 0
}

declare void @baz(i8*)

attributes #0 = { "is-qsort" }
