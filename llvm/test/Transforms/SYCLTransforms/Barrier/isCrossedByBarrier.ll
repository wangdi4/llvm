; RUN: opt -passes=sycl-kernel-barrier -S < %s -disable-output

;;*****************************************************************************
;; This test checks the KernelBarrier pass doesn't produce a broken module
;; for work_group_barrier with non-contant parementers
;;*****************************************************************************
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @_Z18work_group_barrierj12memory_scope(i32)

define void @_ZTS10test_fenceILi1EE() {
sg.loop.exclude:
  %sg.lid.ptr = alloca i32, align 4
  %sg.loop.src.ptr = alloca i32, align 4
  br label %Split.Barrier.BB394

Split.Barrier.BB394:                              ; preds = %sg.dummy.bb.17, %sg.loop.exclude
  call void @_Z18work_group_barrierj12memory_scope(i32 0)
  store i32 0, ptr %sg.lid.ptr, align 4
  store i32 0, ptr %sg.loop.src.ptr, align 4
  %0 = load i32, ptr null, align 4
  br label %Split.Barrier.BB392

Split.Barrier.BB392:                              ; preds = %Split.Barrier.BB394
  call void @_Z18work_group_barrierj12memory_scope(i32 %0)
  br label %sg.dummy.bb.17

sg.dummy.bb.17:                                   ; preds = %Split.Barrier.BB392
  store i32 0, ptr %sg.lid.ptr, align 4
  store i32 12, ptr %sg.loop.src.ptr, align 4
  br label %Split.Barrier.BB394
}
