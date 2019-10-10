; RUN: opt -hir-temp-cleanup -hir-loop-distribute-memrec -S -print-after=hir-loop-distribute-memrec < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

; Check that sparse array reduction is distributed from the rest of the loop.

; BEGIN REGION { }
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %mul1 = 2 * i1  *  i1;
;       |   %conv = uitofp.i64.float(%mul1);
;       |   %add = %conv  +  0x3FB99999A0000000;
;       |   %0 = (%q1)[i1];
;       |   %add3 = %add  +  (%p1)[%0];   <Sparse Array Reduction>
;       |   (%p1)[%0] = %add3;            <Sparse Array Reduction>
;       |   %mul5 = 3 * i1  *  i1;
;       |   %mul6 = %mul5  *  i1;
;       |   %add7 = %mul6  +  1;
;       |   %conv8 = uitofp.i64.float(%add7);
;       |   %add9 = %conv8  +  0x3FC99999A0000000;
;       |   %2 = (%q2)[i1];
;       |   %add12 = %add9  +  (%p2)[%2]; <Sparse Array Reduction>
;       |   (%p2)[%2] = %add12;           <Sparse Array Reduction>
;       + END LOOP
; END REGION

; CHECK:  BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   %mul1 = 2 * i1  *  i1;
; CHECK:       |   %conv = uitofp.i64.float(%mul1);
; CHECK:       |   %add = %conv  +  0x3FB99999A0000000;
; CHECK:       |   (%.TempArray)[0][i1] = %add;
; CHECK:       |   %mul5 = 3 * i1  *  i1;
; CHECK:       |   %mul6 = %mul5  *  i1;
; CHECK:       |   %add7 = %mul6  +  1;
; CHECK:       |   %conv8 = uitofp.i64.float(%add7);
; CHECK:       |   %add9 = %conv8  +  0x3FC99999A0000000;
; CHECK:       |   (%.TempArray1)[0][i1] = %add9;
; CHECK:       + END LOOP
;
;
; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   %0 = (%q1)[i1];
; CHECK:       |   %add = (%.TempArray)[0][i1];
; CHECK:       |   %add3 = %add  +  (%p1)[%0];
; CHECK:       |   (%p1)[%0] = %add3;
; CHECK:       |   %2 = (%q2)[i1];
; CHECK:       |   %add9 = (%.TempArray1)[0][i1];
; CHECK:       |   %add12 = %add9  +  (%p2)[%2];
; CHECK:       |   (%p2)[%2] = %add12;
; CHECK:       + END LOOP
; CHECK:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(float* noalias nocapture %p1, i64* noalias nocapture readonly %q1, float* noalias nocapture %p2, i64* noalias nocapture readonly %q2, float* noalias nocapture readnone %p, float* noalias nocapture readnone %q) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.022 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nuw i64 %i.022, 1
  %mul1 = mul i64 %mul, %i.022
  %conv = uitofp i64 %mul1 to float
  %add = fadd float %conv, 0x3FB99999A0000000
  %arrayidx = getelementptr inbounds i64, i64* %q1, i64 %i.022
  %0 = load i64, i64* %arrayidx, align 8
  %arrayidx2 = getelementptr inbounds float, float* %p1, i64 %0
  %1 = load float, float* %arrayidx2, align 4
  %add3 = fadd float %add, %1
  store float %add3, float* %arrayidx2, align 4
  %mul4 = mul nuw nsw i64 %i.022, 3
  %mul5 = mul i64 %mul4, %i.022
  %mul6 = mul i64 %mul5, %i.022
  %add7 = add nuw nsw i64 %mul6, 1
  %conv8 = uitofp i64 %add7 to float
  %add9 = fadd float %conv8, 0x3FC99999A0000000
  %arrayidx10 = getelementptr inbounds i64, i64* %q2, i64 %i.022
  %2 = load i64, i64* %arrayidx10, align 8
  %arrayidx11 = getelementptr inbounds float, float* %p2, i64 %2
  %3 = load float, float* %arrayidx11, align 4
  %add12 = fadd float %add9, %3
  store float %add12, float* %arrayidx11, align 4
  %inc = add nuw nsw i64 %i.022, 1
  %exitcond = icmp eq i64 %inc, 64
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

