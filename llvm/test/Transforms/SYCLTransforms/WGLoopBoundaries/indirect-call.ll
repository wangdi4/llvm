; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S | FileCheck %s

; The test checks that the pass doesn't crash and treats indirect calls as
; operations w/ side effect and leaves the kernel untouched.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @foo(i64)
declare void @bar(i64)

define void @test_indirect_call(i1 %c) local_unnamed_addr !no_barrier_path !{i1 true} {
; CHECK-LABEL: define void @test_indirect_call
; CHECK-NEXT: entry:
; CHECK-NEXT:   %fn.ptr = select i1 %c, ptr @foo, ptr @bar
; CHECK-NEXT:   call void %fn.ptr(i64 0)
; CHECK-NEXT:   %0 = tail call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT:   %cmp = icmp ne i64 %0, 0
; CHECK-NEXT:   br i1 %cmp, label %if.then, label %exit
entry:
  %fn.ptr = select i1 %c, ptr @foo, ptr @bar
  call void %fn.ptr(i64 0)
  %0 = tail call i64 @_Z13get_global_idj(i32 0)
  %cmp = icmp ne i64 %0, 0
  br i1 %cmp, label %if.then, label %exit

if.then:
  call void @foo(i64 42)
  br label %exit

exit:
  ret void
}

; CHECK-NOT: define [7 x i64] @WG.boundaries.test_indirect_call

declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

!sycl.kernels = !{!0}

!0 = !{ptr @test_indirect_call}

; DEBUGIFY-NOT: WARNING
