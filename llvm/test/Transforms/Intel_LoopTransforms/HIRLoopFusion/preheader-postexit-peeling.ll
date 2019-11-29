; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -disable-output -hir-loop-fusion -print-before=hir-loop-fusion -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This is a test with peeled post-loop, check that preheader and postexit nodes are moved to first and last loops after fusion.

; CHECK-LABEL: Function
; CHECK: BEGIN REGION { }
;              + DO i1 = 0, 99, 1   <DO_LOOP>
;              |   %y.0.lcssa = 0.000000e+00;
;              |
; CHECK:       |      %y.040 = 0.000000e+00;
; CHECK:       |   + DO i2 = 0, %n + -2, 1   <DO_LOOP>
; CHECK:       |   |   %y.040 = %y.040  +  (%p)[i2];
; CHECK:       |   + END LOOP
; CHECK:       |      %y.0.lcssa = %y.040;
; CHECK:       |
; CHECK:       |   %x.0.lcssa = 1.000000e+00;
; CHECK:       |
; CHECK:       |      %x.043 = 1.000000e+00;
; CHECK:       |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:       |   |   %add11 = (%p)[i2]  +  1.000000e+00;
; CHECK:       |   |   %x.043 = %x.043  +  %add11;
; CHECK:       |   + END LOOP
; CHECK:       |      %x.0.lcssa = %x.043;
;              |
;              |   %add16 = %y.0.lcssa  +  %x.0.lcssa;
;              |   %add17 = %r.046  +  %add16;
;              |   %conv18 = fptosi.float.i32(%add17);
;              |   %phitmp = sitofp.i32.float(%conv18);
;              |   %r.046 = %phitmp;
;              + END LOOP
; CHECK: END REGION

; CHECK-LABEL: Function
; CHECK: BEGIN REGION { modified }
;              + DO i1 = 0, 99, 1   <DO_LOOP>
;              |   %y.0.lcssa = 0.000000e+00;
;              |   %x.0.lcssa = 1.000000e+00;
;              |
; CHECK:       |      %x.043 = 1.000000e+00;
; CHECK:       |      %y.040 = 0.000000e+00;
; CHECK:       |   + DO i2 = 0, %n + -2, 1   <DO_LOOP>
; CHECK:       |   |   %add11 = (%p)[i2]  +  1.000000e+00;
; CHECK:       |   |   %x.043 = %x.043  +  %add11;
; CHECK:       |   |   %y.040 = %y.040  +  (%p)[i2];
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 0, 1   <DO_LOOP>
; CHECK:       |   |   %add11 = (%p)[i2 + %n + -1]  +  1.000000e+00;
; CHECK:       |   |   %x.043 = %x.043  +  %add11;
; CHECK:       |   + END LOOP
; CHECK:       |      %x.0.lcssa = %x.043;
; CHECK:       |      %y.0.lcssa = %y.040;
;              |
;              |   %add16 = %y.0.lcssa  +  %x.0.lcssa;
;              |   %add17 = %r.046  +  %add16;
;              |   %conv18 = fptosi.float.i32(%add17);
;              |   %phitmp = sitofp.i32.float(%conv18);
;              |   %r.046 = %phitmp;
;              + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local float @foo(float* nocapture readonly %p, i64 %n) local_unnamed_addr #0 {
entry:
  %sub = add nsw i64 %n, -1
  %cmp239 = icmp sgt i64 %sub, 0
  %cmp742 = icmp sgt i64 %n, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup8, %entry
  %i.047 = phi i64 [ 0, %entry ], [ %inc20, %for.cond.cleanup8 ]
  %r.046 = phi float [ 0.000000e+00, %entry ], [ %phitmp, %for.cond.cleanup8 ]
  br i1 %cmp239, label %for.body4.preheader, label %for.cond6.preheader

for.body4.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup8
  %phitmp.lcssa = phi float [ %phitmp, %for.cond.cleanup8 ]
  ret float %phitmp.lcssa

for.cond6.preheader.loopexit:                     ; preds = %for.body4
  %add.lcssa = phi float [ %add, %for.body4 ]
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.cond6.preheader.loopexit, %for.cond1.preheader
  %y.0.lcssa = phi float [ 0.000000e+00, %for.cond1.preheader ], [ %add.lcssa, %for.cond6.preheader.loopexit ]
  br i1 %cmp742, label %for.body9.preheader, label %for.cond.cleanup8

for.body9.preheader:                              ; preds = %for.cond6.preheader
  br label %for.body9

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %j.041 = phi i64 [ %inc, %for.body4 ], [ 0, %for.body4.preheader ]
  %y.040 = phi float [ %add, %for.body4 ], [ 0.000000e+00, %for.body4.preheader ]
  %arrayidx = getelementptr inbounds float, float* %p, i64 %j.041
  %0 = load float, float* %arrayidx, align 4
  %add = fadd float %y.040, %0
  %inc = add nuw nsw i64 %j.041, 1
  %exitcond = icmp eq i64 %inc, %sub
  br i1 %exitcond, label %for.cond6.preheader.loopexit, label %for.body4

for.cond.cleanup8.loopexit:                       ; preds = %for.body9
  %add12.lcssa = phi float [ %add12, %for.body9 ]
  br label %for.cond.cleanup8

for.cond.cleanup8:                                ; preds = %for.cond.cleanup8.loopexit, %for.cond6.preheader
  %x.0.lcssa = phi float [ 1.000000e+00, %for.cond6.preheader ], [ %add12.lcssa, %for.cond.cleanup8.loopexit ]
  %add16 = fadd float %y.0.lcssa, %x.0.lcssa
  %add17 = fadd float %r.046, %add16
  %conv18 = fptosi float %add17 to i32
  %inc20 = add nuw nsw i64 %i.047, 1
  %phitmp = sitofp i32 %conv18 to float
  %exitcond49 = icmp eq i64 %inc20, 100
  br i1 %exitcond49, label %for.cond.cleanup, label %for.cond1.preheader

for.body9:                                        ; preds = %for.body9.preheader, %for.body9
  %j5.044 = phi i64 [ %inc14, %for.body9 ], [ 0, %for.body9.preheader ]
  %x.043 = phi float [ %add12, %for.body9 ], [ 1.000000e+00, %for.body9.preheader ]
  %arrayidx10 = getelementptr inbounds float, float* %p, i64 %j5.044
  %1 = load float, float* %arrayidx10, align 4
  %add11 = fadd float %1, 1.000000e+00
  %add12 = fadd float %x.043, %add11
  %inc14 = add nuw nsw i64 %j5.044, 1
  %exitcond48 = icmp eq i64 %inc14, %n
  br i1 %exitcond48, label %for.cond.cleanup8.loopexit, label %for.body9
}

