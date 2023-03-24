; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we do not fail when trying to parse
; an invalid accumulative TC blob during loop collapsing.

; HIR before collapse:
;            BEGIN REGION { }
;                  + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;                  |   + DO i2 = 0, 4, 1   <DO_LOOP>
;                  |   |   + DO i3 = 0, 4, 1   <DO_LOOP>
;                  |   |   |   %add11 = (%B)[(%n * %n) * i1 + 5 * %n * i2 + i3]  +  1.000000e+00;
;                  |   |   |   (%A)[(%n * %n) * i1 + 5 * %n * i2 + i3] = %add11;
;                  |   |   + END LOOP
;                  |   + END LOOP
;                  + END LOOP
;            END REGION


; Check that no optimization happened.
; CHECK:     BEGIN REGION { }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(i32 noundef %n, i32 noundef %m, ptr nocapture noundef %A, ptr nocapture noundef readonly %B) local_unnamed_addr {
entry:
  %cmp47 = icmp sgt i32 %n, 0
  br i1 %cmp47, label %for.cond1.preheader.preheader, label %for.end25

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc23
  %i.048 = phi i32 [ %inc24, %for.inc23 ], [ 0, %for.cond1.preheader.preheader ]
  %mul7 = mul i32 %i.048, %n
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc20
  %j.046 = phi i32 [ 0, %for.cond1.preheader ], [ %inc21, %for.inc20 ]
  %mul9 = mul nuw nsw i32 %j.046, 5
  %reass.add = add i32 %mul9, %mul7
  %reass.mul = mul i32 %reass.add, %n
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %0 = trunc i64 %indvars.iv to i32
  %add10 = add i32 %reass.mul, %0
  %idxprom = sext i32 %add10 to i64
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %idxprom
  %1 = load float, ptr %arrayidx, align 4
  %add11 = fadd fast float %1, 1.000000e+00
  %arrayidx19 = getelementptr inbounds float, ptr %A, i64 %idxprom
  store float %add11, ptr %arrayidx19, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond.not, label %for.inc20, label %for.body6

for.inc20:                                        ; preds = %for.body6
  %inc21 = add nuw nsw i32 %j.046, 1
  %exitcond49.not = icmp eq i32 %inc21, 5
  br i1 %exitcond49.not, label %for.inc23, label %for.cond4.preheader

for.inc23:                                        ; preds = %for.inc20
  %inc24 = add nuw nsw i32 %i.048, 1
  %exitcond50.not = icmp eq i32 %inc24, %n
  br i1 %exitcond50.not, label %for.end25.loopexit, label %for.cond1.preheader

for.end25.loopexit:                               ; preds = %for.inc23
  br label %for.end25

for.end25:                                        ; preds = %for.end25.loopexit, %entry
  %idxprom26 = sext i32 %n to i64
  %arrayidx27 = getelementptr inbounds float, ptr %A, i64 %idxprom26
  %2 = load float, ptr %arrayidx27, align 4
  %conv = fptosi float %2 to i32
  ret i32 %conv
}

