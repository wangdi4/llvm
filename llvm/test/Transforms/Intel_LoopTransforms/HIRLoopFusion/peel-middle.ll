; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -disable-output -hir-loop-fusion -print-after=hir-loop-fusion -disable-hir-create-fusion-regions=0 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -disable-hir-create-fusion-regions=0 < %s 2>&1 | FileCheck %s

; Check that after the Loop Fusion the peel loop is placed
; as a nearest sibling of a fused body.

; BEGIN REGION { }
;       + DO i1 = 0, 23, 1   <DO_LOOP>
;       |   %conv = sitofp.i32.double(i1 + 1);
;       |   (@A)[0][i1 + 1] = %conv;
;       + END LOOP
;
;       + DO i1 = 0, 24, 1   <DO_LOOP>
;       |   (@B)[0][i1] = (@A)[0][i1 + 1];
;       + END LOOP
;
;       + DO i1 = 0, 24, 1   <DO_LOOP>
;       |   (i64*)(@C)[0][i1 + 1] = (i64*)(@B)[0][i1 + 1];
;       |   (@B)[0][i1 + 1] = 0.000000e+00;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }

; CHECK: + DO i1 = 0, 23, 1   <DO_LOOP>
; CHECK: |   %conv = sitofp.i32.double(i1 + 1);
; CHECK: |   (@A)[0][i1 + 1] = %conv;
; CHECK: |   (@B)[0][i1] = (@A)[0][i1 + 1];
; CHECK: + END LOOP

; CHECK: + DO i1 = 0, 0, 1   <DO_LOOP>
; CHECK: |   (@B)[0][i1 + 24] = (@A)[0][i1 + 25];
; CHECK: + END LOOP

; CHECK: + DO i1 = 0, 24, 1   <DO_LOOP>
; CHECK: |   (i64*)(@C)[0][i1 + 1] = (i64*)(@B)[0][i1 + 1];
; CHECK: |   (@B)[0][i1 + 1] = 0.000000e+00;
; CHECK: + END LOOP

; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [25 x double] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [25 x double] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [25 x double] zeroinitializer, align 16
@D = dso_local local_unnamed_addr global [25 x double] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv52 = phi i64 [ 1, %entry ], [ %indvars.iv.next53, %for.body ]
  %0 = trunc i64 %indvars.iv52 to i32
  %conv = sitofp i32 %0 to double
  %arrayidx = getelementptr inbounds [25 x double], [25 x double]* @A, i64 0, i64 %indvars.iv52
  store double %conv, double* %arrayidx, align 8
  %indvars.iv.next53 = add nuw nsw i64 %indvars.iv52, 1
  %exitcond54 = icmp eq i64 %indvars.iv.next53, 25
  br i1 %exitcond54, label %for.body6.preheader, label %for.body

for.body6.preheader:                              ; preds = %for.body
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.body6
  %indvars.iv48 = phi i64 [ %indvars.iv.next49, %for.body6 ], [ 1, %for.body6.preheader ]
  %arrayidx8 = getelementptr inbounds [25 x double], [25 x double]* @A, i64 0, i64 %indvars.iv48
  %1 = load double, double* %arrayidx8, align 8
  %2 = add nsw i64 %indvars.iv48, -1
  %arrayidx10 = getelementptr inbounds [25 x double], [25 x double]* @B, i64 0, i64 %2
  store double %1, double* %arrayidx10, align 8
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond51 = icmp eq i64 %indvars.iv.next49, 26
  br i1 %exitcond51, label %for.body21.preheader, label %for.body6

for.body21.preheader:                             ; preds = %for.body6
  br label %for.body21

for.cond.cleanup20:                               ; preds = %for.body21
  ret void

for.body21:                                       ; preds = %for.body21.preheader, %for.body21
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body21 ], [ 1, %for.body21.preheader ]
  %arrayidx23 = getelementptr inbounds [25 x double], [25 x double]* @B, i64 0, i64 %indvars.iv
  %3 = bitcast double* %arrayidx23 to i64*
  %4 = load i64, i64* %3, align 8
  %arrayidx25 = getelementptr inbounds [25 x double], [25 x double]* @C, i64 0, i64 %indvars.iv
  %5 = bitcast double* %arrayidx25 to i64*
  store i64 %4, i64* %5, align 8
  store double 0.000000e+00, double* %arrayidx23, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 26
  br i1 %exitcond, label %for.cond.cleanup20, label %for.body21
}

