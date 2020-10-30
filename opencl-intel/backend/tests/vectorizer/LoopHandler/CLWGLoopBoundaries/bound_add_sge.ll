; RUN: %oclopt -cl-loop-bound -S %s | FileCheck %s

; The test is used to check the cmp instruction emitted for comparing right
; boundary against left boundary is ICMP_SLE.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @foo(i64)
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

define void @constant_kernel(i32 addrspace(1)* noalias %out, i32 %lb, i32 %ub) local_unnamed_addr !no_barrier_path !1 {
entry:
  %gid  = tail call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %gid to i32
  %new_lb = add i32 %lb, %conv
  %cmp = icmp sge i32 %new_lb, %ub
  br i1 %cmp, label %if.end, label %if.then

if.then:
  %sext = shl i64 %gid, 32
  call void @foo(i64 %sext)
  br label %if.end

; CHECK-LABEL: entry
; CHECK: %new_lb = add i32 %lb, %conv
; CHECK-NEXT: %right_lt_left = icmp sle i32 %ub, %lb
; CHECK: define private [7 x i64] @WG.boundaries.constant_kernel(i32 addrspace(1)* %0, i32 %1, i32 %2)

if.end:
  ret void
}

!opencl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*,i32,i32)* @constant_kernel}
!1 = !{i1 true}
