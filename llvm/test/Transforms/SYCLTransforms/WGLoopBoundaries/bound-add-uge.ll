; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S | FileCheck %s

; No early exit boundary is generated for uge comparison with non-const operand.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @foo(i64)
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

define void @constant_kernel(ptr addrspace(1) noalias %out, i32 %lb, i32 %ub) local_unnamed_addr !no_barrier_path !1 {
entry:
  %gid  = tail call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %gid to i32
  %new_lb = add i32 %lb, %conv
  %cmp = icmp uge i32 %new_lb, %ub
  br i1 %cmp, label %if.end, label %if.then

if.then:
  %sext = shl i64 %gid, 32
  call void @foo(i64 %sext)
  br label %if.end

if.end:
  ret void
}

; CHECK-NOT: define [7 x i64] @WG.boundaries.constant_kernel(ptr addrspace(1) noalias %{{.*}}, i32 %{{.*}}, i32 %{{.*}})

!sycl.kernels = !{!0}

!0 = !{ptr @constant_kernel}
!1 = !{i1 true}

; DEBUGIFY-NOT: WARNING
