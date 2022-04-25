; RUN: opt -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-general-unroll -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 %s | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; Check that 6:34 %0 --> %0 FLOW (<) edge is not constructed because it's killed by <30> or <33>.

; This test is using hir-general-unroll to create copies of "%0".

; Also it uses unroll(3) to create at least 3 copies to catch an issue while
; determining loop lexical bounds in HIRDDAnalysis::computeLoopStartStops().

; <0>          BEGIN REGION { modified }
; <27>               + DO i1 = 0, 32, 1   <DO_LOOP> <nounroll>
; <29>               |   + DO i2 = 0, 9, 1   <DO_LOOP>
; <30>               |   |   %0 = (%p)[3 * i1];
; <31>               |   |   (%q)[3 * i1 + i2] = i2 + %0;
; <29>               |   + END LOOP
; <29>               |
; <32>               |
; <32>               |   + DO i2 = 0, 9, 1   <DO_LOOP>
; <33>               |   |   %0 = (%p)[3 * i1 + 1];
; <34>               |   |   (%q)[3 * i1 + i2 + 1] = i2 + %0;
; <32>               |   + END LOOP
; <32>               |
; <26>               |
; <26>               |   + DO i2 = 0, 9, 1   <DO_LOOP>
; <6>                |   |   %0 = (%p)[3 * i1 + 2];
; <11>               |   |   (%q)[3 * i1 + i2 + 2] = i2 + %0;
; <26>               |   + END LOOP
; <27>               + END LOOP
; <0>          END REGION

; CHECK-NOT: 6:34 %0 --> %0 FLOW (<)

source_filename = "outer-loop-unroll.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture readonly %p, i32* nocapture %q) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:
  %indvars.iv23 = phi i64 [ 0, %entry ], [ %indvars.iv.next24, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv23
  br label %for.body4

for.cond.cleanup:
  ret void

for.cond.cleanup3:
  %indvars.iv.next24 = add nuw nsw i64 %indvars.iv23, 1
  %exitcond25 = icmp eq i64 %indvars.iv.next24, 99
  br i1 %exitcond25, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !1

for.body4:
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  %2 = add nuw nsw i64 %indvars.iv, %indvars.iv23
  %arrayidx7 = getelementptr inbounds i32, i32* %q, i64 %2
  store i32 %add, i32* %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

!1 = distinct !{!1, !2}
!2 = !{!"llvm.loop.unroll.count", i32 3}

