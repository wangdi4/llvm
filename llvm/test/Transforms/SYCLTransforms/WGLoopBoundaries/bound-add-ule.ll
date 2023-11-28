; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -debug 2>&1 | FileCheck %s

; The test is used to check the cmp instruction emitted for comparing right
; boundary against left boundary is ICMP_SLE.

; CHECK: WGLoopBoundaries constant_kernel
; CHECK:   found 0 early exit boundaries
; CHECK:   found 0 uniform early exit conditions

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @foo(i64)
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

define void @constant_kernel(ptr addrspace(1) noalias %out, i32 %lb, i32 %ub) local_unnamed_addr !no_barrier_path !1 {
entry:
  %gid  = tail call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %gid to i32
  %new_lb = add i32 %lb, %conv
  %cmp = icmp ule i32 %new_lb, %ub
  br i1 %cmp, label %if.end, label %if.then

if.then:
  %sext = shl i64 %gid, 32
  call void @foo(i64 %sext)
  br label %if.end

; CHECK-LABEL: entry
; CHECK: %new_lb = add i32 %lb, %conv
; CHECK: %right_lt_left = icmp sle i32 %ub, %lb

if.end:
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @constant_kernel}
!1 = !{i1 true}

; DEBUGIFY-COUNT-10: Instruction with empty DebugLoc in function constant_kernel
; DEBUGIFY-NOT: WARNING
