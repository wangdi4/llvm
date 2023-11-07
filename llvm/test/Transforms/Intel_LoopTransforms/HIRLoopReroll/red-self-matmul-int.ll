; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; XFAIL:*

; ICC can reroll this basic matmul. ICX can't today.
; Current reroll implementation does not maintain information of
; definedAtLevel and DefAtLevel of
; possibly new SCEVs of (%4 * %5) or (%1 * %2).
; Notice that HIRParser may not choose to have
; (%4 * %5) or (%1 * %2) separate blobs (i.e blobIndex).
; Extension may require assignement of new symbase, blobIndex and so on.

; #define SIZE 1000
; int A[SIZE][SIZE];
; int B[SIZE][SIZE];
; int C[SIZE][SIZE];
;
; void foo() {
;
;   for (int i=0;  i<SIZE; i++)
;     for (int j=0;  j<SIZE; j++)
;       for (int k=0;  k<SIZE; k=k+2) {
;         C[i][j] += A[i][k] * B[k][j];
;         C[i][j] += A[i][k+1] * B[k+1][j];
;       }
;
; }

; CHECK: Function: foo

; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:            |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:            |   |   %0 = (@C)[0][i1][i2];
; CHECK:            |   |
; CHECK:            |   |   + DO i3 = 0, 499, 1   <DO_LOOP>
; CHECK:            |   |   |   %1 = (@A)[0][i1][2 * i3];
; CHECK:            |   |   |   %2 = (@B)[0][2 * i3][i2];
; CHECK:            |   |   |   %4 = (@A)[0][i1][2 * i3 + 1];
; CHECK:            |   |   |   %5 = (@B)[0][2 * i3 + 1][i2];
; CHECK:            |   |   |   %0 = (%4 * %5)  +  %0 + (%1 * %2);
; CHECK:            |   |   + END LOOP
; CHECK:            |   |
; CHECK:            |   |   (@C)[0][i1][i2] = %0;
; CHECK:            |   + END LOOP
; CHECK:            + END LOOP
; CHECK:      END REGION

; CHECK: Function: foo

; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:            |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:            |   |   %0 = (@C)[0][i1][i2];
; CHECK:            |   |
; CHECK:            |   |   + DO i3 = 0, 999, 1   <DO_LOOP>
; CHECK:            |   |   |   %1 = (@A)[0][i1][i3];
; CHECK:            |   |   |   %2 = (@B)[0][i3][i2];
; CHECK:            |   |   |   %0 = %0 + (%1 * %2);
; CHECK:            |   |   + END LOOP
; CHECK:            |   |
; CHECK:            |   |   (@C)[0][i1][i2] = %0;
; CHECK:            |   + END LOOP
; CHECK:            + END LOOP
; CHECK:      END REGION

;Module Before HIR; ModuleID = 'matmul-basic.c'
source_filename = "matmul-basic.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv65 = phi i64 [ 0, %entry ], [ %indvars.iv.next66, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond.cleanup7, %for.cond1.preheader
  %indvars.iv63 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next64, %for.cond.cleanup7 ]
  %arrayidx18 = getelementptr inbounds [1000 x [1000 x i32]], ptr @C, i64 0, i64 %indvars.iv65, i64 %indvars.iv63
  %arrayidx18.promoted = load i32, ptr %arrayidx18, align 4
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next66 = add nuw nsw i64 %indvars.iv65, 1
  %exitcond67 = icmp eq i64 %indvars.iv.next66, 1000
  br i1 %exitcond67, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.body8
  %add34.lcssa = phi i32 [ %add34, %for.body8 ]
  store i32 %add34.lcssa, ptr %arrayidx18, align 4
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond = icmp eq i64 %indvars.iv.next64, 1000
  br i1 %exitcond, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.cond5.preheader, %for.body8
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next, %for.body8 ]
  %0 = phi i32 [ %arrayidx18.promoted, %for.cond5.preheader ], [ %add34, %for.body8 ]
  %arrayidx10 = getelementptr inbounds [1000 x [1000 x i32]], ptr @A, i64 0, i64 %indvars.iv65, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx10, align 8
  %arrayidx14 = getelementptr inbounds [1000 x [1000 x i32]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv63
  %2 = load i32, ptr %arrayidx14, align 4
  %mul = mul nsw i32 %2, %1
  %add = add nsw i32 %0, %mul
  %3 = or i64 %indvars.iv, 1
  %arrayidx23 = getelementptr inbounds [1000 x [1000 x i32]], ptr @A, i64 0, i64 %indvars.iv65, i64 %3
  %4 = load i32, ptr %arrayidx23, align 4
  %arrayidx28 = getelementptr inbounds [1000 x [1000 x i32]], ptr @B, i64 0, i64 %3, i64 %indvars.iv63
  %5 = load i32, ptr %arrayidx28, align 4
  %mul29 = mul nsw i32 %5, %4
  %add34 = add nsw i32 %mul29, %add
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp6 = icmp ult i64 %indvars.iv.next, 1000
  br i1 %cmp6, label %for.body8, label %for.cond.cleanup7
}



