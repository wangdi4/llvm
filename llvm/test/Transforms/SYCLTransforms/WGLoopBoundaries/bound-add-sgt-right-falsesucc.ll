; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -debug 2>&1 | FileCheck %s

; Check that upper bound isn't incremented twice from 2 to 4.

; CHECK:  found 2 early exit boundaries
; CHECK:    Dim=0, Contains=F, IsGID=F, IsSigned=T, IsUpperBound=T, Bound="  %final_right_bound = select i1 %right_overflow, i64 9223372036854775807, i64 %right_bound"
; CHECK:    Dim=0, Contains=T, IsGID=F, IsSigned=T, IsUpperBound=F, Bound="  %final_left_bound = select i1 %right_lt_left, i64 9223372036854775807, i64 -2"
; CHECK:  found 0 uniform early exit conditions

; CHECK: define [7 x i64] @WG.boundaries.test
; CHECK-NOT: add i64 {{.*}}, 1
; CHECK-NOT: add i64 {{.*}}, 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(ptr addrspace(1) %out) !no_barrier_path !1 {
entry:
  %0 = tail call i64 @_Z12get_local_idj(i32 0)
  %add26.i = add i64 3, %0
  %cmp.i2 = icmp sgt i64 5, %add26.i
  br i1 %cmp.i2, label %for.body.i, label %common.ret

common.ret:                                       ; preds = %for.body.i, %entry
  ret void

for.body.i:                                       ; preds = %entry
  store float 0.000000e+00, ptr addrspace(1) null, align 4
  br label %common.ret
}

declare i64 @_Z12get_local_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}

; DEBUGIFY-COUNT-7: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY-COUNT-39: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test
; DEBUGIFY-COUNT-1: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
