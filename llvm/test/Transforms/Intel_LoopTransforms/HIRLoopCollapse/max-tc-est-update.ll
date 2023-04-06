; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that MAX_TC_EST and LEGAL_MAX_TC are assigned correctly to the collapsed loop, if we can calculate them.

; HIR before collapse:
;            BEGIN REGION { }
;                  + DO i1 = 0, 8, 1   <DO_LOOP>
;                  |   + DO i2 = 0, zext.i16.i64(trunc.i32.i16(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 65535>
;                  |   |   + DO i3 = 0, 29, 1   <DO_LOOP>
;                  |   |   |   %add = (@B)[0][i1 + 1][i2][i3]  +  1.000000e+00;
;                  |   |   |   (@A)[0][i1 + 1][i2][i3] = %add;
;                  |   |   + END LOOP
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR after collapse:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 8, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 30 * zext.i16.i64(trunc.i32.i16(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 600>  <LEGAL_MAX_TC = 1966050>
; CHECK:           |   |   %add = (@B)[0][i1 + 1][0][i2]  +  1.000000e+00;
; CHECK:           |   |   (@A)[0][i1 + 1][0][i2] = %add;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [10 x [20 x [30 x float]]] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [10 x [20 x [30 x float]]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  %and = and i32 %n, 65535
  %tc = zext i32 %and to i64
  %cmp233 = icmp sgt i64 %tc, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc20
  %indvars.iv39 = phi i64 [ 1, %entry ], [ %indvars.iv.next40, %for.inc20 ]
  br i1 %cmp233, label %for.cond4.preheader.preheader, label %for.inc20

for.cond4.preheader.preheader:                    ; preds = %for.cond1.preheader
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.preheader, %for.inc17
  %indvars.iv36 = phi i64 [ %indvars.iv.next37, %for.inc17 ], [ 0, %for.cond4.preheader.preheader ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [10 x [20 x [30 x float]]], ptr @B, i64 0, i64 %indvars.iv39, i64 %indvars.iv36, i64 %indvars.iv
  %0 = load float, ptr %arrayidx10, align 4
  %add = fadd fast float %0, 1.000000e+00
  %arrayidx16 = getelementptr inbounds [10 x [20 x [30 x float]]], ptr @A, i64 0, i64 %indvars.iv39, i64 %indvars.iv36, i64 %indvars.iv
  store float %add, ptr %arrayidx16, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 30
  br i1 %exitcond.not, label %for.inc17, label %for.body6

for.inc17:                                        ; preds = %for.body6
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %exitcond38.not = icmp eq i64 %indvars.iv.next37, %tc
  br i1 %exitcond38.not, label %for.inc20.loopexit, label %for.cond4.preheader

for.inc20.loopexit:                               ; preds = %for.inc17
  br label %for.inc20

for.inc20:                                        ; preds = %for.inc20.loopexit, %for.cond1.preheader
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond41.not = icmp eq i64 %indvars.iv.next40, 10
  br i1 %exitcond41.not, label %for.end22, label %for.cond1.preheader

for.end22:                                        ; preds = %for.inc20
  %1 = load float, ptr @A, align 16
  %conv = fptosi float %1 to i32
  ret i32 %conv
}

