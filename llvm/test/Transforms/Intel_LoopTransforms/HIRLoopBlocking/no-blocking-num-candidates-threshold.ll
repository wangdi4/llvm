; REQUIRES: asserts
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-blocking,print<hir>"  -debug-only=hir-loop-blocking -disable-output -hir-loop-blocking-candidate-num-threshold=2 2>&1 < %s | FileCheck %s
;
; Verify that by the threshold of candidates, the loop nests are not blocked.
;
; CHECK: Function: foo
;
; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:              |   |   %add = (@B)[0][i2][i1]  +  (@A)[0][i1][i2];
; CHECK:              |   |   (@A)[0][i1][i2] = %add;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
;
;
; CHECK:              + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:              |   |   %add31 = (@A)[0][i2][i1]  +  (@B)[0][i1][i2];
; CHECK:              |   |   (@B)[0][i1][i2] = %add31;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION
;
; CHECK: Too many loops to block in a function. BailOut.
;
; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:              |   |   %add = (@B)[0][i2][i1]  +  (@A)[0][i1][i2];
; CHECK:              |   |   (@A)[0][i1][i2] = %add;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
;
;
; CHECK:              + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:              |   |   %add31 = (@A)[0][i2][i1]  +  (@B)[0][i1][i2];
; CHECK:              |   |   (@B)[0][i1][i2] = %add31;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION

; Verify that without that threshold, loop blocking is done on two nests.
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-blocking,print<hir>"  -debug-only=hir-loop-blocking -disable-output 2>&1 < %s | FileCheck %s --check-prefix=DEFAULT

; DEFAULT: BEGIN REGION { modified }
; DEFAULT:       + DO i1 = 0, 15, 1   <DO_LOOP>
; DEFAULT:       |   %min = (-64 * i1 + 999 <= 63) ? -64 * i1 + 999 : 63;
; DEFAULT:       |   + DO i2 = 0, 15, 1   <DO_LOOP>
; DEFAULT:       |   |   %min4 = (-64 * i2 + 999 <= 63) ? -64 * i2 + 999 : 63;
; DEFAULT:       |   |   + DO i3 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; DEFAULT:       |   |   |   + DO i4 = 0, %min4, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; DEFAULT:       |   |   |   |   %add = (@B)[0][64 * i2 + i4][64 * i1 + i3]  +  (@A)[0][64 * i1 + i3][64 * i2 + i4];
; DEFAULT:       |   |   |   |   (@A)[0][64 * i1 + i3][64 * i2 + i4] = %add;
; DEFAULT:       |   |   |   + END LOOP
; DEFAULT:       |   |   + END LOOP
; DEFAULT:       |   + END LOOP
; DEFAULT:       + END LOOP

; DEFAULT:       + DO i1 = 0, 15, 1   <DO_LOOP>
; DEFAULT:       |   %min5 = (-64 * i1 + 999 <= 63) ? -64 * i1 + 999 : 63;
; DEFAULT:       |   + DO i2 = 0, 15, 1   <DO_LOOP>
; DEFAULT:       |   |   %min6 = (-64 * i2 + 999 <= 63) ? -64 * i2 + 999 : 63;
; DEFAULT:       |   |   + DO i3 = 0, %min5, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; DEFAULT:       |   |   |   + DO i4 = 0, %min6, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; DEFAULT:       |   |   |   |   %add31 = (@A)[0][64 * i2 + i4][64 * i1 + i3]  +  (@B)[0][64 * i1 + i3][64 * i2 + i4];
; DEFAULT:       |   |   |   |   (@B)[0][64 * i1 + i3][64 * i2 + i4] = %add31;
; DEFAULT:       |   |   |   + END LOOP
; DEFAULT:       |   |   + END LOOP
; DEFAULT:       |   + END LOOP
; DEFAULT:       + END LOOP
; DEFAULT: END REGION

;source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [1000 x [1000 x double]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [1000 x [1000 x double]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc14
  %indvars.iv64 = phi i64 [ 0, %entry ], [ %indvars.iv.next65, %for.inc14 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [1000 x [1000 x double]], ptr @A, i64 0, i64 %indvars.iv64, i64 %indvars.iv
  %0 = load double, ptr %arrayidx5, align 8
  %arrayidx9 = getelementptr inbounds [1000 x [1000 x double]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv64
  %1 = load double, ptr %arrayidx9, align 8
  %add = fadd fast double %1, %0
  store double %add, ptr %arrayidx5, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond.not, label %for.inc14, label %for.body3

for.inc14:                                        ; preds = %for.body3
  %indvars.iv.next65 = add nuw nsw i64 %indvars.iv64, 1
  %exitcond66.not = icmp eq i64 %indvars.iv.next65, 1000
  br i1 %exitcond66.not, label %for.cond20.preheader.preheader, label %for.cond1.preheader

for.cond20.preheader.preheader:                   ; preds = %for.inc14
  br label %for.cond20.preheader

for.cond20.preheader:                             ; preds = %for.cond20.preheader.preheader, %for.inc39
  %indvars.iv70 = phi i64 [ %indvars.iv.next71, %for.inc39 ], [ 0, %for.cond20.preheader.preheader ]
  br label %for.body22

for.body22:                                       ; preds = %for.cond20.preheader, %for.body22
  %indvars.iv67 = phi i64 [ 0, %for.cond20.preheader ], [ %indvars.iv.next68, %for.body22 ]
  %arrayidx26 = getelementptr inbounds [1000 x [1000 x double]], ptr @B, i64 0, i64 %indvars.iv70, i64 %indvars.iv67
  %2 = load double, ptr %arrayidx26, align 8
  %arrayidx30 = getelementptr inbounds [1000 x [1000 x double]], ptr @A, i64 0, i64 %indvars.iv67, i64 %indvars.iv70
  %3 = load double, ptr %arrayidx30, align 8
  %add31 = fadd fast double %3, %2
  store double %add31, ptr %arrayidx26, align 8
  %indvars.iv.next68 = add nuw nsw i64 %indvars.iv67, 1
  %exitcond69.not = icmp eq i64 %indvars.iv.next68, 1000
  br i1 %exitcond69.not, label %for.inc39, label %for.body22

for.inc39:                                        ; preds = %for.body22
  %indvars.iv.next71 = add nuw nsw i64 %indvars.iv70, 1
  %exitcond72.not = icmp eq i64 %indvars.iv.next71, 1000
  br i1 %exitcond72.not, label %for.end41, label %for.cond20.preheader

for.end41:                                        ; preds = %for.inc39
  ret void
}
