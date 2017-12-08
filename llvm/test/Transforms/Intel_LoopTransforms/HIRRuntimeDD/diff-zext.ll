; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -disable-output -print-after=hir-runtime-dd < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Check that the pair [ q[zext.i32.i64(i1 - 1)], q[i1] ] will not be placed into a single
; group as the distance between them is not constant.

; BEGIN REGION { }
;      + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;       |   (%p)[0] = 0.000000e+00;
;       |   %conv1 = (%q)[i1 + -1]  +  5.000000e-01;  <<<<< %(%q)[zext.i32.i64(i1 + -1)]
;       |   (%q)[i1] = %conv1;
;      + END LOOP
; END REGION

; CHECK-NOT: if (%mv.and == 0)

source_filename = "diff-sext.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(float* nocapture %p, float* nocapture %q, i32 %n) {
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  store float 0.000000e+00, float* %p, align 4
  %sub = add nuw nsw i64 %indvars.iv, 4294967295
  %idxprom = and i64 %sub, 4294967295
  %arrayidx = getelementptr inbounds float, float* %q, i64 %idxprom
  %0 = load float, float* %arrayidx, align 4
  %conv1 = fadd float %0, 5.000000e-01
  %arrayidx3 = getelementptr inbounds float, float* %q, i64 %indvars.iv
  store float %conv1, float* %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

