; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S | FileCheck %s

; The test checks the pass treats get_sub_group_local_id/get_sub_group_id as
; non-uniform and doesn't eliminate the branch.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @foo(i32)

define void @test_get_sub_group_id() local_unnamed_addr !no_barrier_path !{i1 true} {
; CHECK-LABEL: define void @test_get_sub_group_id
; CHECK: %cmp = icmp eq i32 %0, 0
; CHECK: br i1 %cmp, label %if.then, label %exit
entry:
  %0 = tail call i32 @_Z16get_sub_group_idv() #3
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %exit

if.then:
  call void @foo(i32 %0)
  br label %exit

exit:
  ret void
}

define void @test_get_sub_group_local_id() local_unnamed_addr !no_barrier_path !{i1 true} {
; CHECK-LABEL: define void @test_get_sub_group_local_id
; CHECK: %cmp = icmp eq i32 %0, 0
; CHECK: br i1 %cmp, label %if.then, label %exit
entry:
  %0 = tail call i32 @_Z22get_sub_group_local_idv() #3
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %exit

if.then:
  call void @foo(i32 %0)
  br label %exit

exit:
  ret void
}

declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #3
declare i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #3
declare i32 @_Z16get_sub_group_idv() local_unnamed_addr #3

attributes #3 = { nounwind readnone }

!sycl.kernels = !{!0}

!0 = !{void ()* @test_get_sub_group_id, void ()* @test_get_sub_group_local_id}

; DEBUGIFY-NOT: WARNING
