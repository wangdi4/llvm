; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-loop-fusion-skip-vec-prof-check  < %s 2>&1 | FileCheck %s

; Test checks that DD analysis is able to refine the edge between
; collapsed ((@A)[0][i1][i2 + 1][0][i3]) and non-collapsed
; ((@A)[0][i1][i2][0][0]) references and allow fusion
; on the higher level (i2).

; HIR before optimizations:
;            BEGIN REGION { }
;                  + DO i1 = 0, 9, 1   <DO_LOOP>
;                  |   + DO i2 = 0, 8, 1   <DO_LOOP>
;                  |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
;                  |   |   |   + DO i4 = 0, 9, 1   <DO_LOOP>
;                  |   |   |   |   %add = (@B)[0][i1][i2 + 1][i3][i4]  +  1.000000e+00;
;                  |   |   |   |   (@A)[0][i1][i2 + 1][i3][i4] = %add;
;                  |   |   |   + END LOOP
;                  |   |   + END LOOP
;                  |   |
;                  |   |   (@C)[0][i1][i2 + 1] = (@A)[0][i1][i2 + 1][9][9];
;                  |   + END LOOP
;                  |
;                  |
;                  |   + DO i2 = 0, 8, 1   <DO_LOOP>
;                  |   |   %add53 = (@C)[0][i1][i2 + 1]  +  %sum.091;
;                  |   |   %sum.091 = %add53  +  (@A)[0][i1][i2][0][0];
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR after collapse:
;            BEGIN REGION { modified }
;                  + DO i1 = 0, 9, 1   <DO_LOOP>
;                  |   + DO i2 = 0, 8, 1   <DO_LOOP>
;                  |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;                  |   |   |   %add = (@B)[0][i1][i2 + 1][0][i3]  +  1.000000e+00;
;                  |   |   |   (@A)[0][i1][i2 + 1][0][i3] = %add;   < num collapsed levels:2 >
;                  |   |   + END LOOP
;                  |   |
;                  |   |   (@C)[0][i1][i2 + 1] = (@A)[0][i1][i2 + 1][9][9];
;                  |   + END LOOP
;                  |
;                  |
;                  |   + DO i2 = 0, 8, 1   <DO_LOOP>
;                  |   |   %add53 = (@C)[0][i1][i2 + 1]  +  %sum.091;
;                  |   |   %sum.091 = %add53  +  (@A)[0][i1][i2][0][0]; < num collapsed levels:0 >
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR after fusion:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   %add = (@B)[0][i1][i2 + 1][0][i3]  +  1.000000e+00;
; CHECK:           |   |   |   (@A)[0][i1][i2 + 1][0][i3] = %add;
; CHECK:           |   |   + END LOOP
; CHECK:           |   |
; CHECK:           |   |   (@C)[0][i1][i2 + 1] = (@A)[0][i1][i2 + 1][9][9];
; CHECK:           |   |   %add53 = (@C)[0][i1][i2 + 1]  +  %sum.091;
; CHECK:           |   |   %sum.091 = %add53  +  (@A)[0][i1][i2][0][0];
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [10 x [10 x [10 x [10 x float]]]] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [10 x [10 x [10 x [10 x float]]]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [10 x [10 x float]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo() local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc58
  %indvars.iv102 = phi i64 [ 0, %entry ], [ %indvars.iv.next103, %for.inc58 ]
  %sum.091 = phi float [ 0.000000e+00, %entry ], [ %add54.lcssa, %for.inc58 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.end26
  %indvars.iv95 = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next96, %for.end26 ]
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.cond4.preheader, %for.inc24
  %indvars.iv92 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next93, %for.inc24 ]
  br label %for.body9

for.body9:                                        ; preds = %for.cond7.preheader, %for.body9
  %indvars.iv = phi i64 [ 0, %for.cond7.preheader ], [ %indvars.iv.next, %for.body9 ]
  %arrayidx15 = getelementptr inbounds [10 x [10 x [10 x [10 x float]]]], ptr @B, i64 0, i64 %indvars.iv102, i64 %indvars.iv95, i64 %indvars.iv92, i64 %indvars.iv
  %0 = load float, ptr %arrayidx15, align 4
  %add = fadd fast float %0, 1.000000e+00
  %arrayidx23 = getelementptr inbounds [10 x [10 x [10 x [10 x float]]]], ptr @A, i64 0, i64 %indvars.iv102, i64 %indvars.iv95, i64 %indvars.iv92, i64 %indvars.iv
  store float %add, ptr %arrayidx23, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond.not, label %for.inc24, label %for.body9

for.inc24:                                        ; preds = %for.body9
  %indvars.iv.next93 = add nuw nsw i64 %indvars.iv92, 1
  %exitcond94.not = icmp eq i64 %indvars.iv.next93, 10
  br i1 %exitcond94.not, label %for.end26, label %for.cond7.preheader

for.end26:                                        ; preds = %for.inc24
  %arrayidx32 = getelementptr inbounds [10 x [10 x [10 x [10 x float]]]], ptr @A, i64 0, i64 %indvars.iv102, i64 %indvars.iv95, i64 9, i64 9
  %1 = load float, ptr %arrayidx32, align 4
  %arrayidx36 = getelementptr inbounds [10 x [10 x float]], ptr @C, i64 0, i64 %indvars.iv102, i64 %indvars.iv95
  store float %1, ptr %arrayidx36, align 4
  %indvars.iv.next96 = add nuw nsw i64 %indvars.iv95, 1
  %exitcond97.not = icmp eq i64 %indvars.iv.next96, 10
  br i1 %exitcond97.not, label %for.body42.preheader, label %for.cond4.preheader

for.body42.preheader:                             ; preds = %for.end26
  br label %for.body42

for.body42:                                       ; preds = %for.body42.preheader, %for.body42
  %indvars.iv98 = phi i64 [ %indvars.iv.next99, %for.body42 ], [ 1, %for.body42.preheader ]
  %sum.189 = phi float [ %add54, %for.body42 ], [ %sum.091, %for.body42.preheader ]
  %arrayidx46 = getelementptr inbounds [10 x [10 x float]], ptr @C, i64 0, i64 %indvars.iv102, i64 %indvars.iv98
  %2 = load float, ptr %arrayidx46, align 4
  %3 = add nsw i64 %indvars.iv98, -1
  %arrayidx52 = getelementptr inbounds [10 x [10 x [10 x [10 x float]]]], ptr @A, i64 0, i64 %indvars.iv102, i64 %3, i64 0, i64 0
  %4 = load float, ptr %arrayidx52, align 16
  %add53 = fadd fast float %2, %sum.189
  %add54 = fadd fast float %add53, %4
  %indvars.iv.next99 = add nuw nsw i64 %indvars.iv98, 1
  %exitcond101.not = icmp eq i64 %indvars.iv.next99, 10
  br i1 %exitcond101.not, label %for.inc58, label %for.body42

for.inc58:                                        ; preds = %for.body42
  %add54.lcssa = phi float [ %add54, %for.body42 ]
  %indvars.iv.next103 = add nuw nsw i64 %indvars.iv102, 1
  %exitcond104.not = icmp eq i64 %indvars.iv.next103, 10
  br i1 %exitcond104.not, label %for.end60, label %for.cond1.preheader

for.end60:                                        ; preds = %for.inc58
  %add54.lcssa.lcssa = phi float [ %add54.lcssa, %for.inc58 ]
  %5 = load float, ptr @A, align 16
  %add61 = fadd fast float %5, %add54.lcssa.lcssa
  %conv = fptosi float %add61 to i32
  ret i32 %conv
}
