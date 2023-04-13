; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -debug -disable-output 2>&1 | FileCheck %s

; The source code for the test is:
; kernel void test(global int *dst, int b) {
;   int gid = get_global_id(0);
;   if (-20 > b - gid)
;     dst[gid] = gid;
; }
; icmp inst is manually modified to sgt.

; CHECK:  found 2 early exit boundaries
; CHECK:    Dim=0, Contains=F, IsGID=T, IsSigned=T, IsUpperBound=F, Bound="  %to_tid_type = sext i32 %final_right_bound to i64"
; CHECK:    Dim=0, Contains=T, IsGID=T, IsSigned=T, IsUpperBound=T, Bound="  %to_tid_type1 = sext i32 %final_left_bound to i64"
; CHECK:  found 0 uniform early exit conditions

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %dst, i32 noundef %b) !no_barrier_path !1 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0)
  %conv = trunc i64 %call to i32
  %sub = sub nsw i32 %b, %conv
  %cmp = icmp sgt i32 -20, %sub
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %idxprom
  store i32 %conv, ptr addrspace(1) %arrayidx, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

declare i64 @_Z13get_global_idj(i32 noundef)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-COUNT-12: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY-COUNT-37: WARNING: Instruction with empty DebugLoc in function WG.boundaries.test
; DEBUGIFY: WARNING: Missing line
; DEBUGIFY-NOT: WARNING
