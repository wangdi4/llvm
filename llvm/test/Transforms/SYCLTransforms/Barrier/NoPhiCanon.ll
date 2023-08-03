; RUN: opt -passes='sycl-kernel-barrier' -S < %s | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @fixSpecialValues() {
entry:
  call void @dummy_barrier.()
  %0 = tail call i64 @_Z12get_local_idj(i32 0)
  %add.i = add nuw nsw i64 %0, 42
  br label %Split.Barrier.BB1

Split.Barrier.BB1:
  tail call void @_Z18work_group_barrierj12memory_scope(i32 3, i32 1)
  br i1 false, label %L3, label %L1

L1:
  br i1 false, label %L2, label %L3

L2:
  br label %L3

L3:
; CHECK: %add.i.i = phi i64 [ %loadedValue{{[0-9]*}}, %SyncBB{{[0-9]*}} ], [ %loadedValue{{[0-9]*}}, %L1 ], [ %loadedValue{{[0-9]*}}, %L2 ]
  %add.i.i = phi i64 [ %add.i, %Split.Barrier.BB1 ], [ %add.i, %L1 ], [ %add.i, %L2 ]
  br label %Split.Barrier.BB

Split.Barrier.BB:
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

define void @fixCrossBarrierValues() {
entry:
; CHECK-LABEL: define void @fixCrossBarrierValues()
; CHECK-NEXT: entry:
; CHECK-NEXT: %p.addr = alloca ptr, align 8
; CHECK-NEXT: %pCurrBarrier = alloca i32, align 4
; CHECK-NEXT: %pCurrSBIndex = alloca i64, align 8
; CHECK-NEXT: %pLocalIds = alloca [3 x i64], align 8
; CHECK-NEXT: %add.i = add nuw nsw i64 0, 42

  call void @dummy_barrier.()
  %add.i = add nuw nsw i64 0, 42
  %p = alloca i64, align 8
  br label %Split.Barrier.BB1

Split.Barrier.BB1:
  tail call void @_Z18work_group_barrierj12memory_scope(i32 3, i32 1)
  br i1 false, label %L3, label %L1

L1:
  br i1 false, label %L2, label %L3

L2:
  br label %L3

L3:
; CHECK: %add.i.i = phi i64 [ %add.i, %SyncBB{{[0-9]*}} ], [ %add.i, %L1 ], [ %add.i, %L2 ]
  %add.i.i = phi i64 [ %add.i, %Split.Barrier.BB1 ], [ %add.i, %L1 ], [ %add.i, %L2 ]
  %pp = load i64, ptr %p, align 8
  br label %Split.Barrier.BB

Split.Barrier.BB:
  call void @_Z18work_group_barrierj(i32 1)
  call void @fixArgumentUsage(i64 %pp)
  ret void
}

define void @fixArgumentUsage(i64 %add.i) {
entry:
  call void @dummy_barrier.()
  br label %Split.Barrier.BB1

Split.Barrier.BB1:
  tail call void @_Z18work_group_barrierj12memory_scope(i32 3, i32 1)
  br i1 false, label %L3, label %L1

L1:
  br i1 false, label %L2, label %L3

L2:
  br label %L3

L3:
; CHECK: %add.i.i = phi i64 [ %loadedValue{{[0-9]*}}, %SyncBB{{[0-9]*}} ], [ %loadedValue{{[0-9]*}}, %L1 ], [ %loadedValue{{[0-9]*}}, %L2 ]
  %add.i.i = phi i64 [ %add.i, %Split.Barrier.BB1 ], [ %add.i, %L1 ], [ %add.i, %L2 ]
  br label %Split.Barrier.BB

Split.Barrier.BB:
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

declare i64 @_Z12get_local_idj(i32) 
declare void @_Z18work_group_barrierj12memory_scope(i32, i32)
declare void @dummy_barrier.()
declare void @_Z18work_group_barrierj(i32)
