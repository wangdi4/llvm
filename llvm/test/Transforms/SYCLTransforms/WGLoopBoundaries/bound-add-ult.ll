; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -debug 2>&1 | FileCheck %s

; CHECK: WGLoopBoundaries constant_kernel
; CHECK:   found 0 early exit boundaries
; CHECK:   found 0 uniform early exit conditions

; CHECK: WGLoopBoundaries constant_kernel1
; CHECK:   found 0 early exit boundaries
; CHECK:   found 0 uniform early exit conditions

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @foo(i64)
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

; No early exit boundary is generated for ult comparison with non-const operand.
define void @constant_kernel(ptr addrspace(1) noalias %out, i32 %lb, i32 %ub) local_unnamed_addr !no_barrier_path !1 {
entry:
  %gid  = tail call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %gid to i32
  %new_lb = add i32 %lb, %conv
  %cmp = icmp ult i32 %new_lb, %ub
  br i1 %cmp, label %if.end, label %if.then

if.then:
  %sext = shl i64 %gid, 32
  call void @foo(i64 %sext)
  br label %if.end

if.end:
  ret void
}

; No early exit boundary is generated for ult comparison with 2 interval loop boundaries.
define void @constant_kernel1(ptr addrspace(1) noalias %out) local_unnamed_addr !no_barrier_path !1 {
  %1 = tail call i64 @_Z13get_global_idj(i32 0)
  %2 = add nsw i64 %1, -10
  %cmp1.i.i = icmp ult i64 %2, -4
  br i1 %cmp1.i.i, label %if.end.i, label %if.exit

if.end.i:                                         ; preds = %entry
  %arrayidx.i = getelementptr inbounds i32, ptr addrspace(1) %out, i64 %1
  store i32 1, ptr addrspace(1) %arrayidx.i, align 4
  br label %if.exit

if.exit: ; preds = %if.end.i, %entry
  ret void
}

; CHECK-NOT: define [7 x i64] @WG.boundaries.constant_kernel(ptr addrspace(1) noalias %{{.*}}, i32 %{{.*}}, i32 %{{.*}})
; CHECK-NOT: define [7 x i64] @WG.boundaries.constant_kernel1(ptr addrspace(1) noalias %{{.*}}, i32 %{{.*}}, i32 %{{.*}})

!sycl.kernels = !{!0}

!0 = !{ptr @constant_kernel, ptr @constant_kernel1}
!1 = !{i1 true}

; DEBUGIFY-NOT: WARNING
